#include <stdio.h>
#include <stdlib.h> //exit, etc...
#include <unistd.h> //pid_t
#include <time.h>
#include <fcntl.h> //file handling
#include <limits.h> //ULONG_MAX
#include <string.h> //strtok
#include <pthread.h>

#include "shared.h"

pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER; // mutex para a s.c.
int nrPlaces; //variaveis partilhadas

void print_help_menu (){
    printf ("\nUsage: sauna <n. lugares>\n\n");
}

void updateSlots (int change){
  pthread_mutex_lock (&mut);
  nrPlaces += change;
  pthread_mutex_unlock (&mut);
}

int main  (int argc, char *argv[], char *envp[]){
  info_t info;
  info.t0 = clock ();

  clock_t end;

  info.pid = getpid();
  char *rejectsFIFOPath = "/tmp/rejeitados";
  char *entriesFIFOPath = "/tmp/entrada";
  char *registerPath = malloc(15*sizeof(char)); //pid_t is a signed integer
  snprintf(registerPath, 15, "/tmp/bal.%d", info.pid);

  int bytes;
  request_t req;

  if (argc != 2){
    print_help_menu ();
    exit (0);
  }
  else{
    nrPlaces = parse_ulong (argv[1], 10);
    if (nrPlaces == ULONG_MAX){
      exit(1);
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

  //since we dont give the O_NONBLOCK FLAG to open the process must wait until some other process opens the FIFO for reading
  if ((info.rejectsFileDes = open (rejectsFIFOPath, O_WRONLY | O_TRUNC | O_SYNC)) == -1){
    fprintf(stderr, "Error opening file %s\n", rejectsFIFOPath);
    exit (2);
  }

  if ((info.entriesFileDes = open (entriesFIFOPath, O_RDONLY))==-1){
    fprintf(stderr, "Error opening file %s\n", entriesFIFOPath);
    exit (2);
  }
  getchar ();

  //Read from the ENTRIES FIFO
  while ((bytes = read (info.entriesFileDes, &req, sizeof(request_t)))>0){
    printf ("p%lu %c t%lu\n", req.id, req.gender, req.dur);
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
