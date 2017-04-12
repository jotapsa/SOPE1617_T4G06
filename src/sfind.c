#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h> //PATH_MAX
#include <sys/types.h>
#include <unistd.h>
#include "finder.h"

static int parentPid;

void sigint_handler(int signo) // need to find a way to identify child proc
{
  pid_t selfPid = getpid ();

  if (selfPid == parentPid){
    char resp;

    printf("Are you sure you want to terminate (Y/N)?\n");
    resp = getchar();

    if(resp == 'y' || resp == 'Y'){
      kill (0, SIGTERM);
    }
    kill (0, SIGCONT);
  }
  else{
    raise (SIGSTOP);
  }

  return;
}

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

int test_arg(char *argv[]){
  if (!(strcmp(argv[2], "-name") == 0 || strcmp(argv[2], "-type") == 0 || strcmp(argv[2], "-perm") == 0)){
    return 1;
  }
  if (!(strcmp(argv[4], "-print") == 0 || strcmp(argv[4], "-delete") == 0 || strcmp(argv[4], "-exec") == 0)){
    return 1;
  }
  return 0;
}

int main (int argc, char *argv[], char *envp[])
{
  char dir[PATH_MAX];
  struct sigaction signal_handler;

  parentPid = getpid();

  signal_handler.sa_handler = sigint_handler;
  sigemptyset(&signal_handler.sa_mask);
  signal_handler.sa_flags = 0;

  if (sigaction(SIGINT, &signal_handler, NULL) < 0)
  {
    perror("Unable to install SIGINT handler");
    exit (1);
  }

  switch(argc) //==2 (help invoked) || == 5 (delete or print) || == 8 (exec)
  {
    case 1:  //in case user does not insert a single option
    {
      printf("Type 'sfind -help' for instructions\n");
    }
    break;
    case 2:
    {
      if(strcmp(argv[1], "-help") == 0) //in case user asks for help
      print_help_menu();
      else
      printf("Type 'sfind -help' for instructions\n");
    }
    break;
    case 5:
    case 6:
    {
      if(test_arg(argv))
      {
        printf ("Invalid arguments\n");
        printf ("Type 'sfind -help' for instructions\n");
        return 1;
      }
      else
      {
        if(searcher(argv[1], argv) == 1)
        {
          printf("Something went WRONG\n");
          return 1;
        }
        else
        printf("Success\n");
      }
    }
    break;
    default:
    {
      printf ("Type 'sfind -help' for instructions\n");
    }
    break;
  }

  return 0;
}
