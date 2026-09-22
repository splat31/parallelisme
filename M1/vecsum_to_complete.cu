

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define BSIZE 1024

__global__ void kreduce( unsigned int *d_vec,int size) {
    int idx = threadIdx.x;
    for (int offset = size/2 ; offset >= 1; offset/=2) {
        if (idx < offset) {
            d_vec[idx] = d_vec[idx] + d_vec[idx + offset];
        }
    }
    __syncthreads();
}

void reduce( unsigned int *vec,  unsigned int*sum,int size) {
    unsigned int *d_vec;
    int bytes = size*sizeof( unsigned int);

    cudaMalloc((void **)&d_vec,bytes);
    cudaMemcpy(d_vec,vec,bytes,cudaMemcpyHostToDevice);
    kreduce <<<1,size>>>(d_vec, size);

    cudaMemcpy(sum,d_vec,sizeof( unsigned int),cudaMemcpyDeviceToHost);
    cudaFree(d_vec);
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
    if (log2size > 10) {
        printf("Size (%u) is too large: size is limited to 2^10\n", log2size);
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

    
    unsigned int sump = 0;
    reduce (vec,&sump,size);
    printf("sum = %d\n",sump);
    return 0;
}
