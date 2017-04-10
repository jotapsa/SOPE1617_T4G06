#ifndef _FINDER_H_
#define _FINDER_H_

#define MAX_STR_LEN 512

/*Parameters*/
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

int get_type(char *type);

int file_destroyer (char *filename, int type);

char* get_path(char *dir, char *sub);

int seacher (char *dir, char *option, char *filename, char *action);

int search_for_name (char *dir, char *filename, int op);

int search_for_type (char *dir, int type, int op);

int search_for_perm (char *dir, int perm, int op);

#endif //_FINDER_H_
