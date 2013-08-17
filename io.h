
#ifndef _IO_H_
#define _IO_H_

int read_loop(int fd,void* buf,int size);
int write_loop(int fd,void* buf,int size);
int read_peek(int fd,void *buf,int len);
int read_line(int fd,char *buf,int maxlen);

#endif


