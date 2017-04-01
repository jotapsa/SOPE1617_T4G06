#include "finder.h"

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


int seacher (char *dir, char *option, char *filename, char *action)
{
  if(strcmp(option, "-name") == 0)
  {
    if(search_for_name(dir, filename, PRINT) == 0)
    {
      printf("sucesso\n");
      return 0;
    }
  }
  else if(strcmp(option, "-type") == 0)
  {

  }
  else
  {

  }

  return 1;
}

int search_for_name (char *dir, char *filename, int op)
{
  DIR *directory;
  struct dirent *dir_info;
  struct stat dir_stat;
  pid_t pid, pidSon;
  int status;

  if((directory = opendir(dir)) == NULL)
  {
    printf("Could not open %s\n", dir);
    return -1;
  }

  switch (op)
  {
    case PRINT:
    {
      chdir(dir);

      while((dir_info = readdir(directory)) != NULL)
      {
        stat(dir_info->d_name, &dir_stat);

        if(S_ISDIR(dir_stat.st_mode))
        {
          pid = fork();

          if(pid == 0)
          {
            strcat(dir, "/");
            strcat(dir, dir_info->d_name);
            search_for_name(dir, filename, PRINT);
            printf("\n");
            dir = previous_dir(dir, strlen(dir_info->d_name));
          }
          else if(pid > 0)
          {
            pidSon = wait(&status);
          }
          else
            return -1;
        }
        else if(S_ISREG(dir_stat.st_mode))
        {
          strcat(dir, "/");
          strcat(dir, dir_info->d_name);
          write(STDOUT_FILENO,dir,strlen(dir) + 1);
          printf("\n");
          dir = previous_dir(dir, strlen(dir_info->d_name));
        }
      }

      break;
    }
    case DELETE:
    {

      break;
    }
  }
  return 0;
}


char* previous_dir(char *curr, int size)
{
  char previous [strlen(curr) - size + 1];
  int i = 0;

  for(i = 0; i < strlen(curr) - size - 2; i++)
  {
    previous[i] = curr[i];
  }
  strcpy(curr, previous);

  return curr;
}
