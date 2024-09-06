/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

extern int errno;

int write(int fd, char *buffer, int size);

int read(char *b, int maxchars);

void itoa(int a, char *b);

int strlen(char *a);

void perror();

int getpid();

int fork();

void exit();

int yield();

int get_stats(int pid, struct stats *st);

int gotoxy(int x, int y); 

int set_color(int fg, int bg);

void* shmat(int id, void* addr); 

int shmdt(void* addr); 

int shmrm(int id); 

int gettime();

#endif  /* __LIBC_H__ */
