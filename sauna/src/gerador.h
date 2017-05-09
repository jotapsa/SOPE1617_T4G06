#ifndef _GERADOR_H_
#define _GERADOR_H_

typedef enum {GENERATED, REJECTED, DISCARDED} tip;

void print_help_menu ();

static inline char *tipToString (tip t);

void genRegMsg (char * reg, request_t *req, info_t *info, tip t);

void *genRequests (void *arg);

void *handleRejects (void *arg);

#endif
