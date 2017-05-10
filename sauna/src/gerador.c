#include <stdio.h>
#include <string.h> //strlen
#include <time.h>
#include <stdlib.h> //rand, exit...
#include <unistd.h> //pid_t
#include <fcntl.h> //file handling
#include <pthread.h>
#include <limits.h> //ULONG_MAX

#include "shared.h"
#include "gerador.h"

#define NUM_THREADS 2

unsigned long nrReq, maxTime;

void print_help_menu (){
    printf ("\nUsage: gerador <nr. pedidos> <max. utilizacao>\n\n");
}

static inline char *tipToString (tip t){
  static char *str_tip[] = {"PEDIDO", "REJEITADO", "DESCARTADO"}; //static so we only initialize once
  return str_tip[t];
}

void genRegMsg (char * reg, request_t *req, info_t *info, tip t){
  clock_t t1 = clock ();
  double long elapsedTime=0;

  elapsedTime = ((t1-info->t0) / CLOCKS_PER_SEC)*1000;
  snprintf (reg, REG_MAXLEN, "%Lf - %d - %lu: %c - %lu - %s\n", elapsedTime, info->pid, req->id, req->gender, req->dur, tipToString(t));
}

void *genRequests (void *arg){
  request_t req;
  char reg[REG_MAXLEN];
  info_t *info = (info_t *)arg;

  for (unsigned long i=1; i<=nrReq; i++){
    req.id = i;
    req.gender = (rand()%2) ? 'M' : 'F';
    req.dur = (rand()%maxTime)+1; //not a uniform distribution
    write (info->entriesFileDes, &req, sizeof(request_t));

    genRegMsg (reg, &req, info, GENERATED);
    write (info->registerFileDes, reg, strlen(reg));
  }
}

void *handleRejects (void *arg){
  clock_t t1;
  request_t req;
  char reg[REG_MAXLEN];
  info_t *info = (info_t *)arg;

  while (read (info->rejectsFileDes, &req, sizeof(request_t))>0){
    if (req.denials >= 3){
      genRegMsg (reg, &req, info, DISCARDED);
      write (info->registerFileDes, reg, strlen(reg));
    }
    else{
      write (info->entriesFileDes, &req, sizeof(request_t));

      genRegMsg (reg, &req, info, REJECTED);
      write (info->registerFileDes, reg, strlen(reg));
    }
  }
}

int main  (int argc, char *argv[], char *envp[]){
  info_t info;
  info.t0 = clock ();

  clock_t end;

  srand(time(NULL)); //initialize the seed from the current time

  info.pid = getpid();
  char *rejectsFIFOPath = "/tmp/rejeitados";
  char *entriesFIFOPath = "/tmp/entrada";
  char *registerPath = malloc(15*sizeof(char)); //pid_t is a signed integer
  snprintf(registerPath, 15, "/tmp/ger.%d", info.pid);

  pthread_t tid[NUM_THREADS];

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

  if (checkPathREG(registerPath) != 0){
    exit(1);
  }

  if ((info.registerFileDes=open(registerPath, O_WRONLY|O_CREAT|O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))==-1){
    fprintf(stderr, "Error opening file %s\n", registerPath);
    exit (2);
  }


  if (createFIFO (rejectsFIFOPath) != 0 || createFIFO (entriesFIFOPath) != 0){
    exit (1);
  }

  if ((info.rejectsFileDes = open (rejectsFIFOPath, O_RDONLY)) == -1){
    fprintf(stderr, "Error opening file %s\n", rejectsFIFOPath);
    exit (2);
  }

  //since we dont give the O_NONBLOCK FLAG to open the process must wait until some other process opens the FIFO for reading
  if ((info.entriesFileDes=open(entriesFIFOPath, O_WRONLY| O_TRUNC | O_SYNC))==-1){
    fprintf(stderr, "Error opening file %s\n", entriesFIFOPath);
    exit (2);
  }

  pthread_create (&tid[0], NULL, genRequests, &info);
  pthread_create (&tid[1], NULL, handleRejects, &info);

  for (int i=0; i<NUM_THREADS; i++){
    pthread_join (tid[i], NULL);
  }

  if (close (info.entriesFileDes) == -1){
    fprintf(stderr, "Error closing file %s\n", entriesFIFOPath);
    exit (3);
  }

  if (close (info.rejectsFileDes) == -1){
    fprintf(stderr, "Error closing file %s\n", rejectsFIFOPath);
    exit (3);
  }

  if (close (info.registerFileDes) == -1){
    fprintf(stderr, "Error closing file %s\n", registerPath);
    exit (3);
  }

  end = clock(); //fim da medicao de tempo

  printf("Clock:  %4.2Lf s\n", (long double)(end-info.t0)/(CLOCKS_PER_SEC));

  return 0;
}
