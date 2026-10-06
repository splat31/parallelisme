

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define BSIZE 1024

__global__ void kreduce( unsigned int *d_vec,unsigned int *d_partial,int size) {
    __shared__ unsigned int shared[BSIZE];
    
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int idx = threadIdx.x;

    // Charger les données en shared
    if (i < size)
        shared[idx] = d_vec[i];
    else
        shared[idx] = 0;

    __syncthreads();

    // Réduction du bloc
    for (int offset = blockDim.x / 2; offset >= 1; offset /= 2)
    {
        if (idx < offset) {
            shared[idx] += shared[idx + offset];
        }
        __syncthreads();
    }

    // Un seul thread écrit la somme du bloc
    if (idx == 0) {
        d_partial[blockIdx.x] = shared[0];
    }


}

void reduce( unsigned int *vec,  unsigned int*sum,int size) {

    unsigned int *d_vec;
    int bytes = size*sizeof( unsigned int);
    unsigned int *d_partial;
    int nbbloc = (size + BSIZE -1)/BSIZE;

    cudaMalloc((void **)&d_partial, nbbloc * sizeof(unsigned int));
    cudaMalloc((void **)&d_vec,bytes);
    cudaMemcpy(d_vec,vec,bytes,cudaMemcpyHostToDevice);

    printf("nombre de bloc = %d d'une taille de %d thread pour un total de %d\n",nbbloc, BSIZE,size);
    kreduce <<<nbbloc,BSIZE>>>(d_vec,d_partial, size);

    unsigned int *partial =
        (unsigned int *)malloc(nbbloc * sizeof(unsigned int));
    cudaMemcpy(partial, d_partial,
           nbbloc * sizeof(unsigned int),
           cudaMemcpyDeviceToHost);
    for (int i = 0; i < nbbloc; i++) {
    *sum += partial[i];
    }
    cudaFree(d_partial);
    cudaFree(d_vec);
    free(partial);
}



int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: <filename>\n");
        exit(-1);
    }
    unsigned int log2size, size;
    unsigned int *vec;
    FILE *f = fopen(argv[1], "r");
    fscanf(f, "%d\n", &log2size);
    if (log2size > 30) {
        printf("Size (%u) is too large: size is limited to 2^30\n", log2size);
        exit(-1);
    }
    size = 1 << log2size;
    unsigned int bytes = size * sizeof(unsigned int);
    vec = (unsigned int *) malloc(bytes);
    assert(vec);
    for (unsigned int i = 0; i < size; i++) {
        fscanf(f, "%u\n", &(vec[i]));
    }
    fclose(f);

    //fin main de base

    
    unsigned int sum = 0;
    reduce (vec,&sum,size);
    printf("sum = %d\n",sum);
    return 0;
}
