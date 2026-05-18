#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdbool.h>
#define MAX_N 100

void read_problem(char *filename);

int N = 0;
int C;
int M[MAX_N];
int U[MAX_N];


bool Verify(int *E) {
    for (int i = 0;i<N;i++) {
        if (E[i]!= 1) {
            return false;
        }
    }
    return true;
}


double seq(){
    double start,stop,t;
    start = omp_get_wtime(); //openmp

    int (*S)[C+1] = malloc(sizeof(int[N][C+1]));//Allocation de S
    if (S == NULL) {
        fprintf(stderr, "Erreur de malloc\n");
        exit(1);
    }

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

    if (!Verify(E)) {
        fprintf(stderr, "Séquentielle a fait une erreur\n");
        free(S);
        exit(2);
    }
    free(S);
    return t;
}




double par(int nb) {
    double start,stop,t;
    start = omp_get_wtime(); //openmp
    int (*S)[C+1] = malloc(sizeof(int[N][C+1])); //Allocation de S
    if (S == NULL) {
        fprintf(stderr, "Erreur de malloc\n");
        exit(1);
    }

    //Calcul de S
    #pragma omp parallel num_threads(nb)
    {
        #pragma omp for nowait
        for (int j = 0; j<C+1;j++) {
            if (M[0]<=j) {
                S[0][j]=U[0];
            } else {
                S[0][j]=0;
            }
        }    
        for (int i = 1;i<N;i++) {
            #pragma omp for 
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
            E[0]=0;
        }
    }
    //Cas 0
    if (Cactu >= M[0] && S[0][Cactu] > 0) {
        E[0] = 1;
    }
    stop=omp_get_wtime(); //openmp
    t=stop-start;
    
    if (!Verify(E)) {
        fprintf(stderr, "Parrallèle a fait une erreur\n");
        free(S);
        exit(2);
    }
    free(S);
    return t;
}













int main (int argc, char * argv[]) {
    if (argc!=3) {
        fprintf(stderr, "Usage ./ex4-par nbthreads nbthreadsmoy\n"); 
        exit(1);
    }
    read_problem("pb6.txt");
    int nbmax = atoi(argv[1]);
    int nbmoy = atoi(argv[2]);

    double tseq = 0, tpar;
    for(int k=0;k<nbmoy;k++) {
        tseq += seq();
    }
    tseq = tseq/nbmoy;
    printf("\nTemps d'exécution pour la version séquentielle\n\t-séquentielle: %f\n\n",tseq);

    for (int i =1;i<=nbmax;i++) {
        tpar = 0;
        for(int k=0;k<nbmoy;k++) {
            tpar+=par(i);
        }
        tpar = tpar/nbmoy;

        printf("Temps d'exécution de la version parallèle avec %d threads:\n\t-parallèle: %f\n\n",i,tpar);
    }

    return 0;
}












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
  