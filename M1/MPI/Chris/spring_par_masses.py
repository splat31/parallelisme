import numpy as np
import time
import sys
from mpi4py import MPI
from spring_lib import *

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()


try:
    display = not '-nodisplay' in sys.argv
    nb_masses = np.array(int(sys.argv[1]),dtype="i")
    nb_springs = np.array(int(sys.argv[2]),dtype="i")
    nb_turns = np.array(int(sys.argv[3]),dtype="i")
except:
    print(f'Usage {sys.argv[0]} nb_masses nb_strings nb_turns [-nodisplay]')
    sys.exit(0)

if nb_springs == np.array(-1):
    nb_springs = nb_masses * (nb_masses - 1) // 2

if rank == 0:
    masses,springs = generate_springs(int(nb_masses), int(nb_springs))
else:
    masses,springs = np.zeros((nb_masses,6),dtype="i"),np.zeros((nb_springs,4),dtype="i")

nb_interaction = 0

local_masses = np.zeros((nb_masses//size,6),dtype="i")
comm.Scatter(masses,local_masses,root=0)

comm.Bcast(springs,root=0)

time_start = time.time()
for _ in range(nb_turns):

    masses_global = np.zeros((nb_masses, 6),dtype="i")
    comm.Allgather(local_masses, masses_global)

    force = np.zeros((nb_masses//size,2),dtype="i")

    deb = rank * (nb_masses//size)
    fin = rank * (nb_masses//size) + nb_masses//size
    for s in springs:
        F = interaction(s,masses_global)
        nb_interaction += 1

        if deb <= s[IDA] < fin:
            force[s[IDA] - deb] += F
        if deb <= s[IDB] < fin:
            force[s[IDB] - deb] -= F

    for i in range(nb_masses//size):
        update(local_masses,i,force[i])

    if display and rank == 0:
        display_plot(masses, springs)

comm.Allgather(local_masses, masses)

delta_time = time.time() - time_start

all_interactions = np.zeros(size, dtype="i")
comm.Allgather(np.array(nb_interaction, dtype='i'), all_interactions)

if rank == 0:
    total_interaction = sum(all_interactions)
    max_interaction = max(all_interactions)
    min_interaction = min(all_interactions)
    unbalance = max_interaction-min_interaction

    print('Duration:', delta_time)
    print('Signature:', signature(masses))
    print('Interactions:', total_interaction)
    print('Unbalance:', unbalance)
    #print(all_interactions)