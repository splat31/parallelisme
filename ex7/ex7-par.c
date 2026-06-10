#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define MAX_ITER 1000
#define EPSILON 1e-6

typedef struct {
    double x;
    double y;
    double z;
} Point;

typedef struct {
    double x;
    double y;
    double z;
    int nb_elem;
} Cluster;

//NB : distance euclidienne entre deux points A et B = sqrt ( (xA-xB)2+ (yA-yB)2+ (zA-zB)2 )
double distance_eucli(Point p, Cluster c){
    double dx = p.x - c.x;
    double dy = p.y - c.y;
    double dz = p.z - c.z;

    return dx * dx + dy * dy + dz * dz;
}

int main(int argc, char *argv[]){
    if (argc != 4) {
        fprintf(stderr,"Usage : ./seq <fichier> <nb_clusters> <nbthreads>\n");
        exit(-1);
    }

    char *filename = argv[1];
    int k = atoi(argv[2]);
    int nb = atoi(argv[3]);

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Erreur ouverture fichier");
        exit(-1);
    }


    int n;
    if (fscanf(file, "%d", &n) != 1) {
        fprintf(stderr, "Erreur format fichier ena rgument non reconnue\n");
        fclose(file);
        exit(-1);
    }


    Point *points = malloc(n * sizeof(Point));

    for (int i = 0; i < n; i++) {
        if (fscanf(file,"%lf %lf %lf", &points[i].x, &points[i].y, &points[i].z) != 3) {
            fprintf(stderr,"Erreur lecture des point: %d\n",i);
            free(points);
            fclose(file);
            exit(-1);
        }
    }
    fclose(file);


    Cluster *clusters = malloc(k * sizeof(Cluster));
    int *affectation = malloc(n * sizeof(int));

    double *sumx = malloc(k * sizeof(double));
    double *sumy = malloc(k * sizeof(double));
    double *sumz = malloc(k * sizeof(double));
    int *count = malloc(k * sizeof(int));

    double start, stop ,t;
    start = omp_get_wtime();

    #pragma omp parallel for num_threads(nb)
    for (int i = 0; i < k; i++) {
        clusters[i].x = points[i].x;
        clusters[i].y = points[i].y;
        clusters[i].z = points[i].z;
        clusters[i].nb_elem = 0;
    }

    #pragma omp parallel for num_threads(nb)
    for (int i = 0; i < n; i++) {
        affectation[i] = -1;
    }

    int it = 0;
    double variation;

    do {
        #pragma omp parallel num_threads(nb) 
        {
            #pragma omp for
            for (int c = 0; c < k; c++) {
                sumx[c] = 0.0;
                sumy[c] = 0.0;
                sumz[c] = 0.0;
                count[c] = 0;
            }

            double localx[k];
            double localy[k];
            double localz[k];
            int localc[k];
            for (int i = 0; i < k; i++) {
                localx[i] = 0.0;
                localy[i] = 0.0;
                localz[i] = 0.0;
                localc[i] = 0;
            }
            #pragma omp for nowait
            for (int i = 0; i < n; i++) {
                int best_cluster = 0;
                double best_dist = distance_eucli(points[i], clusters[0]);

                for (int c = 1; c < k; c++) {
                    double d = distance_eucli(points[i], clusters[c]);

                    if (d < best_dist) {
                        best_dist = d;
                        best_cluster = c;
                    }
                }

                affectation[i] = best_cluster;
               
                localx[best_cluster] += points[i].x;
                localy[best_cluster] += points[i].y;
                localz[best_cluster] += points[i].z;
                localc[best_cluster]++;
                
            }


            #pragma omp critical 
            {
                for(int i = 0;i<k;i++) {
                    sumx[i] += localx[i];
                    sumy[i] += localy[i];
                    sumz[i] += localz[i];
                    count[i]+= localc[i];
                }
            }


            #pragma omp single
            variation = 0.0;
            #pragma omp for reduction(+:variation)
            for (int c = 0; c < k; c++) {
                double old_x = clusters[c].x;
                double old_y = clusters[c].y;
                double old_z = clusters[c].z;

                if (count[c] > 0) {
                    clusters[c].x = sumx[c] / count[c];
                    clusters[c].y = sumy[c] / count[c];
                    clusters[c].z = sumz[c] / count[c];
                }

                clusters[c].nb_elem = count[c];

                variation += fabs(clusters[c].x - old_x);
                variation += fabs(clusters[c].y - old_y);
                variation += fabs(clusters[c].z - old_z);
            }
        }
        it++;
    } while (variation > EPSILON && it < MAX_ITER);
    stop = omp_get_wtime();
    t = stop - start;



    //Affichage pour les tests (copie colle de moodle)
    printf("it : %d\n", it);
    for (int c = 0; c < k; c++) {
        printf("cluster %d : ""x=%lf y=%lf z=%lf nb_elem=%d\n",c,clusters[c].x,clusters[c].y,clusters[c].z,clusters[c].nb_elem);
    }
    printf("\n\nTemps d'exécution = %f\n" , t);

    free(points);
    free(clusters);
    free(affectation);
    free(sumx);
    free(sumy);
    free(sumz);
    free(count);

    return 0;
}