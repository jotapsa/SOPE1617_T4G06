#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_STR_LEN 256

int main (int argc, char *argv[], char *envp[])
{
  char output[MAX_STR_LEN];

  if(strcmp("-name", argv[1]) == 0)
  {

  }
  else if(strcmp("-type", argv[1]) == 0)
  {

  }
  else if(strcmp("-perm", argv[1]) == 0)
  {

  }
  else if(strcmp("-help", argv[1]) == 0)
  {
    strcpy(output, "\nUsage: sprintf -<options>\n\n");
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
  else //In case the param 1 isn't none of the previous
    write(STDOUT_FILENO, "Type 'sfind -help' for instructions\n", 36);

  return 0;

}
