#ifndef _SAUNA_H_
#define _SAUNA_H_

typedef struct {
  int *pipe_fd;
  unsigned long dur;
} thread_info;

typedef enum {RECIEVED, REJECTED, SERVED} tip;

void print_help_menu ();

static inline char *tipToString (tip t);

void regMsg (char * reg, request_t *req, info_t *info, tip t);

void freeSlot ();

#endif
