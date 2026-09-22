
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

void colorToGrey(color_image_type *col_img, grey_image_type *grey_img){
    for (int i=0; i < col_img->height ; i++)
      for (int j=0; j < col_img->width ; j++){
        int index = i * col_img->width + j;
        color_pixel_type *pix = &(col_img->pixels[index]);
        grey_img->pixels[index] = (299*pix->r + 587*pix->g + 114*pix->b)/1000;
      }
}

/**********************************************************************/

double contrasting (grey_image_type *grey_img,grey_image_type *cons_img) {
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
    C[i] = C[i-1]+H[i]; // en parallele il seras plus judicieux de faire: C[i] = H[0] + H[1] + ... + H[i-1] + H[i]
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

int main(int argc, char ** argv){
  color_image_type * col_img;
  grey_image_type * grey_img;
  grey_image_type * cons_img;
  if (argc != 4){
    printf("Usage: togrey <input image> <output image> <constrated output image>\n");
    exit(-1);
  }
  char *input_file = argv[1];
  char *output_file = argv[2];
  char *output_file2 = argv[3];



  col_img = loadColorImage(input_file);
  grey_img = createGreyImage(col_img->width, col_img->height);

  colorToGrey(col_img, grey_img);
  
  saveGreyImage(output_file, grey_img);
  
  cons_img = createGreyImage(col_img->width, col_img->height);
  double time = contrasting(grey_img,cons_img);
  printf("Temps d'exécution = %f\n" , time);

  saveGreyImage(output_file2, cons_img);

  free(col_img->pixels);
  free(grey_img->pixels);
  free(cons_img->pixels);

  free(col_img);
  free(grey_img);
  free(cons_img);
  
  return 0;
}

