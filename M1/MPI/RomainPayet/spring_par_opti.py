#! /usr/bin/env python3
from mpi4py import MPI
comm = MPI.COMM_WORLD
import numpy as np
import time
import sys

from spring_lib import *

rank = comm.Get_rank()
size = comm.Get_size()

nb_masses = np.zeros(1, dtype='i')
nb_springs = np.zeros(1, dtype='i')
nb_turns = np.zeros(1, dtype='i')
display = False

time_start = 0

if rank == 0:
    try:
        display = not '-nodisplay' in sys.argv
        nb_masses[0] = int(sys.argv[1])
        nb_springs[0] = int(sys.argv[2])
        nb_turns[0] = int(sys.argv[3])
    except:
        print(f'Usage {sys.argv[0]} nb_masses nb_strings nb_turns [-nodisplay]')
        sys.exit(0)

    masses, springs = generate_springs(nb_masses[0], nb_springs[0])
    time_start = time.time()
    if nb_springs[0] == -1:
        nb_springs[0] = len(springs)
else:
    springs = None

# Diffusion des paramètres scalaires (regroupables ou individuels)
comm.Bcast(nb_masses, root=0)
comm.Bcast(nb_turns, root=0)
comm.Bcast(nb_springs, root=0)

m_chunk_size = nb_masses[0] // size
m_start_idx = rank * m_chunk_size
m_end_idx = m_start_idx + m_chunk_size

s_chunk_size = nb_springs[0] // size

if rank != 0:
    masses = np.empty((nb_masses[0], 6), dtype='i')

# 1. Seules les masses complètes sont diffusées ; les ressorts sont distribués via Scatter
comm.Bcast(masses, root=0)

l_springs = np.empty((s_chunk_size, 4), dtype='i')
comm.Scatter(springs, l_springs, root=0)

l_nb_interaction = np.zeros(1, dtype='i')

# 2. Pré-allocation des buffers hors de la boucle temporelle
l_force = np.zeros((nb_masses[0], 2), dtype= np.float64)
my_force = np.zeros((m_chunk_size, 2), dtype= np.float64)
total_force = np.zeros((nb_masses[0], 2), dtype=np.float64)
for _ in range(nb_turns[0]):
    l_force.fill(0)

    # Calcul parfaitement équilibré sur le sous-tableau local de ressorts
    for s in l_springs:
        i = s[IDA]
        j = s[IDB]
        F = interaction(s, masses)
        l_nb_interaction[0] += 1
        l_force[i] += F
        l_force[j] -= F

    comm.Allreduce(l_force, total_force, op=MPI.SUM)
    
    # On isole la partie qui appartient au processus courant
    my_force = total_force[m_start_idx:m_end_idx]

    # Mise à jour uniquement sur le sous-tableau de masses local
    for idx, i in enumerate(range(m_start_idx, m_end_idx)):
        update(masses, i, my_force[idx])

    # Synchronisation des positions mises à jour
    comm.Allgather(masses[m_start_idx:m_end_idx], masses)

    if rank == 0 and display:
        display_plot(masses, springs)

all_interactions = np.zeros(size, dtype='i')
comm.Gather(l_nb_interaction, all_interactions, root=0)

if rank == 0:
    total_interaction = sum(all_interactions)
    max_interaction = max(all_interactions)
    min_interaction = min(all_interactions)
    unbalance = max_interaction - min_interaction
    delta_time = time.time() - time_start
    print('Duration:', delta_time)
    print('Signature:', signature(masses))
    print('Interactions:', total_interaction)
    print('Unbalance:', unbalance)