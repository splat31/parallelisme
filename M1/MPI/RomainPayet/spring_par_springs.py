#! /usr/bin/env python3
from mpi4py import MPI
comm = MPI.COMM_WORLD
import numpy as np
import time
import sys

from spring_lib import *


rank = comm.Get_rank()
size = comm.Get_size()

nb_masses = np.zeros(1,dtype='i')
nb_springs = np.zeros(1,dtype='i')
nb_turns = np.zeros(1,dtype='i')
display=False

time_start=0

if rank==0:
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
    if nb_springs[0]==-1:
        nb_springs[0]=len(springs)


comm.Bcast(nb_masses,root=0)
comm.Bcast(nb_turns,root=0)
comm.Bcast(nb_springs,root=0)

if rank!=0:
    springs=np.empty((nb_springs[0],4),dtype='i')
    masses=np.empty((nb_masses[0],6),dtype='i')
comm.Bcast(springs,root=0)
comm.Bcast(masses,root=0)

chunk_size = nb_springs[0] // size
start_idx = rank * chunk_size
end_idx = start_idx + chunk_size


l_nb_interaction = np.zeros(1,dtype='i')
total_force = np.zeros((nb_masses[0], 2), dtype='i')
for _ in range(nb_turns[0]):
    l_force = np.zeros((nb_masses[0], 2), dtype='i')
    for s_idx in range(start_idx,end_idx):
        s=springs[s_idx]
        i=s[IDA]
        j=s[IDB]
        F=interaction(s,masses)
        l_nb_interaction[0]+=1
        l_force[i]+=F
        l_force[j]-=F

    comm.Allreduce(l_force, total_force, op=MPI.SUM)

    for i in range(nb_masses[0]):
        update(masses,i,total_force[i])

    if rank==0 and display:
        display_plot(masses, springs)

# all_interactions should contain the number of calls of interaction function on all processes
all_interactions = np.zeros(size,dtype='i')
comm.Gather(l_nb_interaction,all_interactions,root=0)

if rank==0:
    total_interaction = sum(all_interactions)
    max_interaction = max(all_interactions)
    min_interaction = min(all_interactions)
    unbalance = max_interaction-min_interaction
    delta_time = time.time() - time_start
    print('Duration:', delta_time)
    print('Signature:', signature(masses))
    print('Interactions:', total_interaction)
    print('Unbalance:', unbalance)
#    print(all_interactions)
