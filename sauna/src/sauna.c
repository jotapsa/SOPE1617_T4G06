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

pthread_cond_t cvar=PTHREAD_COND_INITIALIZER;
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
  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  double long elapsedTime=0;

  elapsedTime = ((t1.tv_sec - info->t0.tv_sec)*MILLISECONDS_PER_SECOND)
    + ((t1.tv_nsec - info->t0.tv_nsec )/NANOSECONDS_PER_MILLISECOND);
  snprintf (reg, REG_MAXLEN, "%Lf - %d - %lu: %c - %lu - %s\n", elapsedTime, info->pid, req->id, req->gender, req->dur, tipToString(t));
  //printf ("%s",reg); //Uncomment to print REGISTER message
}

void incrementIndex (unsigned long *i, unsigned long total){
  if ((*i)+1 < total){
    (*i)++;
  }
  else {
    (*i)=0;
  }
}

void init_stats (stats_t *stats){
  stats->req_rec[MALE]=0;
  stats->req_rec[FEMALE]=0;

  stats->req_rej[MALE]=0;
  stats->req_rej[FEMALE]=0;

  stats->req_serv[MALE]=0;
  stats->req_serv[FEMALE]=0;
}

void update_stats (stats_t *stats, request_t req, tip t){
  switch (t){
    case RECIEVED:{
      if (req.gender == 'M')
        stats->req_rec[MALE]++;
      else
        stats->req_rec[FEMALE]++;
    }
    break;
    case REJECTED:{
      if (req.gender == 'M')
        stats->req_rej[MALE]++;
      else
        stats->req_rej[FEMALE]++;
    }
    break;
    case SERVED:{
      if (req.gender == 'M')
        stats->req_serv[MALE]++;
      else
        stats->req_serv[FEMALE]++;
    }
    break;
    default:
    break;
  }
}

void print_stats (stats_t *stats){
  printf("A sauna:\n");
  printf("Recebeu um total de %lu pedidos\n", stats->req_rec[MALE]+stats->req_rec[FEMALE]);
  printf("\t%lu [M] e %lu [F]\n", stats->req_rec[MALE], stats->req_rec[FEMALE]);
  printf("Rejeitou um total de %lu pedidos\n", stats->req_rej[MALE]+stats->req_rej[FEMALE]);
  printf("\t%lu [M] e %lu [F]\n", stats->req_rej[MALE], stats->req_rej[FEMALE]);
  printf("Serviu um total de %lu pedidos\n", stats->req_serv[MALE]+stats->req_serv[FEMALE]);
  printf("\t%lu [M] e %lu [F]\n", stats->req_serv[MALE], stats->req_serv[FEMALE]);
}

void freeSlot (){
  pthread_mutex_lock (&mut);
  sauna.free ++;
  pthread_cond_signal(&cvar);
  pthread_mutex_unlock (&mut);
}

void *fulfillReq (void *arg){
  struct timespec req;
  thread_info *t = (thread_info *) arg;

  pthread_t tid = pthread_self();
  write(t->pipe_fd[1], &tid, sizeof(pthread_t)); //better write this at the start so the main thread knows to wait for this one

  req.tv_sec = t->dur / 1000;
  req.tv_nsec = (t->dur % 1000) * 1000000; //convert the remainder to nsec

  if (nanosleep(&req, NULL)){
    fprintf(stderr, "Error nanosleep\n");
  }

  freeSlot ();

  pthread_exit (0);
}

int main  (int argc, char *argv[], char *envp[]){
  info_t info;
  clock_gettime(CLOCK_MONOTONIC, &info.t0);

  info.pid = getpid();
  char *rejectsFIFOPath = "/tmp/rejeitados";
  char *entriesFIFOPath = "/tmp/entrada";
  char registerPath[REGFILE_MAXLEN]; //pid_t is a signed integer
  snprintf(registerPath, REGFILE_MAXLEN, "/tmp/bal.%d", info.pid);

  request_t req;
  char reg[REG_MAXLEN];

  thread_info *t;
  int pipe_fd[2];
  unsigned long i=0, total_slots;
  unsigned long spawned_threads=0;
  pthread_t tid;

  unsigned long total_served=0;
  stats_t stats;
  init_stats (&stats);

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
    total_slots = sauna.total;
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
  while ((read (info.entriesFileDes, &req, sizeof(request_t))>0)){
    update_stats (&stats, req, RECIEVED);
    regMsg (reg, &req, &info, RECIEVED);
    write (info.registerFileDes, reg, strlen(reg));

    t[i].pipe_fd = pipe_fd;
    t[i].dur = req.dur;
    pthread_mutex_lock (&mut);
    if (sauna.free == sauna.total){
      //EMPTY SAUNA
      sauna.gender = req.gender;
      sauna.free --;
      pthread_mutex_unlock (&mut);
      pthread_create (&tid, NULL, fulfillReq, &t[i]);

      spawned_threads++;
      incrementIndex (&i, total_slots);
      total_served++;
      update_stats (&stats, req, SERVED);
      regMsg (reg, &req, &info, SERVED);
      write (info.registerFileDes, reg, strlen(reg));
    }
    else if ((req.gender == sauna.gender) && (sauna.free)){
      //still slots available
      sauna.free --;
      pthread_mutex_unlock (&mut);
      pthread_create (&tid, NULL, fulfillReq, &t[i]);

      spawned_threads++;
      incrementIndex (&i, total_slots);
      total_served++;
      update_stats (&stats, req, SERVED);
      regMsg (reg, &req, &info, SERVED);
      write (info.registerFileDes, reg, strlen(reg));
    }
    else if ((req.gender == sauna.gender) && (!sauna.free)){
      //full sauna but same gender
      //wait while it's full
      while (!sauna.free){
        pthread_cond_wait(&cvar, &mut);
      }
      sauna.free --;
      pthread_mutex_unlock (&mut);
      pthread_create (&tid, NULL, fulfillReq, &t[i]);

      spawned_threads++;
      incrementIndex (&i, total_slots);
      total_served++;
      update_stats (&stats, req, SERVED);
      regMsg (reg, &req, &info, SERVED);
      write (info.registerFileDes, reg, strlen(reg));
    }
    else if ((req.gender != sauna.gender)){
      //diferent gender
      pthread_mutex_unlock (&mut);
      req.denials++;
      write (info.rejectsFileDes, &req, sizeof(request_t));

      if (req.denials >= MAX_DENIALS){
        total_served++;
      }
      update_stats (&stats, req, REJECTED);
      regMsg (reg, &req, &info, REJECTED);
      write (info.registerFileDes, reg, strlen(reg));
    }
    else {
      pthread_mutex_unlock (&mut); //just making sure
      printf ("ERRO\n");
    }

    //have we served every request ?
    if (total_served >= req.total_req){
      break;
    }
  }

  //Wait for all the created threads, reading each tid through a pipe
  for (i=0; i<spawned_threads; i++){
    read(pipe_fd[0], &tid, sizeof(pthread_t));
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

  print_stats (&stats);
  printf ("\nFor more details: %s\n", registerPath);
  printf("******************FIM*****************\n");

  return 0;
}
