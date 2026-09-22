#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_N 100

int N = 0;
int C;
int M[MAX_N];
int U[MAX_N];

void read_problem(char *filename){
  char line[256];

  FILE *problem = fopen(filename,"r");
  if (problem == NULL){
    fprintf(stderr,"File %s not found.\n",filename);
    exit(-1);
  }

  while (fgets(line, 256, problem) != NULL){
    switch(line[0]){
    case 'c': // C
      if (sscanf(&(line[2]),"%d\n", &C) != 1){
	fprintf(stderr,"Error in file format in line:\n");
	fprintf(stderr, "%s", line);
	exit(-1);
      }
      break;

    case 'o': // graph size
      if (N >= MAX_N){
	fprintf(stderr,"Too many objects (%d): limit is %d\n", N, MAX_N);
	exit(-1);
      }
      if (sscanf(&(line[2]),"%d %d\n", &(M[N]), &(U[N])) != 2){
	fprintf(stderr,"Error in file format in line:\n");
	fprintf(stderr, "%s", line);
	exit(-1);
      }
      else
	N++;
      break;

    default:
      break;
    }
  }
  if (N == 0){
    fprintf(stderr,"Could not find any object in the problem file. Exiting.");
    exit(1);
  }

}


int main (int argc, char * argv[]) {
    if (argc!=2) {
        fprintf(stderr, "Usage ./ex4-seq filename\n"); 
        exit(1);
    }
    read_problem(argv[1]);
    double start,stop,t;
    start = omp_get_wtime(); //openmp

    int (*S)[C+1] = malloc(sizeof(int[N][C+1]));//Allocation de S

    //Calcul de S
    for (int j = 0; j<C+1;j++) {
        if (M[0]<=j) {
            S[0][j]=U[0];
        } else {
            S[0][j]=0;
        }
    }
    for (int i = 1;i<N;i++) {
        for (int j = 0;j<C+1;j++) {
            if (M[i]<=j) {
                if (S[i-1][j]<S[i-1][j-M[i]]+ U[i]) {
                    S[i][j] = S[i-1][j-M[i]] + U[i];
                } else {
                    S[i][j] = S[i-1][j];
                }
            } else {
                S[i][j] = S[i-1][j];
            }
        }
    }
  
    //E stockera l'information de si les objets sont pris ou non (1 = oui, 0 = non)
    int E[N];

    int Cactu = C;
    //Attribue 1 à l'indice i de E si l'objet i a été pris et 0 sinon 
    for (int i = N-1; i > 0; i--) {

        if (S[i][Cactu] != S[i-1][Cactu]) {
            E[i] = 1;
            Cactu = Cactu - M[i];
        } else { 
            E[i]=0;
        }
    }
    //Cas 0
    if (Cactu >= M[0] && S[0][Cactu] > 0) {
        E[0] = 1;
    }
    stop=omp_get_wtime(); //openmp
    t=stop-start;
    printf("Temps d'exécution version séquentielle : %f\n",t);
    printf("Les objets pris sont:");
    for (int i = 0;i<N;i++) {
        if (E[i]==1) {
             printf(" %d",i);
        }
    }
    printf("\n");
    free(S);
    return 0;
}