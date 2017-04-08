#ifndef _FINDER_H_
#define _FINDER_H_

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

#define MAX_STR_LEN 512
/*Paramenters*/
#define OPTION 1
#define ACTION 2
/*Actions*/
#define PRINT 3
#define DELETE 4
#define EXECUTE 5
/*Types*/
#define FOLDER 6
#define FILE 7
#define LINK 8

void print_help_menu ();

void sigint_handler(int signal);

int test_arg(char *arg, int op);

char* extract_dir(char *str);

char* get_path(char *dir, char *sub);

int seacher (char *dir, char *option, char *filename, char *action);

int search_for_name (char *dir, char *filename, int op);

int search_for_type (char *dir, int type, int op);

#endif
