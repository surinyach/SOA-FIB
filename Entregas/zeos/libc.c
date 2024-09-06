/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

#include <errno.h>

int errno;

void perror(void) 
{
	switch(errno)
	{
		case ENOSYS:
			write(1,"\nSyscall not implemented",strlen("\nSyscall not implemented"));
			break;
		case EFAULT:
			write(1,"\nIncorrect memory access", strlen("\nIncorrect memory access"));
			break;
		case EINVAL:
			write(1,"\nIncorrect size",strlen("\nIncorrect size"));
			break;
		case EBADF:
			write(1,"\nIncorrect file descriptor", strlen("\nIncorrect file descriptor"));
			break;
		case EACCES:
			write(1,"\nWrong access",strlen("\nWrong access"));
			break;
    case ENOMEM:
      write(1,"\nNot enough space", strlen("\nNot enough space"));
		default:
			break;
	}
}

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

