#define MAX_NUM_OBJ 100

int num_obj = 0;
int capacity;
int weight[MAX_NUM_OBJ];
int utility[MAX_NUM_OBJ];

void read_problem(char *filename){
  char line[256];

  FILE *problem = fopen(filename,"r");
  if (problem == NULL){
    fprintf(stderr,"File %s not found.\n",filename);
    exit(-1);
  }

  while (fgets(line, 256, problem) != NULL){
    switch(line[0]){
    case 'c': // capacity
      if (sscanf(&(line[2]),"%d\n", &capacity) != 1){
	fprintf(stderr,"Error in file format in line:\n");
	fprintf(stderr, "%s", line);
	exit(-1);
      }
      break;

    case 'o': // graph size
      if (num_obj >= MAX_NUM_OBJ){
	fprintf(stderr,"Too many objects (%d): limit is %d\n", num_obj, MAX_NUM_OBJ);
	exit(-1);
      }
      if (sscanf(&(line[2]),"%d %d\n", &(weight[num_obj]), &(utility[num_obj])) != 2){
	fprintf(stderr,"Error in file format in line:\n");
	fprintf(stderr, "%s", line);
	exit(-1);
      }
      else
	num_obj++;
      break;

    default:
      break;
    }
  }
  if (num_obj == 0){
    fprintf(stderr,"Could not find any object in the problem file. Exiting.");
    exit(-1);
  }

}
