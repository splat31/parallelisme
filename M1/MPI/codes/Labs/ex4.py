from mpi4py import MPI
import numpy as np
import sys



def cumul(a,b):
    return sum(range(a,b))

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

nb = int(sys.argv[1])

if rank==0:
    numbersall=np.arange(nb,dtype='i')
else:
    numbersall=np.empty(nb,dtype='i')
numberspart = np.empty(nb//size,dtype='i')

comm.Scatter(numbersall, numberspart, root=0)

total=cumul(numberspart[0],numberspart[nb//size-1]+1)
tabtotal=np.array([total], dtype='i')
result= np.empty(1,dtype='i')
comm.Reduce(tabtotal,result, op=MPI.SUM, root=0) 
if rank == 0:
    print("Cumul is ",result[0])

