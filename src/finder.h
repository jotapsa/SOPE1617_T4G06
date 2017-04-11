#ifndef _FINDER_H_
#define _FINDER_H_

//***************************
//Foward dependencies


#define MAX_STR_LEN 512

/*Parameters*/
#define OPTION 1
#define ACTION 2

/*Actions*/
#define PRINT 3
#define DELETE 4
#define EXECUTE 5

/*Types*/
#define DIRECTORY 6
#define FILE 7
#define LINK 8

void print_help_menu ();

void sigint_handler(int signal);

int test_arg(char *argv[]);

int searcher (char *dir, char *argv[]);

#endif //_FINDER_H_
