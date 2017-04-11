#include <stdio.h>
#include <sys/types.h>
#include <signal.h>

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
