#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

#include "io.h"

static void getFileMode(mode_t st_mode, char *mode)
{
	/*
	  	p--管道[
		l--链接
		s_--套接口
		b--块设备
		c--字符
		如果是设备文件（b,c），则在显示文件大小的位置显示的是设备的主驱动号和次驱动号
		buf.st_rdev >>8 ,  buf.st_rdev&0xFF
	*/
	switch(st_mode & S_IFMT)
	{
		/* type of file ，文件类型掩码*/
		case S_IFSOCK: 
			mode[0] ='s';
			break;

		/* type of file ，文件类型掩码*/
		case S_IFMT: 
			mode[0] ='m';
			break;
		/* regular 普通文件*/
		case S_IFREG: 
			mode[0] ='-';
			break;
		/* block special 块设备文件*/
		case S_IFBLK: 
			mode[0] ='b';
			break;
		/* directory 目录文件*/
		case S_IFDIR: 
			mode[0] ='d';
			break;
		/* character special 字符设备文件*/
		case S_IFCHR: 
			mode[0] ='c';
			break;
		/* fifo */
		case S_IFIFO: 
			mode[0] ='i';
			break;
		/* symbolic link 链接文件*/
		case S_IFLNK: 
			mode[0] ='l';
			break;
	}
	mode[1] = st_mode & S_IRUSR ? 'r':'-';
	mode[2] = st_mode & S_IWUSR ? 'w':'-';
	mode[3] = st_mode & S_IXUSR ? 'x':'-';
	mode[4] = st_mode & S_IRGRP ? 'r':'-';
	mode[5] = st_mode & S_IWGRP ? 'w':'-';
	mode[6] = st_mode & S_IXGRP ? 'x':'-';
	mode[7] = st_mode & S_IROTH ? 'r':'-';
	mode[8] = st_mode & S_IWOTH ? 'w':'-';
	mode[9] = st_mode & S_IXOTH ? 'x':'-';
}

static void getUserName(uid_t uid, char *uname)
{
	struct passwd *passwd;
    passwd = getpwuid (uid);
    strcpy(uname, passwd->pw_name);
}

static void getGroupName(gid_t gid, char *gname)
{
	struct group *grp;
	grp = getgrgid(gid);
	if (grp != NULL)
	{		
		strcpy(gname, grp->gr_name);
	}else{
		strcpy(gname, "Unkown");		
	}
}

char months[12][5] = {"Jan", "Feb", "Mar", "Api", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static char * getMonth(int n)
{
	return months[n-1];
}

static void getTime(time_t tm_t, char *tm_str)
{
	struct tm *mtime;
	char tmp[20] = {0};
	mtime = localtime(&tm_t);
	if (mtime->tm_year + 1900 == 2013)
	{		
		sprintf(tmp, "%s %d %d:%02d", getMonth(mtime->tm_mon), mtime->tm_mday, mtime->tm_hour, mtime->tm_min );
	}else{
		sprintf(tmp, "%s %d %d", getMonth(mtime->tm_mon), mtime->tm_mday, mtime->tm_year + 1900 );
	}
	strcpy( tm_str, tmp );
}

void my_list(int data_conn_sock ,char *filepath)
{
	char mymode[11]= "----------";
	struct stat buf;
	struct dirent *p = NULL;
	struct passwd *passwd;	
	char uname[50];
	char gname[50];
	char mtime[50];
	DIR *mydir =  NULL;
	char buffer[100];

	chdir(filepath);
	mydir = opendir(filepath);

	while( (p = readdir(mydir)) != NULL )
	{		
		memset( &buf, 0, sizeof(buf));
		stat(p->d_name,&buf);
		
		getFileMode(buf.st_mode, mymode);
		getUserName(buf.st_uid, uname);
		getGroupName(buf.st_gid, gname);
		getTime(buf.st_mtime, mtime);

		sprintf(buffer,"%s %d %s %s %10d %s %s\r\n", mymode, buf.st_nlink, uname, gname, buf.st_size, mtime, p->d_name);
		if (data_conn_sock != 0)
		{			
			write_loop(data_conn_sock, buffer, strlen(buffer));
		}
	}

	closedir(mydir);
}


