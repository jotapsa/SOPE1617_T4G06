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

void updateSlots (int change){
  pthread_mutex_lock (&mut);
  sauna.free += change;
  pthread_mutex_unlock (&mut);
}


long long freeSlot (){
  for (unsigned long i = 0; i<sauna.total; i++){
    if (sauna.slots[i] != -1){
      return i;
    }
  }
  return -1;
}

void *fulfillReq (void *arg){
  struct timespec req;
  thread_info *info = (thread_info *) arg;

  req.tv_sec = info->dur / 1000;
  req.tv_nsec = (info->dur % 1000) * 1000000; //convert the remainder to nsec

  if (nanosleep(&req, NULL)){
    fprintf(stderr, "Error nanosleep\n");
  }

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

  int bytes;
  request_t req;
  char reg[REG_MAXLEN];

  unsigned long index;
  pthread_t *tid;
  thread_info *tid_info;

  if (argc != 2){
    print_help_menu ();
    exit (0);
  }
  else{
    sauna.total = parse_ulong (argv[1], 10);
    if (sauna.total == ULONG_MAX){
      exit(1);
    }
    sauna.free = sauna.total;
  }
  tid = (pthread_t *) malloc (sizeof(pthread_t)*sauna.total);
  tid_info = (thread_info *) malloc (sizeof(thread_info)*sauna.total);
  sauna.slots = (int *) malloc (sizeof(int)*sauna.total);

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
  while ((bytes = read (info.entriesFileDes, &req, sizeof(request_t)))>0){
    if ((sauna.free == sauna.total) && (sauna.total>0)){
      //EMPTY SAUNA
      sauna.gender = req.gender;
      sauna.free --;
      index = freeSlot();
      tid_info[index].index = index;
      tid_info[index].dur = req.dur;
      pthread_create (&tid[index], NULL, fulfillReq, &tid_info);
    }
    else if ((req.gender == sauna.gender) && (sauna.free<sauna.total) && (sauna.free>0)){
      //still slots available
    }
    else if ((req.gender == sauna.gender) && (sauna.free==0) && (sauna.total>0)){
      //full sauna
    }
    else if ((req.gender != sauna.gender)){
      //diferent gender
      req.denials++;
      write (info.rejectsFileDes, &req, sizeof(request_t));

      regMsg (reg, &req, &info, REJECTED);
      write (info.registerFileDes, reg, strlen(reg));

    }
    printf ("p%lu %c t%lu\n", req.id, req.gender, req.dur);
  }

  for (unsigned long i=0; i<sauna.total; i++){
    pthread_join (tid[i], NULL);
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

  free (sauna.slots);
  free (tid);

  end = clock(); //fim da medicao de tempo

  printf("Clock:  %4.2Lf s\n", (long double)(end-info.t0)/(CLOCKS_PER_SEC));

  return 0;
}
