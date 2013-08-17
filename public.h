
#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#define ERR_EXIT(m) (perror(m), exit(EXIT_FAILURE))

int login(char uid[],char pwd[]);

#endif

