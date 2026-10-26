from mpi4py import MPI
comm = MPI.COMM_WORLD
import numpy as np
rank = comm.Get_rank()
size = comm.Get_size()

def get_max(tab):
    pos = np.argmax(tab)
    return [tab[pos], pos]

SIZE=12
if rank == 0:
    np.random.seed(42)
    tab = np.random.randint(100, size = SIZE, dtype='i')
else:
    tab = None

l_tab = np.empty(SIZE//size, dtype='i')


comm.Scatter(tab, l_tab, root=0)

l_res = np.array(get_max(l_tab), dtype='i')

res = np.empty(2*size, dtype='i')
comm.Gather(l_res, res, root=0)

if rank==0:
    c_max, c_pos = res[0], res[1]
    for i in range(size):
        if res[2*i] > c_max:
            c_max = res[2*i]
            c_pos = res[2*i+1]+i*SIZE//size

    print(c_pos)
