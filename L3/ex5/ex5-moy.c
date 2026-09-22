
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <omp.h>
#include <stdbool.h>

typedef struct color_pixel_struct {
    unsigned char r,g,b;
} color_pixel_type;

typedef struct color_image_struct
{
  int width, height;
  color_pixel_type * pixels;
} color_image_type;

typedef struct grey_image_struct
{
  int width, height;
  unsigned char * pixels;
} grey_image_type;

/**********************************************************************/

bool Verify(char *f1, char *f2) {
    FILE *file1 = fopen(f1, "rb");
    FILE *file2 = fopen(f2, "rb");
    if (!file1 || !file2) {
      if (file1) fclose(file1);
      if (file2) fclose(file2);
      fprintf(stderr,"Erreur ouverture fichier\n");
      return false;
    }
    int c1, c2;

    do {
      c1 = fgetc(file1);
      c2 = fgetc(file2);

      if (c1 != c2) {
          fclose(file1);
          fclose(file2);
          return false;
      }
    } while (c1 != EOF && c2 != EOF);
    fclose(file1);
    fclose(file2);
    return true;
}

/**********************************************************************/

color_image_type * loadColorImage(char *filename){
  int i, width,height,max_value;
  char format[8];
  color_image_type * image;
  FILE * f = fopen(filename,"r");
  if (!f){
    fprintf(stderr,"Cannot open file %s...\n",filename);
    exit(-1);
  }
  fscanf(f,"%s\n",format);
  assert( (format[0]=='P' && format[1]=='3'));  // check P3 format
  while(fgetc(f)=='#') // commentaire
    {
      while(fgetc(f) != '\n'); // aller jusqu'a la fin de la ligne
    }
  fseek( f, -1, SEEK_CUR);
  fscanf(f,"%d %d\n", &width, &height); 
  fscanf(f,"%d\n", &max_value);
  image = malloc(sizeof(color_image_type));
  assert(image != NULL);
  image->width = width;
  image->height = height;
  image->pixels = malloc(width*height*sizeof(color_pixel_type));
  assert(image->pixels != NULL);

  for(i=0 ; i<width*height ; i++){
      int r,g,b;
      fscanf(f,"%d %d %d", &r, &g, &b);
      image->pixels[i].r = (unsigned char) r;
      image->pixels[i].g = (unsigned char) g;
      image->pixels[i].b = (unsigned char) b;
    }
  fclose(f);
  return image;
}

/**********************************************************************/

grey_image_type * createGreyImage(int width, int height){
  grey_image_type * image = malloc(sizeof(grey_image_type));
  assert(image != NULL);
  image->width = width;
  image->height = height;
  image->pixels = malloc(width*height*sizeof(unsigned char));
  assert(image->pixels != NULL);
  return(image);
}

/**********************************************************************/

void saveGreyImage(char * filename, grey_image_type *image){
  int i;
  FILE * f = fopen(filename,"w");
  if (!f){
    fprintf(stderr,"Cannot open file %s...\n",filename);
    exit(-1);
  }
  fprintf(f,"P2\n%d %d\n255\n",image->width,image->height);
  for(i=0 ; i<image->width*image->height ; i++){
    fprintf(f,"%d\n",image->pixels[i]);
  }
  fclose(f);
}

/**********************************************************************/

void saveColorImage(char * filename, color_image_type *image){
  int i;
  FILE * f = fopen(filename,"w");
  if (!f){
    fprintf(stderr,"Cannot open file %s...\n",filename);
    exit(-1);
  }
  fprintf(f,"P3\n%d %d\n255\n",image->width,image->height);
  for(i=0 ; i<image->width*image->height ; i++){
    fprintf(f,"%d\n%d\n%d\n",image->pixels[i].r, image->pixels[i].g, image->pixels[i].b);
  }
  fclose(f);
}

/**********************************************************************/

double colorToGreySeq(color_image_type *col_img, grey_image_type *grey_img){
  double start,stop,t;
  start = omp_get_wtime();
  for (int i=0; i < col_img->height ; i++) {
    for (int j=0; j < col_img->width ; j++){
      int index = i * col_img->width + j;
      color_pixel_type *pix = &(col_img->pixels[index]);
      grey_img->pixels[index] = (299*pix->r + 587*pix->g + 114*pix->b)/1000;
    }
  }
  stop = omp_get_wtime(); 
  t = stop -start;
  return t;
}

/**********************************************************************/

double colorToGreyPar(color_image_type *col_img, grey_image_type *grey_img,int nb){
    double start,stop,t;
    start = omp_get_wtime();
    #pragma omp parallel for num_threads(nb)
    for (int i=0; i < col_img->height ; i++) {
      for (int j=0; j < col_img->width ; j++){
          int index = i * col_img->width + j;
          color_pixel_type *pix = &(col_img->pixels[index]);
          grey_img->pixels[index] = (299*pix->r + 587*pix->g + 114*pix->b)/1000;
      }
    }
    stop = omp_get_wtime(); 
    t = stop -start;
    return t;
  }

/**********************************************************************/

double greyToContrastSeq (grey_image_type *grey_img,grey_image_type *cons_img) {
  int H[256] = {0};
  int C[256] = {0};
  
  double start,stop,t;
  start = omp_get_wtime();
  for (int i=0; i < grey_img->height ; i++) {
      for (int j=0; j < grey_img->width ; j++){
        int index = i * grey_img->width + j;
        H[grey_img->pixels[index]]++;
      }
  }
  C[0] = H[0];
  for (int i = 1; i<256;i++) {
    C[i] = C[i-1]+H[i]; // en parallele on pourras tester de faire: C[i] = H[0] + H[1] + ... + H[i-1] + H[i]
  }

  int S=grey_img->height * grey_img->width;
  for (int i=0; i < grey_img->height ; i++) {
      for (int j=0; j < grey_img->width ; j++){
        int index = i * grey_img->width + j;
        cons_img->pixels[index] = 255*C[grey_img->pixels[index]]/S;
      }
  }
  stop = omp_get_wtime(); 
  t = stop -start;
  return t;
}

/**********************************************************************/
double greyToContrastPar (grey_image_type *grey_img,grey_image_type *cons_img,int nb) {
    int H[256] = {0};
    int C[256] = {0};
    double start,stop,t;
    start = omp_get_wtime();
    #pragma omp parallel num_threads(nb)
    {
      #pragma omp for reduction (+:H[:256]) 
      for (int i=0; i < grey_img->height ; i++) {
          for (int j=0; j < grey_img->width ; j++){
            int index = i * grey_img->width + j;
            unsigned char pixel = grey_img->pixels[index];
  
            H[pixel]++;
          }
      }
      
      C[0] = H[0];

      #pragma omp for schedule(dynamic , 10)
      for (int i = 1; i<256;i++) {
        for (int j = 0;j<=i;j++) {
          C[i]+= H[j];
        }
      }/*
      for (int i = 1; i<256;i++) {
        C[i] = C[i-1]+H[i]; // en parallele on pourras tester de faire: C[i] = H[0] + H[1] + ... + H[i-1] + H[i]
      }*/
      int S=grey_img->height * grey_img->width;
  
      #pragma omp for
      for (int i=0; i < grey_img->height ; i++) {
          for (int j=0; j < grey_img->width ; j++){
            int index = i * grey_img->width + j;
            cons_img->pixels[index] = 255*C[grey_img->pixels[index]]/S;
          }
      }
    }
    
    stop = omp_get_wtime(); 
    t = stop -start;
    return t;
  }
/**********************************************************************/

double seq(color_image_type * col_img,grey_image_type * grey_img,grey_image_type *cons_img,char* output_file,char* output_file2) {
    double time = 0;
    time+=colorToGreySeq(col_img, grey_img);
    saveGreyImage(output_file, grey_img);
    time += greyToContrastSeq(grey_img,cons_img);
    saveGreyImage(output_file2, cons_img);
    return time;
}

/**********************************************************************/

double par(color_image_type * col_img,grey_image_type * grey_img,grey_image_type *cons_img,char* output_file,char* output_file2,int nb) {
  double time = 0;
  time+=colorToGreyPar(col_img, grey_img,nb);
  saveGreyImage(output_file, grey_img);
  time += greyToContrastPar(grey_img,cons_img,nb);
  saveGreyImage(output_file2, cons_img);
  return time;
}

/**********************************************************************/
int main(int argc, char ** argv){
  color_image_type * col_img;
  grey_image_type * grey_img;
  grey_image_type * cons_img;
  if (argc != 4 && argc != 5){
    printf("Usage: togrey <input image> <nbthreads> <nbmoy> (<verify_folder>)\n"); //verify_folder = -w pour windows -l pour linux et -m pour macos tout autre entrée seras considere commene linux
    exit(-1);
  }
  char *input_file = argv[1];
  int nb = atoi(argv[2]);
  
  char output_file[256];
  char output_file2[256];
  char verify_file[256];
  int nbmoy = atoi(argv[3]);
  if (nbmoy <= 0) {
    fprintf(stderr,"Nbmoy est inférieur ou égal à 0\n");
    exit(-1);
  }
  char name[128];
  strcpy(name, argv[1]);

  // enlève l'extension .ppm
  char *dot = strrchr(name, '.');
  if (dot) {
    *dot = '\0';
  }
  sprintf(output_file,"./obtained/moy/%s.grey.pgm",name);
  sprintf(output_file2,"./obtained/moy/%s.contrast.pgm",name);


  char *verify_folder = "resultats";

  if (argc == 5) {
      if (argv[4][0] == '-') {
          switch (argv[4][1]) {
              case 'w':
                  verify_folder = "windows/";
                  break;
              case 'm':
                  verify_folder = "__MACOSX/._";
                  break;
              case 'l': 
                  verify_folder = "resultats/";
                  break;

              default:
                  fprintf(stderr,"Option inconnue ( %s ) (utilisation de resultats). Rappel:-w pour windows , -l pour linux, -m pour mac\n",argv[4]);
          }
      }
      else {
        fprintf(stderr,"Option inconnue ( %s ) (utilisation de resultats). Rappel:-w pour windows , -l pour linux, -m pour mac\n",argv[4]);
      }
  }
  printf("Début des test avec le dossier de vérification: %s\n",verify_folder);
  sprintf(verify_file,"./resultats/%s%s.contrast.pgm",verify_folder,name);


  col_img = loadColorImage(input_file);
  grey_img = createGreyImage(col_img->width, col_img->height);
  cons_img = createGreyImage(col_img->width, col_img->height);

  double tseq = 0;
  for (int i = 0;i<nbmoy ;i++) {
    tseq += seq(col_img,grey_img,cons_img,output_file,output_file2);
    if (!Verify(output_file2,verify_file)) {
      free(col_img->pixels);
      free(grey_img->pixels);
      free(cons_img->pixels);
  
      free(col_img);
      free(grey_img);
      free(cons_img);
      fprintf(stderr,"Seq a fait une erreur. Vérifiez que vous utilisez bien le bon dossier de résultats (-w pour windows , -l pour linux, -m pour mac)\n");
      exit(-1);
    }
  }
  tseq = tseq/nbmoy;
  printf("\nTemps d'exécution pour la version séquentielle\n\t-séquentielle: %f\n\n",tseq);

  for (int i = 2;i<=nb;i++) {
    double tpar = 0;
    for (int j = 0;j<nbmoy ;j++) {
      tpar += par(col_img,grey_img,cons_img,output_file,output_file2,i);
      if (!Verify(output_file2,verify_file)) {
        free(col_img->pixels);
        free(grey_img->pixels);
        free(cons_img->pixels);
    
        free(col_img);
        free(grey_img);
        free(cons_img);
        fprintf(stderr,"Par a fait une erreur. Vérifiez que vous utilisez bien le bon dossier de résultats (-w pour windows , -l pour linux, -m pour mac)\n");
        exit(-1);
      }
    }
    tpar = tpar/nbmoy;
    printf("Temps d'exécution de la version parallèle avec %d threads:\n\t-parallèle: %f\n\n",i,tpar);
  }

  free(col_img->pixels);
  free(grey_img->pixels);
  free(cons_img->pixels);

  free(col_img);
  free(grey_img);
  free(cons_img);
  
  return 0;
}

