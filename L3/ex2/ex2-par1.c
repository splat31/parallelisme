#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>
#define SIZE 20000



int main(int argc, char *argv[]){

  if (argc!=2 ) {
    fprintf(stderr, "Usage ./ex2-par1 nbthreads\n"); //nbthread: nombre de thread qui seront utilisé pour le parallèlisme
    exit(1);
  }

  int nb, i , j, somme=0;
  nb=atoi(argv[1]);
  double t,start,stop;
  int* matrice_A;

  // Création d'un tableau de sommes partielles
  int spartielle[nb][16];
  for (int i = 0; i < nb; i++) {
    spartielle[i][0] = 0;
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
    int id = omp_get_thread_num();//recupere l'id pour savoir quelle partie de tableau le thread va utiliser
    #pragma omp for
    for(i = 0; i < SIZE; i++){
      for(j = 0; j < SIZE; j++){
        spartielle[id][0]+=matrice_A[i*SIZE + j];
      }
    }

    //Un thread s'occupe de calculer la somme finale après la synchro implicite du for
    #pragma omp single 
    {
      for (int a = 0;a<nb;a++) {
        somme+=spartielle[a][0];
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

