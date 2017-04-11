#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h> //PATH_MAX
#include "finder.h"

int main (int argc, char *argv[], char *envp[])
{
  char dir[PATH_MAX];

  struct sigaction signal_handler;

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
      return 0;
    }

    case 2:
    {
      if(strcmp(argv[1], "-help") == 0) //in case user asks for help
      print_help_menu();
      else
      printf("Type 'sfind -help' for instructions\n");
      return 0;
    }
    case 5: //in case user wants -print or -delete
    {
      strncpy(dir, extract_dir(argv[1]), PATH_MAX);

      printf("dir:\t%s\n", dir);

      if(test_arg(argv))
      {
        printf ("Invalid arguments\n");
        printf ("Type 'sfind -help' for instructions\n");
        return 1;
      }
      else
      {
        if(searcher(dir, argv) == 1)
        {
          printf("Something went WRONG\n");
          return 1;
        }
        else
        printf("Success\n");
      }
      return 0;
    }

    case 8: //in case user wants -exec
    {
      printf("Case exec\n");
      return 0;
    }

    default:
    {
      printf ("Type 'sfind -help' for instructions\n");
      return 0;
    }
  }

  return 0;
}
