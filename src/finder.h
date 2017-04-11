#ifndef _FINDER_H_
#define _FINDER_H_

//***************************
//Foward dependencies


#define MAX_STR_LEN 512

/*Search Options*/
typedef enum {NAME, TYPE, PERMISSION} search_type;
/*Actions*/
typedef enum {PRINT, DELETE, EXECUTE} action_type;
/*File type*/
typedef enum {DIRECTORY, REGULAR, LINK} file_type;


int searcher (char *dir, char *argv[]);

#endif //_FINDER_H_
