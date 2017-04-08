#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "finder.h"

int main (int argc, char *argv[], char *envp[])
{
   char dir[PATH_MAX + 1];

   struct sigaction signal_handler;

   signal_handler.sa_handler = sigint_handler;

   sigemptyset(&signal_handler.sa_mask);

   signal_handler.sa_flags = 0;

   if (sigaction(SIGINT, &signal_handler, NULL) < 0)
   {
      fprintf(stderr,"Unable to install SIGINT handler\n"); //should print to erro stream

      exit(1);
    }

   switch(argc) //==2 (help invoked) || == 5 (delete or print) || == 8 (exec)
   {
   	case 1:  //in case user does not insert a single option
   	{
   		write(STDOUT_FILENO, "Type 'sfind -help' for instructions\n", 36);

   		return 0;
   	}

   	case 2:
   	{
   		if(strcmp(argv[1], "-help") == 0) //in case user asks for help
   			print_help_menu();
   		else
   			write(STDOUT_FILENO, "Type 'sfind -help' for instructions\n", 36);

   		return 0;
   	}
   	case 5: //in case user wants -print or -delete
   	{
   		strcpy(dir, extract_dir(argv[1]));

   		if(strcmp(dir, "ERROR") == 0)
      {
        printf("%s is an invalid dir\n", argv[1]);
        return 1;
      }
   		else
   			printf("dir:\t%s\n", dir);

      if(!(test_arg(argv[2], OPTION) && test_arg(argv[4], ACTION)))//checks if OPTION and ACTION are valid
      {
        printf("%s ou %s ARE NOT valid\n", argv[2], argv[4]);

        return 1;
      }
      else
      {
        if(seacher(dir, argv[2], argv[3], argv[4]) == 1)
        {
          printf("Something went WRONG\n");
          
          return 1;
        }
        else
          printf("Sucess\n");
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
   		write(STDOUT_FILENO, "Type 'sfind -help' for instructions\n", 36);

   		return 0;
   	}
   }
}
