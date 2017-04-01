#ifndef _ABOUT_H_
#define _ABOUT_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <wait.h>
#include "finder.h"

#define PRINT 1
#define DELETE 2
#define MAX_STR_LEN 256
#define OPTION 3
#define ACTION 4

void print_help_menu ();

int test_arg(char *arg, int op);

char* extract_dir(char *str);

char* previous_dir(char *curr, int size);

int seacher (char *dir, char *option, char *filename, char *action);

int search_for_name (char *dir, char *filename, int op);

#endif
