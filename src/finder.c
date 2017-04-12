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

search_type getSearchOption (char *option){
  if (strcmp("-name", option) == 0)
  return NAME;

  if (strcmp("-type", option) == 0)
  return TYPE;

  if (strcmp("-perm", option) == 0)
  return PERMISSION;

  return -1;
}

action_type getActionType (char *action){
  if (strcmp("-print", action) == 0)
  return PRINT;

  if (strcmp("-delete", action) == 0)
  return DELETE;

  if (strcmp("-exec", action) == 0)
  return EXECUTE;

  return -1;
}

file_type getFileType (char *type)
{
  if(strcmp("f", type) == 0)
  return REGULAR;

  else if (strcmp("d", type) == 0)
  return DIRECTORY;

  else if (strcmp("l", type) == 0)
  return LINK;

  return -1;
}


int compare_file_perm(char *perm, mode_t st_mode)
{
  char *file_perm = malloc(5*sizeof(char));
  sprintf(file_perm, "%#o", st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)); //%#o (Octal) -> 0 prefix inserted.

  return strcmp(perm, file_perm);
}

char* getFilePath (char *dirPath, char *fileName)
{
  char *filePath = (char*) malloc((strlen(dirPath) + strlen(fileName) + 2) * sizeof(char)); //plus 2 because of '\0' and '/'
  sprintf(filePath,"%s/%s", dirPath, fileName);

  return filePath;
}

int deleteFile (char *filename, struct stat fileInfo_stat){
  char *cmd;

  if (S_ISDIR(fileInfo_stat.st_mode)){
    cmd = (char*) malloc ((strlen("rm -fR ") + strlen(filename) + 3)*sizeof(char)); //allocates space for cmdstring
    sprintf (cmd, "rm -fR '%s'", filename); //the ' is to make sure files with spaces in the name are deleted too
  }
  else if ( S_ISREG(fileInfo_stat.st_mode) || S_ISLNK(fileInfo_stat.st_mode)){
    cmd = (char*) malloc ((strlen("rm ") + strlen(filename) + 3)*sizeof(char));
    sprintf (cmd, "rm -fR '%s'", filename);
  }

  switch (system(cmd))
  {
    case -1:{
      printf ("fork() failed or waitpid returned an error != EINTR\n");
      return 1;
    }
    break;

    case 127:{
      printf ("exec() has failed, and %s was not deleted\n", filename);
      return 1;
    }
    break;

    /*
    default:
      exit(1);
    break;
    */
  }

  return 0;
}

int execOnFile (char *filename, char *argv[]){
  /*
  Write here
  */
  sleep (1);
  return 0;
}

/*argv[2] references type, argv[3] references filename type or mode_t , argv[4] references action*/
int searcher_aux (char *filePath, char *argv[], struct stat fileInfo_stat, struct dirent *fileInfo_dirent){
  switch (getSearchOption(argv[2])){
    case NAME:
    {
      if (strcmp(fileInfo_dirent->d_name, argv[3])==0){
        switch (getActionType(argv[4])) {
          case PRINT:
            printf("%s\n", filePath);
          break;
          case DELETE:
            deleteFile (filePath, fileInfo_stat);
          break;
          case EXECUTE:
            execOnFile (filePath, argv);
          break;
          default:
          break;
        }
      }
    }
    break;
    case TYPE:
      if ((getFileType(argv[3]) == REGULAR && S_ISREG(fileInfo_stat.st_mode))
      || (getFileType(argv[3]) == DIRECTORY && S_ISDIR(fileInfo_stat.st_mode))
      || (getFileType(argv[3]) == LINK && S_ISLNK(fileInfo_stat.st_mode))){
        switch (getActionType(argv[4])) {
          case PRINT:
            printf("%s\n", filePath);
          break;
          case DELETE:
            deleteFile (filePath, fileInfo_stat);
          break;
          case EXECUTE:
            execOnFile (filePath, argv);
          break;
          default:
          break;
        }
      }
    break;
    case PERMISSION:
    {
      if (compare_file_perm (argv[3], fileInfo_stat.st_mode)==0){
        switch (getActionType(argv[4])) {
          case PRINT:
            printf("%s\n", filePath);
          break;
          case DELETE:
            deleteFile (filePath, fileInfo_stat);
          break;
          case EXECUTE:
            execOnFile (filePath, argv);
          break;
          default:
          break;
        }
      }
    }
    break;
    default:
    break;
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
