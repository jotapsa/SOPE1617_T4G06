#include <stdio.h>
#include <stdlib.h> //mkfifo, exit
#include <unistd.h> //unlink
#include <sys/stat.h>  //lstatq
#include <errno.h> //errno
#include <limits.h> //ULONG_MAX

int createFIFO (const char* file){
  struct stat buf;

  if (lstat (file, &buf) == 0){
    if S_ISFIFO (buf.st_mode){
      return 0;
    }

    if (unlink(file) == -1){
      fprintf (stderr, "Error removing File\n");
      return 1;
    }

    if (mkfifo(file, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1){
      fprintf (stderr, "Error creating FIFO\n");
      return 1;
    }

  }
  else{
    if (mkfifo(file, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1){
      fprintf (stderr, "Error creating FIFO\n");
      return 1;
    }
  }

  return 0;
}

int checkPathREG (const char* file){
  struct stat buf;

  if (lstat (file, &buf) == 0){
    if S_ISREG (buf.st_mode){
      return 0; //exists and is a regularFile
    }

    if (unlink(file) == -1){
      fprintf (stderr, "Error removing File\n");
      return 1;
    }
  }

  return 0;
}

unsigned long parse_ulong(char *str, int base){
  char *endptr;
  unsigned long val;

  errno = 0; //the program should set errno to 0 before calling strtoul

  val = strtoul(str, &endptr, base);
  if (errno == ERANGE && val == ULONG_MAX ){
    fprintf (stderr, "strtoul overflow\n");
    return ULONG_MAX;
  }

  if (errno != 0 && val == 0) {
    fprintf (stderr, "strtoul: base not supported or no digits seen\n");
    return ULONG_MAX;
  }

  if (endptr == str) {
	  fprintf (stderr, "parse_long: no digits were found in %s \n", str);
	  return ULONG_MAX;
  }

  return val;
}
