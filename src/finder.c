#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h> //PATH_MAX
#include <sys/types.h>
#include <unistd.h> //fork
#include <wait.h> //waitpid
#include <sys/stat.h> // struct stat
#include <dirent.h> //Type DIR
#include "finder.h"


void sigint_handler(int signo)
{
  char resp;

  while(resp != 'y' && resp != 'Y' && resp != 'n' && resp != 'N' )
  {
    printf("Are you sure you want to terminate (Y/N)?\n");
    scanf("%c", &resp);
  }

  if(resp == 'y' || resp == 'Y')
  exit(0);

  return;
}

void print_help_menu () //Prints the instructions for usage
{
  char output[MAX_STR_LEN];

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

int test_arg(char *arg, int op)
{
  if(op == OPTION) //means that we are testing if it is a valid option
  {
    if(strcmp(arg, "-name") != 0 && strcmp(arg, "-type") != 0 && strcmp(arg, "-perm") != 0)
    return 0;
    else
    return 1;
  }
  else //means that we are testing if it is a valid action
  {
    if(strcmp(arg, "-print") != 0 && strcmp(arg, "-delete") != 0 && strcmp(arg, "-exec") != 0)
    return 0;
    else
    return 1;
  }
}

char* extract_dir(char *str) //it will return the directory to start seaching
{
  char buff[PATH_MAX + 1];
  char* dir;
  struct stat dir_inf;

  if(strcmp(str, "~") == 0) //search in the home directory
  {
    strcpy(buff,"/home/");

    strcat(buff,getenv("username"));

    strcpy(dir,buff);

    return dir;
  }
  else if(strcmp(str, ".") == 0) // . means that is to search in the current dir
  {
    return getcwd(buff, PATH_MAX + 1 );
  }
  else if(strcmp(str, "..") == 0) // .. means that we have to search in the previous directory
  {
    if(chdir("..") == -1)
    {
      printf("cant change to parent directory\n");
      return NULL;
    }
    else
    return getcwd(buff, PATH_MAX + 1 );
  }
  else //means that the user wants to search in a diferent non-related directoty
  {
    if(lstat(str, &dir_inf) == -1)
    return "ERROR";
    else
    return str;
  }

  return "ERROR";
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

int seacher (char *dir, char *option, char *filename, char *action)
{
  if(strcmp(option, "-name") == 0)
  {
    if(strcmp(action, "-print") == 0)
    {
      if(search_for_name(dir, filename, PRINT)  == 0)
      return 0;
      else
      return 1;
    }
    else
    {
      if(search_for_name(dir, filename, DELETE)  == 0)
      return 0;
      else
      return 1;
    }
  }

  else if(strcmp(option, "-type") == 0)
  {
    if(strcmp(action, "-print") == 0)
    {
      if(search_for_type(dir, get_type(filename), PRINT)  == 0)
      return 0;
      else
      return 1;
    }
    else
    {
      if(search_for_type(dir, get_type(filename), DELETE)  == 0)
      return 0;
      else
      return 1;
    }
  }
  else
  {

  }

  return 1;
}

int search_for_name (char *dir, char *filename, int op)
{
  DIR *directory;
  struct dirent *sub;
  struct stat dir_stat;
  pid_t pid;
  int status;
  char output[PATH_MAX + 2];

  if((directory = opendir(dir)) == NULL)
  {
    printf("Could not open %s\n", dir);
    return 1;
  }

  chdir(dir);

  while((sub = readdir(directory)) != NULL) //it will go through all the things in the directory
  {
    if(strcmp(sub->d_name, ".") != 0 && strcmp(sub->d_name, "..") != 0) //We don t want to analyse those
    {
      char path[strlen(dir) + strlen(sub->d_name) + 2]; //plus 2 because of '\0' and '/'

      sprintf(path,"%s/%s", dir, sub->d_name); //valid path creation

      if (lstat(path, &dir_stat) == -1)
      {
        printf("lstat ERROR\n");

        return 1;
      }

      if(S_ISDIR(dir_stat.st_mode) && !S_ISLNK(dir_stat.st_mode)) //found a directory
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

          if(strcmp(sub->d_name, filename) == 0 && op == PRINT)
          {
            printf ("%s\n", path);
          }
          else if(strcmp(sub->d_name, filename) == 0 && op == DELETE)
          {
            char cmd[strlen("sudo rm -r ") + strlen(sub->d_name) + 3]; //prepares string with enough size for rm command
            strcpy(cmd, "sudo rm -r '"); //the ' is to make sure files with spaces in the name are deleted too
            strcat(cmd, sub->d_name);
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
                printf ("exec() has failed, and %s was not deleted\n", sub->d_name);
                return 1;
              }
            }
          }
        }

        else
        {
          printf("PID ERROR\n");

          return 1;
        }
      }

      else if(S_ISREG(dir_stat.st_mode) && strcmp(sub->d_name, filename) == 0) //found a regular file && and the name of the file correspond to the filename we are looking for
      {
        if(op == PRINT) //prints the directory
        printf("%s\n", path);
        else //destroys the found file
        {
          char cmd[strlen("rm ") + strlen(sub->d_name) + 3]; //prepares string with enough size for rm command
          strcpy(cmd, "rm '"); //the ' is to make sure files with spaces in the name are deleted too
          strcat(cmd, sub->d_name);
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
              printf ("exec() has failed, and %s was not deleted\n");
              return 1;
            }
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
  struct dirent *sub;
  struct stat dir_stat;
  pid_t pid;
  int status;
  char *path;
  char output[PATH_MAX + 2];

  if((directory = opendir(dir)) == NULL)
  {
    printf("Could not open %s\n", dir);
    return 1;
  }

  if(type == -1)
  return 1;

  chdir(dir);

  while((sub = readdir(directory)) != NULL) //it will go through all the things in the directory
  {
    if(strcmp(sub->d_name, ".") != 0 && strcmp(sub->d_name, "..") != 0)
    {
      char path[strlen(dir) + strlen(sub->d_name) + 2]; //plus 2 because of '\0' and '/'

      sprintf(path,"%s/%s", dir, sub->d_name); //valid path creation

      if (lstat(path, &dir_stat) == -1)
      {
        printf("lstat ERROR\n");

        return 1;
      }

      switch (type)
      {
        case FOLDER: //case we are searching for folders/dirs
        {
          if(S_ISDIR(dir_stat.st_mode) && !S_ISREG(dir_stat.st_mode) ) //FIRST, create new process and search there
          {
            pid = fork();

            if(pid == 0) //new process
            {
              search_for_type(path, type, op);

              exit(0);
            }

            else if(pid > 0) //SECOND, act on the directory
            {
              waitpid(pid, NULL, 0); //wait for sub-process to finish

              if(op == PRINT) //prints path to directory
              {
                sprintf(output,"%s\n",path);
                write(STDOUT_FILENO,output,strlen(output));
              }

              else
              {
                char cmd[strlen("rm -fR ") + strlen(sub->d_name) + 3]; //prepares string with enough size for rm command
                strcpy(cmd, "rm -fR '"); //the ' is to make sure files with spaces in the name are deleted too
                strcat(cmd, sub->d_name);
                strcat(cmd,"'");
                switch (system(cmd)) //give the correct error message depending on system return
                {
                  case -1:
                  {
                    strcpy(output, "fork() failed or waitpid returned an error != EINTR\n");
                    write(STDOUT_FILENO,output,strlen(output));
                    return 1;
                  }
                  case 127:
                  {
                    sprintf(output,"exec() has failed, and %s was not deleted\n", sub->d_name);
                    write(STDOUT_FILENO,output,strlen(output));
                    return 1;
                  }
                }
              }
            }

            else
            {
              printf("PID ERROR\n");

              return 1;
            }
          }
        }
        case FILE:
        {
          if(S_ISDIR(dir_stat.st_mode) && !S_ISLNK(dir_stat.st_mode)) //create new process and search there
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
              printf("PID ERROR\n");

              return 1;
            }
          }
          else if(S_ISREG(dir_stat.st_mode) && !S_ISLNK(dir_stat.st_mode)) //act on the file
          {
            if(op == PRINT && type == FILE) //prints path to file THIS FIXED THE FIRST *bug*, NOT SURE WHY XD
            {
              sprintf(output,"%s\n",path);
              write(STDOUT_FILENO,output,strlen(output));
            }

            else if(op == DELETE && type == FILE) //THIS FIXED THE FIRST *bug*, NOT SURE WHY XD
            {
              char cmd[strlen("rm ") + strlen(sub->d_name) + 3]; //prepares string with enough size for rm command
              strcpy(cmd, "rm '"); //the ' is to make sure files with spaces in the name are deleted too
              strcat(cmd, sub->d_name);
              strcat(cmd,"'");
              switch (system(cmd)) //give the correct error message depending on system return
              {
                case -1:
                {
                  strcpy(output, "fork() failed or waitpid returned an error != EINTR\n");
                  write(STDOUT_FILENO,output,strlen(output));
                  return 1;
                }
                case 127:
                {
                  sprintf(output,"exec() has failed, and %s was not deleted\n", sub->d_name);
                  write(STDOUT_FILENO,output,strlen(output));
                  return 1;
                }
              }
            }
          }
        }
        case LINK:
        {
          if(S_ISDIR(dir_stat.st_mode) && !S_ISLNK(dir_stat.st_mode)) //create new process and search there
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
              printf("PID ERROR\n");

              return 1;
            }
          }
          else if(S_ISLNK(dir_stat.st_mode) && !S_ISREG(dir_stat.st_mode)) //act on the link
          {
            if(op == PRINT) //prints path to link
            {
              printf ("%s\n",path);
            }

            else
            {
              char cmd[strlen("rm ") + strlen(sub->d_name) + 3]; //prepares string with enough size for rm command
              strcpy(cmd, "rm '"); //the ' is to make sure files with spaces in the name are deleted too
              strcat(cmd, sub->d_name);
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
                  printf ("exec() has failed, and %s was not deleted\n", sub->d_name);
                  return 1;
                }
              }
            }
          }
        }
      }
    }
  }
  return 0;
}
