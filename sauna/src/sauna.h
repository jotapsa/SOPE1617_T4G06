#ifndef _SAUNA_H_
#define _SAUNA_H_

typedef struct {
  int *pipe_fd;
  unsigned long dur;
} thread_info;

typedef struct {
  unsigned long req_rec[2];
  unsigned long req_rej[2];
  unsigned long req_serv[2];
} stats_t;

typedef enum {RECIEVED, REJECTED, SERVED} tip;

void print_help_menu ();

static inline char *tipToString (tip t);

void regMsg (char * reg, request_t *req, info_t *info, tip t);

void init_stats (stats_t *stats);

void update_stats (stats_t *stats, request_t req, tip t);

void print_stats (stats_t *stats);

void freeSlot ();

#endif
