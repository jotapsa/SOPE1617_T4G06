#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h> //PATH_MAX
#include <sys/types.h>
#include <unistd.h> //fork
#include <sys/wait.h> //waitpid
#include <sys/stat.h> // struct stat
#include <dirent.h> //Type DIR

#include "finder.h"

void sigint_handler(int signo) // need to find a way to identify child proc
{
  char resp;

  //kill (0, SIGSTOP);

  printf("Are you sure you want to terminate (Y/N)?\n");
  resp = getchar();

  if(resp == 'y' || resp == 'Y'){
    kill (0, SIGTERM);
  }

  //kill (0, SIGCONT);
  return;
}

/*Prints the instructions for usage*/
void print_help_menu ()
{
  printf ("\nUsage: sfind DIR -<options>\n\n");

  printf ("Options:\n\t-name string -> search for a file with the name in string\n\n");
  printf ("\t-type c\n\t\t-> case c = f - search for a normal file\n");
  printf ("\t\t-> case c = d - search for a directory\n\t\t-> case c = l - search for a link\n\n");
  printf ("\t-perm mode -> search for a file that has the permissions\n\t\t\tequivalent to the octal number mode\n\n");

  printf ("Actions:\n");
  printf ("\t-print -> shows the found files on the screen\n\n");
  printf ("\t-delete -> delete the found files\n\n");
  printf ("\t-exec command -> execute command\n\n");
}

/* checks if OPTION and ACTION are valid */
int test_arg(char *argv[]){
  if (!(strcmp(argv[2], "-name") == 0 || strcmp(argv[2], "-type") == 0 || strcmp(argv[2], "-perm") == 0)){
    return 1;
  }
  if (!(strcmp(argv[4], "-print") == 0 || strcmp(argv[4], "-delete") == 0 || strcmp(argv[4], "-exec") == 0)){
    return 1;
  }

  return 0;
}

int getType(char *type)
{
  if(strcmp("f", type) == 0)
  return FILE;

  else if (strcmp("d", type) == 0)
  return DIRECTORY;

  else if (strcmp("l", type) == 0)
  return LINK;

  else
  return -1;
}

int compare_file_perm(char *perm, mode_t st_mode)
{
  char *file_perm = malloc(5*sizeof(char));
  sprintf(file_perm, "%o", st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));

  return strcmp(perm, file_perm);
}

char* getFilePath (char *dirPath, char *fileName)
{
  char *filePath = (char*) malloc((strlen(dirPath) + strlen(fileName) + 2) * sizeof(char));//plus 2 because of '\0' and '/'
  sprintf(filePath,"%s/%s", dirPath, fileName); //valid path creation

  return filePath;
}

int file_destroyer (char *filename, int type)
{
  switch(type)
  {
    case DIRECTORY:
    {
      char cmd[strlen("rm -fR ") + strlen(filename) + 3]; //prepares string with enough size for rm command
      strcpy(cmd, "rm -fR '"); //the ' is to make sure files with spaces in the name are deleted too
      strcat(cmd, filename);
      strcat(cmd,"'");
      switch (system(cmd)) //give the correct error message depending on system return
      {
        case -1:
        {
          printf ("fork() failed or waitpid returned an error != EINTR\n");
          return 1;
        }
        case 127:
        {
          printf ("exec() has failed, and %s was not deleted\n", filename);
          return 1;
        }
      }
    }
    case FILE:
    case LINK:
    {
      char cmd[strlen("rm ") + strlen(filename) + 3]; //prepares string with enough size for rm command
      strcpy(cmd, "rm '"); //the ' is to make sure files with spaces in the name are deleted too
      strcat(cmd, filename);
      strcat(cmd,"'");

      switch (system(cmd)) //give the correct error message depending on system return
      {
        case -1:
        {
          printf ("fork() failed or waitpid returned an error != EINTR\n");
          return 1;
        }
        case 127:
        {
          printf ("exec() has failed, and %s was not deleted\n", filename);
          return 1;
        }
      }
    }
  }

  return 0;
}

/*argv[2] references type, argv[3] references filename type or mode_t , argv[4] references action*/
int searcher_aux (char *filePath, char *argv[], struct stat fileInfo_stat, struct dirent *fileInfo_dirent){

  if ((strcmp (argv[2], "-name")==0) && (strcmp(fileInfo_dirent->d_name, argv[3])==0)){
    if (strcmp (argv[4], "-print")==0){
      printf("%s\n", filePath);
    }
    else if (strcmp(argv[4], "-delete")==0){
      sleep(1);
    }
  }
  else if (strcmp (argv[2], "-type")==0){
    if (strcmp (argv[4], "-print")==0){
      switch (getType(argv[3])){
        case FILE:
        if (S_ISREG(fileInfo_stat.st_mode)){
          printf ("%s\n", filePath);
        }
        break;
        case DIRECTORY:
        if (S_ISDIR(fileInfo_stat.st_mode)){
          printf ("%s\n", filePath);
        }
        break;
        case LINK:
        if (S_ISLNK(fileInfo_stat.st_mode)){
          printf ("%s\n", filePath);
        }
        break;
        default:
        break;
      }
    }
    else if (strcmp(argv[4], "-delete")==0){
      sleep(1);
    }
  }
  else if ((strcmp (argv[2], "-perm")==0) && (compare_file_perm (argv[3], fileInfo_stat.st_mode)==0)){
    if (strcmp (argv[4], "-print")==0){
      printf("%s\n", filePath);
    }
    else if (strcmp(argv[4], "-delete")==0){
      sleep(1);
    }
  }

  return 0;
}

int searcher (char *dirPath, char *argv[]){
  DIR *directory;
  struct dirent *fileInfo_dirent;
  struct stat fileInfo_stat;
  pid_t pid;
  int status;
  char *filePath, output[PATH_MAX];

  if((directory = opendir(dirPath)) == NULL)
  {
    perror (dirPath);
    exit (1);
  }

  //printf ("Searching %s\n", dirPath);
  chdir(dirPath);

  while((fileInfo_dirent = readdir(directory)) != NULL) //it will go through all the things in the directory
  {
    if(strcmp(fileInfo_dirent->d_name, ".") != 0 && strcmp(fileInfo_dirent->d_name, "..") != 0)
    {
      filePath = getFilePath(dirPath, fileInfo_dirent->d_name);

      if (lstat(filePath, &fileInfo_stat) == -1)
      {
        perror (filePath);
        exit (1);
      }

      //directory
      if(S_ISDIR(fileInfo_stat.st_mode) && !S_ISLNK(fileInfo_stat.st_mode))
      {
        pid = fork();

        if (pid == -1){
          perror ("fork failed");
          exit(1);
        }
        if (pid == 0) {
          searcher (filePath, argv);
          exit(0);
        }
        else{
          //waitpid(pid, NULL, 0); // for now
          searcher_aux (filePath, argv, fileInfo_stat, fileInfo_dirent);
        }

      }
      //regular file
      else if(S_ISREG(fileInfo_stat.st_mode) || S_ISLNK(fileInfo_stat.st_mode)){
        searcher_aux (filePath, argv, fileInfo_stat, fileInfo_dirent);
      }
    }
  }

  while (wait(&status) >0); // Wait for every child process to terminate.

  return 0;
}
