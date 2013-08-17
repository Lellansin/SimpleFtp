#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "config.h"

extern char LOCAL_IP[20];
extern int PORT_LISTEN;
extern int MAX_COUNT;
extern int MAX_IP_COUNT;
extern int LIMIT_SPEED;

int strrwd(char *src, char *key, char *value)
{
	char *p,*q;
	int len;
	p = strchr(src, '=');
	q = strchr(src, '\n');

	if (p!=NULL && q!=NULL)
	{
		*q = '\0';
		strncpy(key, src, p - src);
		strcpy(value, p+1);
	}
}

void config()
{
	FILE *fd;
	char buf[50]="";
	char key[50]="";
	char value[50]="";

	fd = fopen("config.txt", "r");

	if (fd == NULL)
	{
		printf("配置文件打开失败！\n");
		exit(-1);
	}

	while(fgets(buf, 50, fd))
	{
		if (strrwd(buf, key, value))
		{
			if (strcmp(key, "ip") == 0)
			{				
				strcpy(LOCAL_IP, value);
			}else if(strcmp(key, "port") == 0)
			{
				PORT_LISTEN = atoi(value);
			}else if(strcmp(key, "max_count") == 0)
			{
				MAX_COUNT = atoi(value);
			}else if(strcmp(key, "max_ip_count") == 0)
			{
				MAX_IP_COUNT = atoi(value);
			}else if(strcmp(key, "limit_speed") == 0)
			{
				LIMIT_SPEED = atoi(value);
			}
			memset(key, 0, strlen(key));
		}
	}

	fclose(fd);
}
