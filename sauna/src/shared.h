#ifndef _SHARED_H_
#define _SHARED_H_

#define REGFILE_MAXLEN 15
#define REG_MAXLEN 70

#define MAX_DENIALS 3

#define MILLISECONDS_PER_SECOND 1000
#define NANOSECONDS_PER_MILLISECOND 1000000

#define MALE 0
#define FEMALE 1

typedef struct {
  struct timespec t0;
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
  unsigned long total_req;
} request_t;

typedef struct {
  unsigned long total;
  unsigned long free;
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
