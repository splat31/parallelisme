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

  // Allocations
  matrice_A = (int*) malloc(SIZE*SIZE*sizeof(int)) ;
  
  
  // Initialisations
  for(i = 0; i < SIZE; i++){
    for(j = 0; j < SIZE; j++){
      matrice_A[i*SIZE + j] = 1;// (int)rand()%10;
    }
  }

  
  start = omp_get_wtime(); //openmp
  #pragma omp parallel num_threads(nb) private(i,j) 
  {
    //reduction pour ajouter a la sommme
    #pragma omp for reduction (+:somme) nowait
    for(i = 0; i < SIZE; i++){
      for(j = 0; j < SIZE; j++){
        somme+=matrice_A[i*SIZE + j];
      }
    }
  }

  
  stop=omp_get_wtime(); //openmp
  t=stop-start;
  printf("%d\t%f\n",nb,t);
  printf("somme =%d\n", somme);
  // Liberations
  free(matrice_A);
  return EXIT_SUCCESS;
}
