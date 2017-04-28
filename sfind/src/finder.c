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
  int res;
  char *file_perm = malloc(5*sizeof(char));

  sprintf(file_perm, "%#o", st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)); //%#o (Octal) -> 0 prefix inserted.
  res = strcmp(perm, file_perm);

  free(file_perm);
  return res;
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
      free(cmd);
      return 1;
    }
    break;

    case 127:{
      printf ("exec() has failed, and %s was not deleted\n", filename);
      free(cmd);
      return 1;
    }
    break;

    default:
    break;
  }
  free(cmd);
  return 0;
}

int execOnFile (char *filename, char *argv[]){

  if(execlp(argv[5],argv[5],filename,NULL) == -1){
    perror("Invalid command");
    return 1;
  }

  /*
  char *cmd = (char*) malloc ((strlen(argv[5]) + strlen(filename) + 4)*sizeof(char)); //allocates space for cmdstring
  sprintf (cmd, "%s '%s'", argv[5], filename);
  system (cmd);
  free(cmd);
  */
  return 0;
}

/*argv[2] references type, argv[3] references filename type or mode_t , argv[4] references action, argv[5] if received is the command */
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
      || (getFileType(argv[3]) == LINK && S_ISLNK(fileInfo_stat.st_mode))){ //if we are looking for regular/directory/link, and it is a regular file/directory/link
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

  chdir(dirPath);

  while((fileInfo_dirent = readdir(directory)) != NULL) //it will go through all the things in the directory
  {
    if(strcmp(fileInfo_dirent->d_name, ".") != 0 && strcmp(fileInfo_dirent->d_name, "..") != 0)
    {
      filePath = getFilePath(dirPath, fileInfo_dirent->d_name);

      if (lstat(filePath, &fileInfo_stat) == -1)
      {
        perror (filePath);
        free(filePath); //allocated in getFilePath method
        exit (1);
      }

      //directory
      if(S_ISDIR(fileInfo_stat.st_mode) && !S_ISLNK(fileInfo_stat.st_mode))
      {
        pid = fork();

        if (pid == -1){
          perror ("fork failed");
          free(filePath); //allocated in getFilePath method
          exit(1);
        }
        if (pid == 0) {
          searcher (filePath, argv);
          free(filePath); //allocated in getFilePath method
          exit (0); // we dont want the child to return to main
        }
        else{
          //Commenting the line below makes the current process keep on going, while its fine for -print and -delete makes the program unstable when doing -exec
          waitpid(pid, NULL, 0); // Uncommenting this makes the process wait while the son goes through the directory
          searcher_aux (filePath, argv, fileInfo_stat, fileInfo_dirent);
        }

      }
      //regular file
      else if(S_ISREG(fileInfo_stat.st_mode) || S_ISLNK(fileInfo_stat.st_mode)){
        searcher_aux (filePath, argv, fileInfo_stat, fileInfo_dirent);
      }
      free(filePath); //allocated in getFilePath method
    }
  }
  while (wait(&status) >0); // Wait for every child process to terminate.

  return 0;
}
