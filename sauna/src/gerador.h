#ifndef _GERADOR_H_
#define _GERADOR_H_

typedef struct {
  unsigned long req_gen[2];
  unsigned long req_rej_rec[2];
  unsigned long req_rej_dis[2];
} stats_t;

typedef enum {GENERATED, REJECTED, DISCARDED} tip;

void print_help_menu ();

static inline char *tipToString (tip t);

void genRegMsg (char * reg, request_t *req, info_t *info, tip t);

void update_stats (stats_t *stats, request_t req, tip t);

void *genRequests (void *arg);

void *handleRejects (void *arg);

#endif
