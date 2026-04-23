#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>
#define SIZE 20000

int main(int argc, char *argv[]){

  if (argc!=2 ) {
    fprintf(stderr, "Usage ./ex2-par1 nbthreads\n");
    return 1;
  }

  int nb, i , j, somme=0;
  nb=atoi(argv[1]);
  double t,start,stop;
  int* matrice_A;

  int spartielle[nb];
  for (int i = 0; i < nb; i++) {
    spartielle[i] = 0;
  }
  // Allocations
  matrice_A = (int*) malloc(SIZE*SIZE*sizeof(int)) ;
  
  
  // Initialisations
  for(i = 0; i < SIZE; i++){
    for(j = 0; j < SIZE; j++){
      matrice_A[i*SIZE + j] = 1;
    }
  }

  // Calcul en faisant varier le nombre de threads    
  start = omp_get_wtime(); //openmp
  #pragma omp parallel num_threads(nb) private(i,j)
  {
    int id = omp_get_thread_num();
    #pragma omp for
    for(i = 0; i < SIZE; i++){
      for(j = 0; j < SIZE; j++){
        spartielle[id]+=matrice_A[i*SIZE + j];
        
      }
    }
  }

  for (int a = 0;a<nb;a++) {
        somme+=spartielle[a];
      }
  stop=omp_get_wtime(); //openmp
  t=stop-start;
  printf("%d\t%f\n",nb,t);
  printf("somme =%d\n", somme);
  // Liberations
  free(matrice_A);
  return EXIT_SUCCESS;
}

