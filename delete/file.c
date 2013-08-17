#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 5

int copy(char *dest, char *src)
{
	int fd_src, fd_dest;
	char buf[BUFSIZE];
	if (argc != 3)
	{	
		help();
	}
	fd_src = open(src, O_RDONLY);
	if(fd_src == -1)
	{
		printf("open failed!\n");
		exit(-1);
	}
	fd_dest = open(dest, O_CREAT | O_WRONLY, 0666);
	if(fd_dest == -1)
	{
		printf("open fale!\n");
		exit(-1);
	}

	while(read(fd_src, buf, BUFSIZE) )
	{
		printf("%s", buf);
	}
	close(fd_src);
	close(fd_dest);
	printf("\ncopy succeed!\n");
	return 0;
}


