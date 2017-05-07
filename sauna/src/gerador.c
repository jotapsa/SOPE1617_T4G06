#include <stdio.h>
#include <time.h>
#include <stdlib.h> //rand, exit...
#include <unistd.h> //pid_t
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

void genRequests (void *arg){
  char *req = malloc (sizeof(char)*100);
  char G;
  unsigned long utilTime;

  for (unsigned long i=1; i<nrReq; i++){
    G = genGender();
    utilTime = (rand()%maxTime)+1; //not a uniform distribution

    sprintf (req, "p%lu %c t%lu", i, G, utilTime);
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

  return 0;
}
