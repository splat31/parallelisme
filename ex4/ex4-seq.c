#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>

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
    
    int S[N][C+1];
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
    printf(" objet | M[i] | U[i] |");
    for (int i = 0;i<C+1;i++) {
        printf(" %d |",i);
    }
    printf("\n");
    for (int i = 0;i<N;i++) {
        printf(" %5d | %4d | %4d |",i,M[i],U[i]);
        for (int j = 0;j <C+1;j++) {
            printf(" %d |",S[i][j]);
        }
        printf("\n");
    }
    int Umax = S[N-1][C];
    int capactu = C;
    int E[N];

    /* initialize E */
    for (int i = 0; i < N; i++) {
        E[i] = 0;
    }

    int c = C;

    /* reconstruct solution */
    for (int i = N-1; i > 0; i--) {

        if (S[i][c] != S[i-1][c]) {
            E[i] = 1;
            c = c - M[i];
        }
    }

    /* handle object 0 separately */
    if (c >= M[0] && S[0][c] > 0) {
        E[0] = 1;
    }

    printf("Les objets pris sont: ");
    for (int i = 0;i<N;i++) {
        if (E[i]==1) {
             printf(" %d",i);
        }
    }
    printf("\n");
    return 0;
}