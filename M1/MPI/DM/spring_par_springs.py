#python3 spring_seq.py 8 -1 100
#! /usr/bin/env python3

import numpy as np
import time
import sys
from spring_lib import *
from mpi4py import MPI

#structure data:
#       -0 -> ID        (ID de la masse)
#       -1 -> POSX      (Position horizontale)
#       -2 -> POSY      (Position vertticale)
#       -3 -> SPEEDX    (Vitesse horizontale)   
#       -4 -> SPEEDY    (Vitesse verticale)
#       -5 -> weight    (Masse)



#Main
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

if rank ==0:
    try:
        display = not '-nodisplay' in sys.argv
        nb_masses = int(sys.argv[1])
        nb_springs = int(sys.argv[2])
        nb_turns = int(sys.argv[3])
        params = np.array([nb_masses, nb_springs, nb_turns, display, 1], dtype='i')
    except:
        params = np.zeros(5,dtype='i')
else:
    params = np.empty(5,dtype='i')

comm.Bcast(params, root=0)

#Verifie si les arguments ont bien ete recu par 0. Arrete tout sinon
if params[4] == 0 :
    if rank == 0:
         print(f'Usage {sys.argv[0]} nb_masses nb_strings nb_turns [-nodisplay]')
    sys.exit(1)






nb_masses, nb_springs, nb_turns, display,paramok = params
if rank == 0:
    masses, springs = generate_springs(nb_masses, nb_springs)
    time_start = time.time()
else:
    masses = np.empty((nb_masses, 6), dtype='i')
    springs = None

if rank == 0:
    nb_springs_array = np.array([len(springs)], dtype='i')
else:
    nb_springs_array = np.empty(1, dtype='i')

comm.Bcast(nb_springs_array, root=0)

nb_springs = nb_springs_array[0]

comm.Bcast(masses, root=0)



# 3. Répartition des springs et indices
local_n = nb_springs // size
local_springs = np.empty((local_n, 4), dtype='i')
comm.Scatter(springs, local_springs, root=0)


nb_interaction = 0

for _ in range(nb_turns):

    if rank == 0 and display:
        display_plot(masses, springs)

    local_force = np.zeros((nb_masses, 2), dtype=float)

    for s in local_springs:
        i = int(s[IDA])
        j = int(s[IDB])

        F = interaction(s, masses)
        local_force[i] += F
        local_force[j] -= F
        nb_interaction += 1

    force = np.zeros_like(local_force)
    comm.Allreduce(local_force, force, op=MPI.SUM)
    for k in range(nb_masses):
        update(masses, k, force[k])



# nb interactions
local_count = np.array([nb_interaction], dtype='i')
if rank == 0:
    all_interactions = np.empty(size, dtype='i')
else:
    all_interactions = None
comm.Gather(local_count, all_interactions, root=0)





if rank==0:
    delta_time = time.time() - time_start
    total_interaction = sum(all_interactions)
    max_interaction = max(all_interactions)
    min_interaction = min(all_interactions)
    unbalance = max_interaction-min_interaction
    print('Duration:', delta_time)
    print('Signature:', signature(masses))
    print('Interactions:', total_interaction)
    print('Unbalance:', unbalance)
