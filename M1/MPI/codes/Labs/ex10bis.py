
import sys, time
import numpy as np
from mpi4py import MPI
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

# see primes.py for nb_primes definition
def nb_primes(n):
    result = 0
    for i in range(1, n+1):
        if n%i == 0:
            result += 1
    return result

upper_bound = int(sys.argv[1])

comm.barrier()
start_time = time.time()






local_max = np.zeros(1, dtype='i')
for val in range(rank+1, upper_bound+1, size):
    local_max[0] = max(local_max[0], nb_primes(val))
local_end_time = time.time()

current_max = np.zeros(1, dtype='i')
comm.Reduce(local_max, current_max, root=0, op=MPI.MAX)

end_time = time.time()
print(f'{rank}: {local_end_time-start_time} s')

if rank == 0:
    print(f'Round Robin {current_max[0]} in {end_time - start_time} s')
