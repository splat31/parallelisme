#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "cuda.h"


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
// Version GPU : histogramme dans la mémoire globale
// ==============================================
__global__
void compute_histogram_global(char *text, int num, int *histo)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < num) {

        char c = text[i];

        if (c >= 'a' && c <= 'z') {
            int index = c - 'a';

            atomicAdd(&histo[index], 1);
        }
    }
}


// ==============================================
int main(int argc, char *argv[])
{
    int histo[26];

    if (argc != 3) {
        printf("Usage: ./histo <filename> <number of characters>\n");
        return -1;
    }

    int num = atoi(argv[2]);

    // Lecture du fichier sur le CPU
    char *text = read_text(argv[1], &num);

    if (text == NULL)
        return -1;


    // ==========================================
    // Allocation mémoire GPU
    // ==========================================

    char *d_text;
    int *d_histo;

    cudaMalloc((void **)&d_text, num * sizeof(char));
    cudaMalloc((void **)&d_histo, 26 * sizeof(int));


    // Copie du texte CPU -> GPU
    cudaMemcpy(d_text, text,
               num * sizeof(char),
               cudaMemcpyHostToDevice);


    // Initialisation de l'histogramme GPU à 0
    cudaMemset(d_histo, 0, 26 * sizeof(int));


    // ==========================================
    // Lancement du kernel
    // ==========================================

    int blockSize = 256;
    int gridSize = (num + blockSize - 1) / blockSize;

    compute_histogram_global<<<gridSize, blockSize>>>(
        d_text,
        num,
        d_histo
    );

    cudaDeviceSynchronize();


    // ==========================================
    // Copie histogramme GPU -> CPU
    // ==========================================

    cudaMemcpy(histo, d_histo,
               26 * sizeof(int),
               cudaMemcpyDeviceToHost);


    // ==========================================
    // Affichage
    // ==========================================

    int sum = 0;

    for (int i = 0; i < 26; i++) {
        printf("%c : %d\n", 'a' + i, histo[i]);
        sum += histo[i];
    }


    // ==========================================
    // Libération mémoire
    // ==========================================

    cudaFree(d_text);
    cudaFree(d_histo);
    free(text);

    return 0;
}