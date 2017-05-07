#ifndef _SHARED_H_
#define _SHARED_H_

/* Creates FIFO file, only if one doesn't already exist in the specified path*/
int createFIFO (const char* file);

/*Parse long value from string*/
unsigned long parse_ulong(char *str, int base);

#endif
