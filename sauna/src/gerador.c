#include <stdio.h>
#include <string.h> //strlen
#include <time.h>
#include <stdlib.h> //rand, exit...
#include <unistd.h> //pid_t
#include <fcntl.h> //file handling
#include <pthread.h>
#include <limits.h> //ULONG_MAX

#include "shared.h"

unsigned long nrReq, maxTime;

void print_help_menu (){
    printf ("\nUsage: gerador <nr. pedidos> <max. utilizacao>\n\n");
}

char genGender (){
  if (rand()%2==0){
    return 'M';
  }
  else{
    return 'F';
  }
}

/*
  arg - entriesFileDes
*/
void *genRequests (void *arg){
  char *req = malloc (sizeof(char)*27);
  char G;
  unsigned long utilTime;
  int entriesFileDes = *(int *)arg;

  for (unsigned long i=1; i<=nrReq; i++){
    G = genGender();
    utilTime = (rand()%maxTime)+1; //not a uniform distribution
    sprintf (req, "p%lu %c t%lu\n", i, G, utilTime);
    //printf ("%s", req);
    write (entriesFileDes, req, strlen(req));
  }

  free (req);
}

int main  (int argc, char *argv[], char *envp[]){

  srand(time(NULL)); //initialize the seed from the current time
  pid_t pid = getpid();;
  int rejectsFileDes, entriesFileDes;
  char *rejectsFIFOPath = "/tmp/rejeitados";
  char *entriesFIFOPath = "/tmp/entrada";

  pthread_t tid[2];

  if (argc != 3){
    print_help_menu ();
    exit (0);
  }
  else{
    nrReq = parse_ulong (argv[1], 10);
    if (nrReq == ULONG_MAX){
      exit(1);
    }

    maxTime = parse_ulong (argv[2], 10);
    if (maxTime == ULONG_MAX || maxTime > RAND_MAX){
      exit (1);
    }
  }

  if (createFIFO (rejectsFIFOPath) != 0 || createFIFO (entriesFIFOPath) != 0){
    exit (1);
  }

  //since we dont give the O_NONBLOCK FLAG to open the process must wait until some other process opens the FIFO for reading
  if ((entriesFileDes=open(entriesFIFOPath, O_WRONLY| O_TRUNC | O_SYNC))==-1){
    fprintf(stderr, "Error opening file %s\n", entriesFIFOPath);
    exit (2);
  }

  pthread_create (&tid[0], NULL, genRequests, &entriesFileDes);

  pthread_join (tid[0], NULL);

  if (close (entriesFileDes) == -1){
    fprintf(stderr, "Error closing file %s\n", entriesFIFOPath);
    exit (3);
  }

  return 0;
}
