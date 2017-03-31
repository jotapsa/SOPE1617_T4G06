#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_STR_LEN 256
#define OPTION 1
#define ACTION 2

void print_help_menu ();

int test_arg(char *arg, int op);

char* extract_dir(char *str);

int main (int argc, char *argv[], char *envp[])
{
   char dir[PATH_MAX + 1];
   
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
   			printf("%s is an invalid dir\n", argv[1]);
   		else
   			printf("diretorio:\t%s\n", dir);
   		
   		return 0;
   	}
   	
   	case 8: //in case user wants -exec
   	{
   		printf("Caso exec\n");
   		
   		return 0;
   	}
   	
   	default:
   	{
   		write(STDOUT_FILENO, "Type 'sfind -help' for instructions\n", 36);
   		
   		return 0;
   	}
   }
}

void print_help_menu () //Prints the instructions for usage
{
  char output[MAX_STR_LEN];

  strcpy(output, "\nUsage: sfind DIR -<options>\n\n");
  write(STDOUT_FILENO, output, strlen(output));
  strcpy(output, "Options:\n\t-name string -> search for a file with the name in string\n\n");
  write(STDOUT_FILENO, output, strlen(output));
  strcpy(output, "\t-type c\n\t\t-> case c = f - search for a normal file\n");
  write(STDOUT_FILENO, output, strlen(output));
  strcpy(output, "\t\t-> case c = d - search for a directory\n\t\t-> case c = l - search for a link\n\n");
  write(STDOUT_FILENO, output, strlen(output));
  strcpy(output, "\t-perm mode -> search for a file that has the permissions\n\t\t\tequivalent to the octal number mode\n\n");
  write(STDOUT_FILENO, output, strlen(output));
  strcpy(output, "Actions:\n");
  write(STDOUT_FILENO, output, strlen(output));
  strcpy(output, "\t-print -> shows the found files on the screen\n\n");
  write(STDOUT_FILENO, output, strlen(output));
  strcpy(output, "\t-delete -> delete the found files\n\n");
  write(STDOUT_FILENO, output, strlen(output));
  strcpy(output, "\t-exec command -> execute command\n\n");
  write(STDOUT_FILENO, output, strlen(output));
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
		return getcwd( buff, PATH_MAX + 1 );
	}
	else if(strcmp(str, "..") == 0) // .. means that we have to search in the previous directory
	{
		if(chdir("..") == -1)
		{
			printf("cant change to parent directory\n");
			return NULL;
		}
		else
			return getcwd( buff, PATH_MAX + 1 );
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


