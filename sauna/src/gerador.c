#include <stdio.h>
#include <time.h>
#include <stdlib.h> //rand, exit...
#include <unistd.h> //pid_t
#include <pthread.h>

int nrReq, maxTime;

void print_help_menu (){
    printf ("\nUsage: gerador <nr. pedidos> <max. utilizacao>\n\n");
}

void genRequests (void *arg){
  int r = rand();
}

int main  (int argc, char *argv[], char *envp[]){

  srand(time(NULL)); //initialize the seed from the current time
  pid_t pid;

  pthread_t tid[2];

  if (argc != 3){
    print_help_menu ();
    exit (0);
  }
  else{
    //validar argumento
  }


  pid = getpid();
  return 0;
}
