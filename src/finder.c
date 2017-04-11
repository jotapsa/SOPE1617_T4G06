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

/* Returns the the pathname to start seaching*/
char* extract_dir(char *str){
  char buff[PATH_MAX];
  char* dir;
  struct stat dir_inf;

  if(lstat(str, &dir_inf) == -1){
    perror (str);
    exit (1);
  }
  else
    chdir(str);

  printf ("%s\n",getcwd(buff, PATH_MAX));
  return getcwd(buff, PATH_MAX);
}


int get_type(char *type)
{
  if(strcmp("f", type) == 0)
  return FILE;

  else if (strcmp("d", type) == 0)
  return FOLDER;

  else if (strcmp("l", type) == 0)
  return LINK;

  else
  return -1;
}

char* get_important_digits(char *digits)
{
  char *important = malloc(5*sizeof(char)); //4 digits plus '\0'

  important[3] = digits[strlen(digits) - 1];

  important[2] = digits[strlen(digits) - 2];

  important[1] = digits[strlen(digits) - 3];

  important[0] = digits[strlen(digits) - 4];

  return important;
}

int compare_file_perm(char *perm, mode_t file)
{
  char *file_perm = malloc(10*sizeof(char)); //to make sure it fits xD

  sprintf(file_perm, "%o", file);

  file_perm = get_important_digits(file_perm);

  return strcmp(perm, file_perm);
}

char* get_new_path (char *str1, char *str2)
{
  char *path = malloc((strlen(str1) + strlen(str2) + 2) * sizeof(char));//plus 2 because of '\0' and '/'

  sprintf(path,"%s/%s", str1, str2); //valid path creation

  return path;
}

int file_destroyer (char *filename, int type)
{
  switch(type)
  {
    case FOLDER:
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

/*argv[2] references type, argv[3] references filename, argv[4] references action*/
int searcher_aux (char *path, char *argv[], struct dirent *fileInfo_dirent){

  if ((strcmp (argv[2], "-name")==0) && (strcmp(fileInfo_dirent->d_name, argv[3])==0)){
    if (strcmp (argv[4], "-print")==0){
      printf("%s\n", path);
    }
    else if (strcmp(argv[4], "-delete")==0){
      sleep(1);
    }
  }
  else if (strcmp (argv[2], "-type")==0){
    sleep(1);
  }
  else if (strcmp (argv[2], "-perm")==0){
    sleep(1);
  }
  return 0;
}

int searcher (char *dir, char *argv[]){
  DIR *directory;
  struct dirent *fileInfo_dirent;
  struct stat fileInfo_stat;
  pid_t pid;
  int status;
  char *path, output[PATH_MAX];

  if((directory = opendir(argv[1])) == NULL)
  {
    perror (argv[1]);
    exit (1);
  }

  chdir(argv[1]);

  while((fileInfo_dirent = readdir(directory)) != NULL) //it will go through all the things in the directory
  {
    if(strcmp(fileInfo_dirent->d_name, ".") != 0 && strcmp(fileInfo_dirent->d_name, "..") != 0)
    {
      path = get_new_path(dir, fileInfo_dirent->d_name);

      if (lstat(path, &fileInfo_stat) == -1)
      {
        perror (path);
        exit (1);
      }
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
        searcher (path, argv);
        exit(0);
      }
      else{
        //waitpid(pid, NULL, 0);
        //searcher_aux (path, argv);
      }

    }
    //regular file
    else if(S_ISREG(fileInfo_stat.st_mode) || S_ISLNK(fileInfo_stat.st_mode)){
      searcher_aux (path, argv, fileInfo_dirent);
    }

  }
  return 0;
}
/*
int search_for_name (char *dir, char *filename, int op)
{
  DIR *directory;
  struct dirent *fileInfo_dirent;
  struct stat fileInfo_stat;
  pid_t pid;
  int status;
  char output[PATH_MAX];
  char *path;

  if((directory = opendir(dir)) == NULL)
  {
    perror (dir);
    exit (1);
  }

  chdir(dir);

  while((fileInfo_dirent = readdir(directory)) != NULL) //it will go through all the things in the directory
  {
    if(strcmp(fileInfo_dirent->d_name, ".") != 0 && strcmp(fileInfo_dirent->d_name, "..") != 0) //We don t want to analyse those
    {
      path = get_new_path(dir, fileInfo_dirent->d_name);

      if (lstat(path, &fileInfo_stat) == -1)
      {
        perror (path);
        exit (1);
      }

      if(S_ISDIR(fileInfo_stat.st_mode) && !S_ISLNK(fileInfo_stat.st_mode)) //found a directory
      {
        pid = fork();

        if(pid == 0) //the new process
        {
          search_for_name(path,filename, op);

          exit(0);
        }

        else if(pid > 0) //the current process has to wait for the new one to finish
        {
          waitpid(pid, NULL, 0);

          if(strcmp(fileInfo_dirent->d_name, filename) == 0 && op == PRINT)
            printf ("%s\n", path);

          else if(strcmp(fileInfo_dirent->d_name, filename) == 0 && op == DELETE)
          {
            if(file_destroyer(fileInfo_dirent->d_name, FOLDER) == 1)
            {
              printf("Failed to delete %s\n", fileInfo_dirent->d_name);
              return 1;
            }
          }
        }

        else
        {
          perror ("pid");
          exit(1);
        }
      }

      else if(S_ISREG(fileInfo_stat.st_mode) && strcmp(fileInfo_dirent->d_name, filename) == 0) //found a regular file && and the name of the file correspond to the filename we are looking for
      {
        if(op == PRINT) //prints the directory
        printf("%s\n", path);
        else //destroys the found file
        {
          if(file_destroyer(fileInfo_dirent->d_name, FILE) == 1)
          {
            printf("Failed to delete %s\n", fileInfo_dirent->d_name);
            return 1;
          }
        }
      }
    }
  }
  return 0;
}

int search_for_type (char *dir, int type, int op)
{
  DIR *directory;
  struct dirent *fileInfo_dirent;
  struct stat fileInfo_stat;
  pid_t pid;
  int status;
  char *path;
  char output[PATH_MAX];

  if((directory = opendir(dir)) == NULL)
  {
    perror (dir);
    exit (1);
  }

  if(type == -1)
    return 1;

  chdir(dir);

  while((fileInfo_dirent = readdir(directory)) != NULL) //it will go through all the things in the directory
  {
    if(strcmp(fileInfo_dirent->d_name, ".") != 0 && strcmp(fileInfo_dirent->d_name, "..") != 0)
    {
      path = get_new_path(dir, fileInfo_dirent->d_name);

      if (lstat(path, &fileInfo_stat) == -1)
      {
        perror (path);
        exit (1);
      }

      switch (type)
      {
        case FOLDER: //case we are searching for folders/dirs
        {
          if(S_ISDIR(fileInfo_stat.st_mode) && !S_ISREG(fileInfo_stat.st_mode) ) //FIRST, create new process and search there
          {
            pid = fork();

            if(pid == 0) //new process
            {
              search_for_type(path, type, op);

              exit(0);
            }

            else if(pid > 0) //SECOND, act on the directory
            {
              waitpid(pid, NULL, 0); //wait for fileInfo_dirent-process to finish

              if(op == PRINT) //prints path to directory
                printf ("%s\n", path);

              else
              {
                if(file_destroyer(fileInfo_dirent->d_name, FOLDER) == 1)
                {
                  printf("Failed to delete %s\n", fileInfo_dirent->d_name);
                  return 1;
                }
              }
            }

            else
            {
              perror ("pid");
              exit(1);
            }
          }
        }
        case FILE:
        {
          if(S_ISDIR(fileInfo_stat.st_mode) && !S_ISLNK(fileInfo_stat.st_mode)) //create new process and search there
          {
            pid = fork();

            if(pid == 0) //new process
            {
              search_for_type(path, type, op);

              exit(0);
            }

            else if(pid > 0) //current process just waits
              waitpid(pid, NULL, 0);

            else
            {
              perror ("pid");
              exit (1);
            }
          }
          else if(S_ISREG(fileInfo_stat.st_mode) && !S_ISLNK(fileInfo_stat.st_mode)) //act on the file
          {
            if(op == PRINT && type == FILE) //prints path to file THIS FIXED THE FIRST *bug*, NOT SURE WHY XD
              printf("%s\n",path);

            else if(op == DELETE && type == FILE) //THIS FIXED THE FIRST *bug*, NOT SURE WHY XD
            {
              if(file_destroyer(fileInfo_dirent->d_name, FILE) == 1)
              {
                printf("Failed to delete %s\n", fileInfo_dirent->d_name);
                return 1;
              }
            }
          }
        }
        case LINK:
        {
          if(S_ISDIR(fileInfo_stat.st_mode) && !S_ISLNK(fileInfo_stat.st_mode)) //create new process and search there
          {
            pid = fork();

            if(pid == 0) //new process
            {
              search_for_type(path, type, op);

              exit(0);
            }

            else if(pid > 0) //current process
              waitpid(pid, NULL, 0);

            else
            {
              perror ("pid");
              exit (1);
            }
          }
          else if(S_ISLNK(fileInfo_stat.st_mode) && !S_ISREG(fileInfo_stat.st_mode)) //act on the link
          {
            if(op == PRINT) //prints path to link
              printf ("%s\n",path);

            else
            {
              if(file_destroyer(fileInfo_dirent->d_name, LINK) == 1)
              {
                printf("Failed to delete %s\n", fileInfo_dirent->d_name);
                return 1;
              }
            }
          }
        }
      }
    }
  }
  return 0;
}

int search_for_perm (char *dir, char *perm, int op)
{
  DIR *directory;
  struct dirent *fileInfo_dirent;
  struct stat fileInfo_stat;
  pid_t pid;
  int status;
  char *path;
  char output[PATH_MAX];

  if((directory = opendir(dir)) == NULL)
  {
    perror (dir);
    exit (1);
  }

  chdir(dir);

  while((fileInfo_dirent = readdir(directory)) != NULL) //it will go through all the things in the directory
  {
    if(strcmp(fileInfo_dirent->d_name, ".") != 0 && strcmp(fileInfo_dirent->d_name, "..") != 0) //We don t want to analyse those
    {
      path = get_new_path(dir, fileInfo_dirent->d_name);

      if (lstat(path, &fileInfo_stat) == -1)
      {
        perror (path);
        exit (1);
      }

      if(S_ISDIR(fileInfo_stat.st_mode) && !S_ISLNK(fileInfo_stat.st_mode)) //found a directory
      {
        pid = fork();

        if(pid == 0) //the new process
        {
          search_for_perm(path, perm, op);

          exit(0);
        }
        else if(pid > 0)
        {
          if(compare_file_perm(perm, fileInfo_stat.st_mode) == 0 && op == PRINT)
            printf ("%s\n", path);

          else if(compare_file_perm(perm, fileInfo_stat.st_mode) == 0 && op == DELETE)
          {
            if(file_destroyer(fileInfo_dirent->d_name, FOLDER) == 1)
            {
              printf("Failed to delete %s\n", fileInfo_dirent->d_name);
              return 1;
            }
          }
        }
        else
        {
          perror ("pid");
          exit(1);
        }
      }
      else if((S_ISREG(fileInfo_stat.st_mode) || S_ISLNK(fileInfo_stat.st_mode)) && compare_file_perm(perm, fileInfo_stat.st_mode) == 0) //found a regular file && and the name of the file correspond to the filename we are looking for
      {
        if(op == PRINT)
          printf ("%s\n", path);
        else if(op == DELETE)
        {
          if(file_destroyer(fileInfo_dirent->d_name, FILE) == 1)
          {
            printf("Failed to delete %s\n", fileInfo_dirent->d_name);
            return 1;
          }
        }
      }
    }
  }

  return 0;
}
*/
