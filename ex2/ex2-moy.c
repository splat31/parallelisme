#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <sys/time.h>
#define SIZE 20000

//Version sequentielle
double seq(int* matrice_A) {
    int i , j, somme=0;
    double start,stop,t;
    start = omp_get_wtime(); //openmp
    for(i = 0; i < SIZE; i++){
        for(j = 0; j < SIZE; j++){
            somme+=matrice_A[i*SIZE + j];
        }
    }
    stop=omp_get_wtime(); //openmp
    t=stop-start;
    //vérifie que la somme a bien était calculer comme on met que des 1 dans la matrice
    if (somme!=SIZE*SIZE) {
        fprintf(stderr, "Seq a fait un mauvais calcul de somme\n");
        exit(2);
    }
    return t;
}

//version partielle 1
double par1(int* matrice_A, int nb) {
    int i , j, somme=0;
    double start,stop,t;
    //recupere l'id pour savoir quelle partie de tableau le thread va utiliser
    int spartielle[nb][16];
    for (int i = 0; i < nb; i++) {
        spartielle[i][0] = 0;
    }

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


    start = omp_get_wtime(); //openmp
    //vérifie que la somme a bien était calculer comme on met que des 1 dans la matrice
    if (somme!=SIZE*SIZE) {
        fprintf(stderr, "Par1 a fait un mauvais calcul de somme\n");
        exit(2);
    }
    return t;
}

double par2(int* matrice_A,int nb) {
    int i , j, spartielle,somme=0;
    double start,stop,t;
    start = omp_get_wtime(); //openmp
    #pragma omp parallel num_threads(nb) private(i,j,spartielle)
    {
        spartielle=0;
        #pragma omp for nowait
            for(i = 0; i < SIZE; i++){
                for(j = 0; j < SIZE; j++){
                    spartielle+=matrice_A[i*SIZE + j];
                }
        }
        //section critique 
        #pragma omp atomic
        somme+=spartielle;
    }

    
    stop=omp_get_wtime(); //openmp
    t=stop-start;
    //vérifie que la somme a bien était calculer comme on met que des 1 dans la matrice
    if (somme!=SIZE*SIZE) {
        fprintf(stderr, "Par2 a fait un mauvais calcul de somme\n");
        exit(2);
    }
    return t;
}

double par3(int* matrice_A,int nb) {
    int i , j, somme=0;  
    double start,stop,t;
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
    //vérifie que la somme a bien était calculer comme on met que des 1 dans la matrice
    if (somme!=SIZE*SIZE) {
        fprintf(stderr, "Par3 a fait un mauvais calcul de somme\n");
        exit(2);
    }
    return t;
}

int main(int argc, char *argv[]){

    if (argc!=3 ) {
        fprintf(stderr, "Usage ./ex2-par1 nbthreads nbthreadsmoy\n");
        exit(1);
    }

    int nbmax,nbmoy;
    nbmax=atoi(argv[1]);
    nbmoy=atoi(argv[2]);
    double tseq = 0 , tpar1 , tpar2 ,tpar3;
    int* matrice_A;

    // Allocations
    matrice_A = (int*) malloc(SIZE*SIZE*sizeof(int)) ;
    
    
    // Initialisations
    for(int i = 0; i < SIZE; i++){
        for(int j = 0; j < SIZE; j++){
        matrice_A[i*SIZE + j] = 1;
        }
    }

    //Calcul séquentielle
    for(int k=0;k<nbmoy;k++) {
        tseq += seq(matrice_A);
    }
    tseq = tseq/nbmoy;
    printf("\nTemps d'exécution pour la version séquentielle\n\t-séquentielle: %f\n\n",tseq);

    //Calculs des versions parallèles
    for (int i =1;i<=nbmax;i++) {
        tpar1 = 0;
        tpar2 = 0;
        tpar3 = 0;
        for(int k=0;k<nbmoy;k++) {
            tpar1+=par1(matrice_A,i);
        }
        tpar1 = tpar1/nbmoy;

        for(int k=0;k<nbmoy;k++) {
            tpar2+=par2(matrice_A,i);
        }
        tpar2 = tpar2/nbmoy;

        for(int k=0;k<nbmoy;k++) {
            tpar3+=par3(matrice_A,i);
        }
        tpar3 = tpar3/nbmoy;

        printf("Temps d'exécution des versions parallèles avec %d threads:\n\t-parallèle version 1: %f\n\t-parallèle version 2: %f\n\t-parallèle version 3: %f\n\n",i,tpar1,tpar2,tpar3);
    }
    
    // Liberations
    free(matrice_A);
    return EXIT_SUCCESS;
}