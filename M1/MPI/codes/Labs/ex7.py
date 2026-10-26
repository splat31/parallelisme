from mpi4py import MPI
import time, random
import numpy as np
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

nb = 100001
inside = np.zeros(1, dtype='i')

start_time = time.time()
#
local_nb = nb // size
if rank == size - 1:
    local_nb += nb % size
    
for _ in range(local_nb):
    x,y  = random.random(), random.random()
    if x*x + y*y <= 1:
        inside +=1

result = np.zeros(1, dtype='i')
comm.Reduce(inside, result, op = MPI.SUM, root=0)
#
end_time = time.time()

print(f'Process {rank} tried {local_nb} times')
if rank == 0:
    print("Pi =", 4 * float(result[0])/nb, "in ", end_time-start_time, 'seconds')
