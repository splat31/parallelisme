# python3 ./number.py 42
from mpi4py import MPI
import numpy as np
import sys

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
number = sys.argv[1]


A = np.zeros(1, dtype='f')
        
if rank == 0:
        A[0] = number
	
print ("At start in process of rank ",rank," the passnumber is ",A[0])

comm.Bcast(A, root=0)

print ("After collective in process of rank ",rank," the passnumber is ",A[0])

