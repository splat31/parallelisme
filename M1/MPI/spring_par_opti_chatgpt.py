
#!/usr/bin/env python3

import numpy as np
import time
import sys
from spring_lib import *
from mpi4py import MPI


# --------------------------------------------------
# Initialisation MPI
# --------------------------------------------------

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()


# --------------------------------------------------
# Paramètres
# --------------------------------------------------

if rank == 0:
    try:
        display = '-nodisplay' not in sys.argv
        nb_masses = int(sys.argv[1])
        nb_springs = int(sys.argv[2])
        nb_turns = int(sys.argv[3])

        params = np.array(
            [nb_masses, nb_springs, nb_turns, display, 1],
            dtype='i'
        )

    except Exception:
        params = np.zeros(5, dtype='i')

else:
    params = np.empty(5, dtype='i')


comm.Bcast(params, root=0)

if params[4] == 0:
    if rank == 0:
        print(
            f'Usage: {sys.argv[0]} '
            'nb_masses nb_springs nb_turns [-nodisplay]'
        )
    sys.exit(1)


nb_masses, nb_springs, nb_turns, display, _ = params


# --------------------------------------------------
# Génération des données
# --------------------------------------------------

if rank == 0:
    masses, springs = generate_springs(
        nb_masses,
        nb_springs
    )

    time_start = time.time()

else:
    masses = np.empty((nb_masses, 6), dtype='i')
    springs = None


# --------------------------------------------------
# Distribution des masses
# --------------------------------------------------

if rank == 0:
    spring_count = np.array([len(springs)], dtype='i')
else:
    spring_count = np.empty(1, dtype='i')

comm.Bcast(spring_count, root=0)

nb_springs = int(spring_count[0])

comm.Bcast(masses, root=0)


# --------------------------------------------------
# Distribution équilibrée des ressorts
# --------------------------------------------------

# Permet de distribuer également les ressorts
# lorsque nb_springs n'est pas divisible par size.

counts = np.array(
    [
        nb_springs // size + (r < nb_springs % size)
        for r in range(size)
    ],
    dtype='i'
)

displs = np.zeros(size, dtype='i')
displs[1:] = np.cumsum(counts[:-1])

local_n = int(counts[rank])

local_springs = np.empty(
    (local_n, 4),
    dtype='i'
)

comm.Scatterv(
    [
        springs,
        counts * 4,
        displs * 4,
        MPI.INT
    ] if rank == 0 else None,
    local_springs,
    root=0
)


# --------------------------------------------------
# Préparation des indices des ressorts
# --------------------------------------------------

# On récupère les indices une seule fois.

spring_i = local_springs[:, IDA]
spring_j = local_springs[:, IDB]

spring_dist = local_springs[:, DIST]
spring_stiffness = local_springs[:, STIFFNESS]


# --------------------------------------------------
# Préallocation des tableaux
# --------------------------------------------------

local_force = np.zeros(
    (nb_masses, 2),
    dtype='i'
)

force = np.zeros(
    (nb_masses, 2),
    dtype='i'
)

# Tableaux temporaires pour les interactions

dx = np.empty(local_n, dtype=np.float64)
dy = np.empty(local_n, dtype=np.float64)

dist = np.empty(local_n, dtype=np.float64)
spring_force = np.empty(local_n, dtype=np.float64)


# --------------------------------------------------
# Calcul vectorisé des interactions
# --------------------------------------------------

def calculate_forces(masses, springs_i, springs_j,
                     springs_dist, springs_stiffness,
                     local_force, dx, dy, dist, spring_force):

    local_force.fill(0)

    if len(springs_i) == 0:
        return

    # Différence de positions

    np.subtract(
        masses[springs_i, POSX],
        masses[springs_j, POSX],
        out=dx,
        casting='unsafe'
    )

    np.subtract(
        masses[springs_i, POSY],
        masses[springs_j, POSY],
        out=dy,
        casting='unsafe'
    )

    # Distance entre les masses

    np.hypot(dx, dy, out=dist)

    # Calcul de la force des ressorts

    valid = dist >= 0.1 * UNIT

    spring_force.fill(0)

    spring_force[valid] = (
        springs_stiffness[valid] / UNIT
        *
        (
            dist[valid]
            - springs_dist[valid] / UNIT
        )
    )

    # Composantes des forces
    # Conversion en entiers pour conserver le comportement
    # de interaction() dans spring_lib.py.

    fx = np.zeros(len(springs_i), dtype='i')
    fy = np.zeros(len(springs_i), dtype='i')

    fx[valid] = (
        -dx[valid] * spring_force[valid] / dist[valid]
    ).astype('i')

    fy[valid] = (
        -dy[valid] * spring_force[valid] / dist[valid]
    ).astype('i')

    # Accumulation des forces sur les masses.
    # np.add.at gère correctement les indices répétés.

    np.add.at(local_force[:, 0], springs_i, fx)
    np.add.at(local_force[:, 1], springs_i, fy)

    np.add.at(local_force[:, 0], springs_j, -fx)
    np.add.at(local_force[:, 1], springs_j, -fy)


# --------------------------------------------------
# Mise à jour vectorisée des masses
# --------------------------------------------------

def update_masses(masses, force):

    # Calcul des nouvelles vitesses

    new_speed = (
        masses[:, SPEEDX:SPEEDY + 1].astype(np.float64)
        +
        force.astype(np.float64) / masses[:, WEIGHT, None]
    )

    # Calcul des nouvelles positions

    new_pos = (
        masses[:, POSX:POSY + 1].astype(np.float64)
        + new_speed
    )

    # Limitation des positions

    new_pos = np.clip(
        new_pos,
        -10 * UNIT,
        10 * UNIT
    )

    # Affectation aux tableaux entiers :
    # conversion identique au comportement de update().

    masses[:, POSX:POSY + 1] = new_pos
    masses[:, SPEEDX:SPEEDY + 1] = new_speed


# --------------------------------------------------
# Simulation
# --------------------------------------------------

for turn in range(nb_turns):

    if rank == 0 and display:
        display_plot(masses, springs)

    # Calcul des forces locales vectorisé

    calculate_forces(
        masses,
        spring_i,
        spring_j,
        spring_dist,
        spring_stiffness,
        local_force,
        dx,
        dy,
        dist,
        spring_force
    )

    # Réduction des forces entre processus

    comm.Allreduce(
        local_force,
        force,
        op=MPI.SUM
    )

    # Mise à jour des masses

    update_masses(masses, force)


# --------------------------------------------------
# Statistiques
# --------------------------------------------------

local_count = np.array(
    [local_n * nb_turns],
    dtype='i'
)

if rank == 0:
    all_interactions = np.empty(size, dtype='i')
else:
    all_interactions = None

comm.Gather(
    local_count,
    all_interactions,
    root=0
)


# --------------------------------------------------
# Résultats
# --------------------------------------------------

if rank == 0:

    delta_time = time.time() - time_start

    total_interaction = int(np.sum(all_interactions))
    max_interaction = int(np.max(all_interactions))
    min_interaction = int(np.min(all_interactions))

    unbalance = max_interaction - min_interaction

    print('Duration:', delta_time)
    print('Signature:', signature(masses))
    print('Interactions:', total_interaction)
    print('Unbalance:', unbalance)
