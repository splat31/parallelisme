#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>
#define SIZE 20000

int main(int argc, char **argv){
  int nb, i , j, somme;
  double t,start,stop;
  int* matrice_A;

  // Allocations
  matrice_A = (int*) malloc(SIZE*SIZE*sizeof(int)) ;
  
  
  // Initialisations
  for(i = 0; i < SIZE; i++){
    for(j = 0; j < SIZE; j++){
      matrice_A[i*SIZE + j] = (int)rand()%10;
    }
  }

  // Calcul en faisant varier le nombre de threads    
  start = omp_get_wtime(); //openmp
  for(i = 0; i < SIZE; i++){
    for(j = 0; j < SIZE; j++){
      somme+=matrice_A[i*SIZE + j];
      
    }
  }
  stop=omp_get_wtime(); //openmp
  t=stop-start;
  printf("%d\t%f\n",nb,t);
  printf("somme =%d", somme);
  // Liberations
  free(matrice_A);
  return EXIT_SUCCESS;
}