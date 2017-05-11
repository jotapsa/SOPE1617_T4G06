#include <stdio.h>
#include <stdlib.h> //exit, etc...
#include <unistd.h> //pid_t
#include <time.h>
#include <fcntl.h> //file handling
#include <limits.h> //ULONG_MAX
#include <string.h> //strtok
#include <pthread.h>

#include "shared.h"
#include "sauna.h"

pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER; // mutex para a s.c.
sauna_t sauna; //variaveis partilhadas

void print_help_menu (){
    printf ("\nUsage: sauna <n. lugares>\n\n");
}

static inline char *tipToString (tip t){
  static char *str_tip[] = {"RECEBIDO", "REJEITADO", "SERVIDO"}; //static so we only initialize once
  return str_tip[t];
}

void regMsg (char * reg, request_t *req, info_t *info, tip t){
  clock_t t1 = clock ();
  double long elapsedTime=0;

  elapsedTime = ((t1-info->t0) / CLOCKS_PER_SEC)*1000;
  snprintf (reg, REG_MAXLEN, "%Lf - %d - %lu: %c - %lu - %s\n", elapsedTime, info->pid, req->id, req->gender, req->dur, tipToString(t));
}

void incrementIndex (unsigned long *i, unsigned long total){
  if ((*i)+1 < total){
    (*i)++;
  }
  else {
    (*i)=0;
  }
}

void freeSlot (){
  pthread_mutex_lock (&mut);
  sauna.free ++;
  pthread_mutex_unlock (&mut);
}

void *fulfillReq (void *arg){
  struct timespec req;
  pthread_t tid;
  thread_info *t = (thread_info *) arg;

  req.tv_sec = t->dur / 1000;
  req.tv_nsec = (t->dur % 1000) * 1000000; //convert the remainder to nsec

  if (nanosleep(&req, NULL)){
    fprintf(stderr, "Error nanosleep\n");
  }

  freeSlot ();

  tid = pthread_self();
  write(t->pipe_fd[1], &tid, sizeof(pthread_t));

  return NULL;
}

int main  (int argc, char *argv[], char *envp[]){
  info_t info;
  info.t0 = clock ();

  clock_t end;

  info.pid = getpid();
  char *rejectsFIFOPath = "/tmp/rejeitados";
  char *entriesFIFOPath = "/tmp/entrada";
  char *registerPath = malloc(15*sizeof(char)); //pid_t is a signed integer
  snprintf(registerPath, 15, "/tmp/bal.%d", info.pid);

  request_t req;
  char reg[REG_MAXLEN];

  thread_info *t;
  int pipe_fd[2];
  unsigned long i=0, total;
  pthread_t tid;

  if (argc != 2){
    print_help_menu ();
    exit (0);
  }
  else{
    sauna.total = parse_ulong (argv[1], 10);
    if (sauna.total == ULONG_MAX || sauna.total <= 0){
      fprintf(stderr, "Invalid parameter\n");
      print_help_menu ();
      exit(1);
    }
    sauna.free = sauna.total;
    total = sauna.total;
  }
  t = malloc (sizeof(thread_info) * sauna.total); //its impossible to be waiting for more than "sauna.total" threads to spawn so we can just give each thread a different instance
  pipe (pipe_fd);

  if (checkPathREG(registerPath) != 0){
    exit(1);
  }

  if ((info.registerFileDes=open(registerPath, O_WRONLY|O_CREAT|O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))==-1){
    fprintf(stderr, "Error opening file %s\n", registerPath);
    exit (2);
  }


  if (createFIFO (rejectsFIFOPath) != 0 || createFIFO (entriesFIFOPath) != 0){
    exit (1);
  }

  //since we dont give the O_NONBLOCK FLAG to open the process must wait until some other process opens the FIFO for reading
  if ((info.rejectsFileDes = open (rejectsFIFOPath, O_WRONLY | O_TRUNC | O_SYNC)) == -1){
    fprintf(stderr, "Error opening file %s\n", rejectsFIFOPath);
    exit (2);
  }

  if ((info.entriesFileDes = open (entriesFIFOPath, O_RDONLY))==-1){
    fprintf(stderr, "Error opening file %s\n", entriesFIFOPath);
    exit (2);
  }

  //Read from the ENTRIES FIFO
  while (read (info.entriesFileDes, &req, sizeof(request_t))>0){
    t[i].pipe_fd = pipe_fd;
    t[i].dur = req.dur;
    pthread_mutex_lock (&mut);
    if (sauna.free == sauna.total){
      //EMPTY SAUNA
      sauna.gender = req.gender;
      sauna.free --;
      pthread_mutex_unlock (&mut);
      pthread_create (&tid, NULL, fulfillReq, &t[i]);
      incrementIndex (&i, total);
    }
    else if ((req.gender == sauna.gender) && (sauna.free)){
      //still slots available
      sauna.free --;
      pthread_mutex_unlock (&mut);
      pthread_create (&tid, NULL, fulfillReq, &t[i]);
      incrementIndex (&i, total);
      //create thread
    }
    else if ((req.gender == sauna.gender) && (!sauna.free)){
      //full sauna but same gender
      //wait
      //sauna.free --;
      pthread_mutex_unlock (&mut);
      /*
      pthread_create (&tid, NULL, fulfillReq, &t[i]);
      incrementIndex (&i, total);
      */
      //create thread;
    }
    else if ((req.gender != sauna.gender)){
      //diferent gender
      pthread_mutex_unlock (&mut);
      req.denials++;
      write (info.rejectsFileDes, &req, sizeof(request_t));

      regMsg (reg, &req, &info, REJECTED);
      write (info.registerFileDes, reg, strlen(reg));

    }
    else {
      pthread_mutex_unlock (&mut); //just making sure
      printf ("ERRO\n");
    }
    printf ("p%lu %c t%lu\n", req.id, req.gender, req.dur);
  }

  //Wait for all the created threads, reading each tid through a pipe
  while (read(pipe_fd[0], &tid, sizeof(tid))>0){
    pthread_join(tid, NULL);
  }

  if (close (pipe_fd[0]) == -1){
    fprintf(stderr, "Error closing the receptor side of the pipe\n");
    exit (3);
  }

  if (close (pipe_fd[1]) == -1){
    fprintf(stderr, "Error closing the emitor side of the pipe\n");
    exit (3);
  }


  if (close (info.entriesFileDes) == -1){
    fprintf(stderr, "Error closing file %s\n", entriesFIFOPath);
    exit (3);
  }

  if (close (info.rejectsFileDes) == -1){
    fprintf(stderr, "Error closing file %s\n", rejectsFIFOPath);
    exit (3);
  }

  if (close (info.registerFileDes) == -1){
    fprintf(stderr, "Error closing file %s\n", registerPath);
    exit (3);
  }

  free (t);


  end = clock(); //fim da medicao de tempo

  printf("Clock:  %4.2Lf s\n", (long double)(end-info.t0)/(CLOCKS_PER_SEC));

  return 0;
}
