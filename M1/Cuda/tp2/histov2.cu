#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "cuda.h"


// ==============================================
// Lecture du fichier
// ==============================================
char *read_text(char *filename, int *num)
{
    char *text = (char *)malloc((*num + 1) * sizeof(char));

    if (text == NULL)
        return NULL;

    FILE *f = fopen(filename, "r");

    if (f == NULL) {
        printf("File %s could not be opened\n", filename);
        free(text);
        return NULL;
    }

    int i = 0;
    char c;

    while ((i < *num) && (c = fgetc(f)) != EOF) {
        text[i] = c;
        i++;
    }

    *num = i;
    text[i] = 0;

    fclose(f);

    return text;
}


// ==============================================
// VERSION 2
//
// Chaque bloc possède un histogramme local
// dans la mémoire partagée.
//
// Les threads du bloc remplissent cet histogramme
// local, puis l'histogramme local est ajouté à
// l'histogramme global.
// ==============================================
__global__
void compute_histogram(char *text, int num, int *histo)
{
    // Histogramme local du bloc en mémoire partagée
    __shared__ int local_histo[26];


    // ------------------------------------------
    // Initialisation de l'histogramme local
    // ------------------------------------------

    if (threadIdx.x < 26) {
        local_histo[threadIdx.x] = 0;
    }

    // On attend que tous les threads aient terminé
    // l'initialisation
    __syncthreads();


    // ------------------------------------------
    // Chaque thread traite un caractère
    // ------------------------------------------

    int i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < num) {

        char c = text[i];

        if (c >= 'a' && c <= 'z') {

            int index = c - 'a';

            // Mise à jour atomique de l'histogramme
            // local du bloc
            atomicAdd(&local_histo[index], 1);
        }
    }


    // ------------------------------------------
    // On attend que tous les threads aient
    // terminé de remplir local_histo
    // ------------------------------------------

    __syncthreads();


    // ------------------------------------------
    // Ajout de l'histogramme local à
    // l'histogramme global
    // ------------------------------------------

    if (threadIdx.x < 26) {

        atomicAdd(&histo[threadIdx.x],
                  local_histo[threadIdx.x]);
    }
}


// ==============================================
// MAIN
// ==============================================
int main(int argc, char *argv[])
{
    int histo[26];


    // ------------------------------------------
    // Vérification des arguments
    // ------------------------------------------

    if (argc != 3) {
        printf("Usage: ./histo <filename> <number of characters>\n");
        return -1;
    }


    // Nombre maximum de caractères à lire
    int num = atoi(argv[2]);


    // ------------------------------------------
    // Lecture du fichier sur le CPU
    // ------------------------------------------

    char *text = read_text(argv[1], &num);

    if (text == NULL)
        return -1;


    printf("Number of characters read: %d\n", num);


    // ==========================================
    // ALLOCATION MEMOIRE GPU
    // ==========================================

    char *d_text;
    int *d_histo;


    cudaMalloc((void **)&d_text,
               num * sizeof(char));

    cudaMalloc((void **)&d_histo,
               26 * sizeof(int));


    // ==========================================
    // COPIE DU TEXTE CPU -> GPU
    // ==========================================

    cudaMemcpy(d_text,
               text,
               num * sizeof(char),
               cudaMemcpyHostToDevice);


    // ==========================================
    // INITIALISATION DE L'HISTOGRAMME GLOBAL
    // ==========================================

    cudaMemset(d_histo,
               0,
               26 * sizeof(int));


    // ==========================================
    // PARAMETRES DU KERNEL
    // ==========================================

    int blockSize = 256;

    int gridSize = (num + blockSize - 1) / blockSize;


    printf("Block size : %d\n", blockSize);
    printf("Grid size  : %d\n", gridSize);


    // ==========================================
    // LANCEMENT DU KERNEL
    // ==========================================

    compute_histogram<<<gridSize, blockSize>>>(
        d_text,
        num,
        d_histo
    );


    // Attendre la fin du kernel
    cudaDeviceSynchronize();


    // ==========================================
    // COPIE DE L'HISTOGRAMME GPU -> CPU
    // ==========================================

    cudaMemcpy(histo,
               d_histo,
               26 * sizeof(int),
               cudaMemcpyDeviceToHost);


    // ==========================================
    // AFFICHAGE DU RESULTAT
    // ==========================================

    int sum = 0;

    for (int i = 0; i < 26; i++) {

        printf("%c : %d\n",
               'a' + i,
               histo[i]);

        sum += histo[i];
    }


    // ==========================================
    // LIBERATION DE LA MEMOIRE
    // ==========================================

    cudaFree(d_text);
    cudaFree(d_histo);

    free(text);


    return 0;
}