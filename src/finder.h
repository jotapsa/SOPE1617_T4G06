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

int test_arg(char *argv[]);

char* extract_dir(char *str);

int get_type(char *type);

char* get_important_digits(char *digits);

int compare_file_perm(char *perm, mode_t file);

char* get_new_path (char *str1, char *str2);

int file_destroyer (char *filename, int type);

char* get_path(char *dir, char *sub);

int searcher (char *dir, char *argv[]);

#endif //_FINDER_H_
