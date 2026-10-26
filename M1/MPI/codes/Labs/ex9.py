from mpi4py import MPI
import numpy as np

comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

# We assume n and DIM are constant known by all processes
n = 8
DIM = 3*4*5

if rank == 0:
    np.random.seed(10)
    A = np.random.rand(DIM,DIM)
    res = np.linalg.matrix_power(A, 2**(n-1))
    print('Signature ref', res.trace())

else:
    A = np.zeros((DIM,DIM))

comm.Bcast(A, root=0)

delta = rank*(DIM // size)

for _ in range(n-1):
    local_res = np.zeros((DIM//size, DIM))
    for lin in range(DIM//size):
        for col in range(DIM):
            for i in range(DIM):
                local_res[lin][col] += A[lin+delta][i] * A[i][col]

    comm.Allgather(local_res, A)

if rank==0:
    print('Signature mpi', A.trace())
