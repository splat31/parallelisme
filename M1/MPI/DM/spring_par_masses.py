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
    nb_springs_array = np.array([len(springs)], dtype='i')
else:
    masses = None
    springs = None
    nb_springs_array = np.empty(1, dtype='i')

comm.Bcast(nb_springs_array, root=0)
nb_springs = int(nb_springs_array[0])

if rank != 0:
    springs = np.empty((nb_springs, 4), dtype='i')

comm.Bcast(springs, root=0)


#Répartition des masses et indices
local_n = nb_masses // size
local_masses = np.empty((local_n, 6), dtype='i')
comm.Scatter(masses, local_masses, root=0)
all_masses = np.empty((nb_masses, 6), dtype='i')
start = rank * local_n
end = start + local_n



nb_interaction = 0

for _ in range(nb_turns):
    comm.Allgather(local_masses, all_masses)

    if rank == 0 and display:
        display_plot(all_masses, springs)

    local_force = np.zeros((local_n, 2), dtype=float)

    for s in springs:
        i = int(s[IDA])
        j = int(s[IDB])

        if start <= i < end or start <= j < end:
            F = interaction(s, all_masses)
            nb_interaction += 1
            if start <= i < end:
                local_force[i - start] += F
            if start <= j < end:
                local_force[j - start] -= F

    for k in range(local_n):
        update(local_masses, k, local_force[k])



if rank == 0:
    final_masses = np.empty((nb_masses, 6), dtype='i')
else:
    final_masses = None
comm.Gather(local_masses, final_masses, root=0)


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
    print('Signature:', signature(final_masses))
    print('Interactions:', total_interaction)
    print('Unbalance:', unbalance)
