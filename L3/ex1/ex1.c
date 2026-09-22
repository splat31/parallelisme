#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#define SIZE 2000

int main(int argc, char **argv){
  int nb, i , j, k;
  double t,start,stop;
  double* matrice_A;
  double* matrice_B;
  double* matrice_res;

  // Allocations
  matrice_A = (double*) malloc(SIZE*SIZE*sizeof(double)) ;
  matrice_B = (double*) malloc(SIZE*SIZE*sizeof(double)) ;
  matrice_res = (double*) malloc(SIZE*SIZE*sizeof(double)) ;

  // Initialisations
  for(i = 0; i < SIZE; i++){
    for(j = 0; j < SIZE; j++){
      matrice_A[i*SIZE + j] = (double)rand()/(double)RAND_MAX;
      matrice_B[i*SIZE + j] = (double)rand()/(double)RAND_MAX;
    }
  }

  // Calcul en faisant varier le nombre de threads
  printf("Nb.threads\tTps.\n");
  for(nb=8;nb<=18;nb++){      
    start = omp_get_wtime(); //openmp
    #pragma omp parallel for num_threads(nb) private(j,k) //openmp
    for(i = 0; i < SIZE; i++){
      for(j = 0; j < SIZE; j++){
        matrice_res[i*SIZE + j] = 0.0;
        for(k = 0; k < SIZE; k++){
          matrice_res[i*SIZE + j] += (matrice_A[i*SIZE + k]*matrice_B[k*SIZE + j]);
        }
      }
    }
    stop=omp_get_wtime(); //openmp
    t=stop-start;
    printf("%d\t%f\n",nb,t);
  }

  // Liberations
  free(matrice_A);
  free(matrice_B);
  free(matrice_res);
  return EXIT_SUCCESS;
}
/*
Nb.threads	Tps.
1	25.368863
2	14.114263
3	9.235109
4	6.934251
5	5.492564
6	4.599046
7	3.892419
8	3.525214
9	3.119089
10	3.095529
11	2.700768
12	2.422343
13	2.354726
14	2.238173
15	2.395643
16	2.472059
17	2.470181
18	2.450258



plus on a de thread plus on accelere jusqua arriver au nombre de coeur (machine de tp 14 coeur)
*/