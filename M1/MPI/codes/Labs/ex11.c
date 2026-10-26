#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ROOT 0

int main(int argc, char** argv) {
    int rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int *data = NULL;
    if (rank == ROOT) {
    data = malloc(size*sizeof(int));
        for (int i = 0; i < size; i++) {
            data[i] = 2*i+1;
        }
    }

    int recv_data;
    MPI_Scatter(data, 1, MPI_INT, &recv_data, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    printf("Process %d received data %d\n", rank, recv_data);

    if (rank == ROOT)
    free(data);
    
    MPI_Finalize();
 return 0;
}
