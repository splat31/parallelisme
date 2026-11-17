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
    masses, springs = np.zeros((nb_masses,6),dtype="i"), np.zeros((nb_springs,4),dtype="i")

nb_interaction = 0

local_springs = np.zeros((nb_springs//size,4),dtype="i")
comm.Scatter(springs,local_springs,root=0)

comm.Bcast(masses,root=0)

time_start = time.time()
for _ in range(int(nb_turns)):

    force = np.zeros((nb_masses,2),dtype="i")

    for s in local_springs:
        F = interaction(s,masses)
        nb_interaction += 1

        force[s[IDA]] += F
        force[s[IDB]] -= F

    res = np.zeros((nb_masses,2),dtype="i")
    comm.Reduce(force,res,root=0,op=MPI.SUM)
    if rank == 0:
        for i in range(nb_masses):
            update(masses,i,res[i])
    comm.Bcast(masses,root=0)

    if display and rank == 0:
        display_plot(masses, springs)

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
