#include <stdio.h>
#include <stdlib.h> //exit, etc...
#include <unistd.h> //pid_t
#include <fcntl.h> //file handling
#include <limits.h> //ULONG_MAX

#include "shared.h"

int nrPlaces;

void print_help_menu (){
    printf ("\nUsage: sauna <n. lugares>\n\n");
}

int main  (int argc, char *argv[], char *envp[]){

  pid_t pid = getpid();
  int rejectsFileDes, entriesFileDes;
  char *rejectsFIFOPath = "/tmp/rejeitados";
  char *entriesFIFOPath = "/tmp/entrada";

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

  if (createFIFO (rejectsFIFOPath) != 0 || createFIFO (entriesFIFOPath) != 0){
    exit (1);
  }

  if ((entriesFileDes = open (entriesFIFOPath, O_RDONLY)) == -1){
    fprintf(stderr, "Error opening file %s\n", entriesFIFOPath);
    exit (2);
  }

  //since we dont give the O_NONBLOCK FLAG to open the process must wait until some other process opens the FIFO for reading
  if ((rejectsFileDes = open (rejectsFIFOPath, O_WRONLY | O_TRUNC | O_SYNC)) == -1){
    fprintf(stderr, "Error opening file %s\n", rejectsFIFOPath);
    exit (2);
  }

  if (close (rejectsFileDes) == -1){
    fprintf(stderr, "Error closing file %s\n", rejectsFIFOPath);
    exit (3);
  }

  if (close (entriesFileDes) == -1){
    fprintf(stderr, "Error closing file %s\n", entriesFIFOPath);
    exit (3);
  }

  return 0;
}
