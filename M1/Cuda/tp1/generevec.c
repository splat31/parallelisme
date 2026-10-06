
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv){
  if (argc != 4 && argc != 5){
    printf("Usage: <log(size)> <maxval> <filename> <test>\n"); //1024 = 2**10 test c pour faire en sorte que chaque valeur du tab soit la meme
    exit(-1);
  }
  int log2size = atoi(argv[1]);
  int max = atoi(argv[2]);
  int bool_test = 0;
  if (argc == 5) {
    bool_test = 1;
    printf("option active\n");
  }
  
  FILE *f = fopen(argv[3],"w");
  fprintf(f,"%d\n",log2size);
  int size = 1 << log2size;
  srand(time(NULL));
  if (bool_test == 1) {
    for (int i=0; i<size; i++){
      fprintf(f,"%u\n",max);
    }
  }
  else {
    for (int i=0; i<size; i++){
      unsigned int x = rand() % (max+1);
      fprintf(f,"%u\n",x);
    }
  }
  fclose(f);
}
