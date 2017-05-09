#ifndef _SHARED_H_
#define _SHARED_H_

#define REG_MAXLEN 70

typedef struct {
  clock_t t0;
  pid_t pid;
  int rejectsFileDes;
  int entriesFileDes;
  int registerFileDes;
} info_t;

typedef struct {
  unsigned long id;
  char gender;
  unsigned long dur;
  int denials;
} request_t;

typedef struct {
  unsigned long total, free;
  char gender;
} sauna_t;

/* Creates FIFO file, only if one doesn't already exist in the specified path*/
int createFIFO (const char* file);

/* If a regular file already exists in the path returns 0.
  If there is a file but not a regular file it deletes it returns 0
  If no file returns 0*/
int checkPathREG (const char* file);

/*Parse long value from string*/
unsigned long parse_ulong(char *str, int base);

#endif
