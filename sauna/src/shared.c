#include <stdio.h>
#include <stdlib.h> //mkfifo, exit
#include <unistd.h> //unlink
#include <sys/stat.h>  //lstatq

int createFIFO (const char* file){
  struct stat buf;

  if (lstat (file, &buf) == 0){
    if S_ISFIFO (buf.st_mode){
      return 0;
    }

    if (unlink(file) == -1){
      fprintf (stderr, "Error removing File\n");
      exit (1);
    }

    if (mkfifo(file, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1){
      fprintf (stderr, "Error creating FIFO\n");
      exit (1);
    }

  }
  else{
    if (mkfifo(file, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1){
      fprintf (stderr, "Error creating FIFO\n");
      exit (1);
    }
  }

  return 0;
}
