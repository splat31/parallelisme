# python3 ./number.py 42
from mpi4py import MPI
import numpy as np

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

# try with and without commenting the next line
#np.random.seed(0)
numbers = np.random.randint(100, size=10, dtype='i')
min = 100
max = 0
for i in range(10):
    if numbers[i]>max:
        max = numbers[i]
    if numbers[i]<min:
        min = numbers[i]

tabmax = np.array([max])
tabmin = np.array([min])

resultmax = np.empty(size,dtype='i')
resultmin = np.empty(size,dtype='i')
comm.Gather(tabmax, resultmax, root=0)
comm.Gather(tabmin, resultmin, root=0)

if rank==0:
    
    good = True
    for i in range(size-1):
        if (resultmin[i]!=resultmin[i+1]):
            good = False
        if (resultmax[i]!=resultmax[i+1]):
            good = False
    print("Min et Max sont pareils ? ",good)


