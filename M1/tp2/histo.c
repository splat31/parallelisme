
#include "stdio.h"
#include "stdlib.h"
#include "assert.h"


// ==============================================
char * read_text(char *filename, int *num){

  char *text = (char *) malloc((*num+1)*sizeof(char));
  if (text == NULL) return NULL;
  FILE *f = fopen(filename,"r");
  if (f==NULL){
    printf("File %s could not be opened\n", filename);
    return NULL;
  }
  int i = 0;
  char c;
  while( (i < *num) && (c=fgetc(f))!=EOF){
      text[i] = c;
      i++;
  }
  *num = i;
  text[i] = 0;
  fclose(f);
  return text;
}

void compute_histogram(char *text, int num, int *histo){
  for (int i=0; i<26; i++)
    histo[i] = 0;

  for (int i=0; i<num ; i++){
    char c = text[i];
    if ( (c >= 'a') && (c <= 'z') )
      histo[c-'a']++;
  }
}

// ==============================================
int main(int argc, char *argv[]) {
    int histo[26];

    if (argc != 3){
      printf("Usage: ./histo <filename> <number of characters>");
      return -1;
    }
    int num = atoi(argv[2]);

    char *text = read_text(argv[1], &num);
    if (text == NULL)
        return -1;

    compute_histogram(text,num,histo);

    int sum = 0;
    for (int i=0;i<26;i++){
      printf("%c : %d\n", 'a'+i, histo[i]);
      sum += histo[i];
    }
    return 0;
}
