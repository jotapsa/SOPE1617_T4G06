#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int createRejectsFIFO (const char* file){
  struct stat buf;

  if (lstat (file, &buf) == 0){
    if S_ISFIFO (buf.st_mode){
      return 0;
    }

    if (unlink(file) == -1){
      fprintf (stderr, "Error removing File\n");
      exit (1);
    }

    if (mkfifo (file, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1){
      fprintf (stderr, "Error creating FIFO\n");
      exit (1);
    }

  }
  else{
    if (mkfifo (file, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1){
      fprintf (stderr, "Error creating FIFO\n");
      exit (1);
    }
  }

  return 0;
}


void print_help_menu (){
    printf ("\nUsage: sauna <n. lugares>\n\n");
}

int main  (int argc, char *argv[], char *envp[]){

  pid_t pid;
  int rejectsFileDes;
  char *rejectsFifoPath = "/tmp/rejeitados";

  if (argc != 2){
    print_help_menu ();
    exit (0);
  }
  else{
    //validar argumento
  }

  createRejectsFIFO (rejectsFifoPath);

  //since we dont give the O_NONBLOCK FLAG to open the process must wait until some other process opens the FIFO for reading
  if ((rejectsFileDes = open (rejectsFifoPath, O_WRONLY | O_TRUNC)) == -1){
    fprintf(stderr, "Error opening file\n");
    exit (2);
  }

  pid = getpid();

  if (close (rejectsFileDes) == -1){
    fprintf(stderr, "Error closing file\n");
    exit (3);
  }

  return 0;
}
