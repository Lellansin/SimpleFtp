#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include "io.h"

//接收指定字符个数
int read_loop(int fd,void* buf,int size)
{
	int ret;
	int num_read = 0;

	while (1)
	{
		ret = read(fd,buf+num_read,size);
		if (ret == -1)
		{
			if (errno == EINTR)//说明中断是因为信号而中断
				continue;
			else
				return -1;
		}

		if (ret == 0)
			return num_read;

		num_read += ret;
		size -= ret;
		if (size == 0)
			return num_read;
	}

}
//指定写的字符个数，一定要写完
int write_loop(int fd,void* buf,int size)
{
	int ret;
	int num_written = 0;

	while (1)
	{
		ret = write(fd,buf+num_written,size);
		if (ret == -1)
		{
			if (errno == EINTR)//被信号所中断
				continue;
			else
				return -1;
		}

		if (ret == 0)
			return num_written;

		num_written += ret;
		size -= ret;

		if (size == 0)
			return num_written;
	}
}

/*recv
MSG_PEEK：指示数据接收后，在接收队列中保留原数据，不将其删除，
随后的读操作还可以接收相同的数据。
*/
int read_peek(int fd,void *buf,int len)
{
	int ret = 0;
	for(;;)
	{
		ret = recv(fd,buf,len,MSG_PEEK);
		if(ret == -1 && errno == EINTR)
			continue;
		else
			return ret;
	}
}

//按行读取数据
int read_line(int fd,char *buf,int maxlen)
{
	char *p = buf;
	int num_get=maxlen,i,num_read,retval;
	for(;;)
	{
		retval = read_peek(fd,p,num_get);
		if (retval <= 0)
			return retval;

		num_read = retval;

		for(i=0;i<num_read;i++)
		{
			if(p[i] == '\n')
			{
				retval = read_loop(fd,p,i+1);
				if(retval != i+1)
				{
					return -1;
				}
				p = strchr(buf,'\n');
				*(++p) = '\0';
				return retval;
			}
		}

		num_get -= num_read;
		retval = read_loop(fd,p,num_read);
		if(retval != num_read)
			return -1;
		p += num_read;

	}
}
