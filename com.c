#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pwd.h>

#include "com.h"
#include "error.h"
#include "public.h"
#include "net.h"
#include "sem.h"
#include "speed.h"

extern int connfd;	// 命令连接socket
int pasv_connsock;	// pasv链接socket
uint16_t pasv_port;	// pasv链接端口

char port_ip[50];	// port链接ip
int port_port;		// port链接端口

int data_conn_sock;	// 数据连接socket
int rest_value;		// 文件续传偏移

char last_com[10];	// 上一个命令
char connuser[20];	// 当前用户名

pid_t MAIN_PID;		// 总进程的 pid

extern char DirNOW[PATH_LEN];	// 当前目录名
extern char LOCAL_IP[];			// 本地ip

int TMP_UPLOAD_BYTES;			// 临时上传字节
int TMP_DOWNLOAD_BYTES;			// 临时下载字节

extern int LIMIT_SPEED; 		// 限速
extern long transfer_data_now;	// 当前传输累计字节数
extern long transfer_data_last;	// 上一次传输累计字节数

struct 
{
	char com[PATH_LEN];
	void (*Func)(SESSION *nowsession);
} ComToFun[]= {
	{"USER", do_user },
	{"PASS", do_pass },
	{"SYST", do_syst },
	{"FEAT", do_feat },
	{"CLNT", do_clnt },
	{"PWD" , do_pwd  },
	{"TYPE", do_type },
	{"REST", do_rest },
	{"PASV", do_pasv },
	{"PORT", do_port },
	{"LIST", do_list },
	{"CWD" , do_cwd  },
	{"MKD" , do_mkd  },
	{"RMD" , do_rmd  },
	{"DELE", do_dele },
	{"RNFR", do_rnfr },
	{"RNTO", do_rnto },
	{"SITE", do_site },
	{"CDUP", do_cdup },
	{"STOR", do_stor },
	{"RETR", do_retr },
	{"SIZE", do_size },
	{"MDTM", do_mdtm },
} ;


void auto_switch(SESSION *nowsession)
{
	int len =sizeof(ComToFun)/sizeof(ComToFun[0]);
	int i;

	for (i = 0; i < len; ++i)
	{
		if ( strcmp(ComToFun[i].com, nowsession->com) == 0 )
		{
			ComToFun[i].Func(nowsession);
			statistics();
			break;
		}
	}

	if (i >= len)
	{
		printf("Commond error: not Found\nThe command is : [%s] \n", nowsession->com);
		char *text = "500 Unknown command.\r\n";
		write_loop(connfd, text, strlen(text));
	}
}

void get_com(char *recvbuf, SESSION *nowsession)
{
	char tmp[PATH_LEN];
	char *p;

	strcpy(tmp, recvbuf);
	p = strchr(tmp, ' ');

	if (p != NULL)
	{
		*p = '\0';
		strcpy(nowsession->com, tmp);
	}else if( (p = strchr(tmp, '\r')) != 0)
	{
		*p = '\0';
		strcpy(nowsession->com, tmp);
	}else{
		printf("com get error\n");
	}
}


void get_param(char *recvbuf, SESSION *nowsession)
{
	char tmp[PATH_LEN];
	char *p=NULL,*r=NULL;

	strcpy(tmp, recvbuf);
	p = strchr(tmp, ' ');
	r = strchr(tmp, '\r');

	if (p != NULL && r != NULL)
	{
		*r = '\0';
		strcpy(nowsession->param, p+1);
	}else
	{
		// printf("param get error\n");
	}
}

void get_dir(char *recvbuf, SESSION *nowsession)
{
	char buf[200];
	if (recvbuf == NULL)
	{
		if ( getcwd(buf, sizeof(buf)-1) != 0)
		{
			strcpy(nowsession->dir, buf);
		}else{
			// printf("error getcwd\n");
			perror("get_dir");
		}
	}else{
		char *p=NULL,*r=NULL;
		strcpy(buf, recvbuf);
		p = strchr(buf, '\r');
		r = strrchr(buf, '\r');
		if (p != NULL && r != NULL)
		{
			*r = '\0';
			strcpy(nowsession->dir, p+1);
		}
	}
}

char Uname[20];

void do_user(SESSION *nowsession)
{
	// 检查用户名

	strcpy(Uname, nowsession->param);
	strcpy(connuser, Uname);

	char *text = "331 Please send password\r\n";
	write_loop(connfd, text, strlen(text));
}

void do_pass(SESSION *nowsession)
{
	// 检查密码
   	if(login(Uname, nowsession->param))
   	{
   		// printf("密码检查成功，准备回复验证\n");
	    char buf[1024];
		memset(buf,0,sizeof(buf));
		char *text = "230 Login successful\r\n";
		write_loop(connfd, text, strlen(text));
		
		struct passwd *sp = getpwnam(Uname);
		if(sp!=NULL)
		{
			
		int res=setegid(sp->pw_gid);
			if(res!=0)
				ERR_EXIT("setegid");
			res=seteuid(sp->pw_uid);
			if(res!=0)
				ERR_EXIT("setegid");
				
			res=chdir(sp->pw_dir);
			if(res!=0)
				ERR_EXIT("setegid");
		}
	}else{
		write_loop(connfd, "530 Permission denied.\r\n", strlen("530 Permission denied.\r\n"));
	}
}


void do_syst(SESSION *nowsession)
{
	// 发送当前状态
	char *text = "215 Unix Type: L8\r\n";
	write_loop(connfd, text, strlen(text));
}

void do_feat(SESSION *nowsession)
{
	// 告诉客户端特性
	char *text = "211-Features: \r\n EPRT\r\n EPSV\r\n MDTM\r\n PASV\r\n REST STREAM\r\n SIZE\r\n TVFS\r\n211 End\r\n";
	write_loop(connfd, text, strlen(text));
}

void do_clnt(SESSION *nowsession)
{
	// 不知名命令
	char *text = "500 Unknown command.\r\n";
	write_loop(connfd, text, strlen(text));
}

void do_pwd(SESSION *nowsession)
{
	// 返回当前目录
	char text[PATH_LEN];
	sprintf(text,"257 \"%s\"\r\n", nowsession->dir);
	write_loop(connfd, text, strlen(text));
}

void do_type(SESSION *nowsession)
{
	// A I ascii 或 二进制
	char *text = "200 Switching to ASCII mode.\r\n";
	write_loop(connfd, text, strlen(text));
}

void do_rest(SESSION *nowsession)
{
	// A I ascii 或 二进制
	char text[PATH_LEN];
	rest_value = atoi(nowsession->param);
	// printf("接收到rest偏移量 %d\n", rest_value);
	sprintf(text,"350 Restart position accepted (%d).\r\n", rest_value);
	write_loop(connfd, text, strlen(text));
}

void do_port(SESSION *nowsession)
{
	write_loop(connfd, "200 port ok\r\n", strlen("200 port ok\r\n"));

	char tempport[10],temp[50];
	
	int i,n1,n2,n3,n4,n5,n6;
	unsigned int port1,port2;
	
	strcpy(temp, nowsession->param);
	sscanf(temp,"%d,%d,%d,%d,%d,%d",&n1,&n2,&n3,&n4,&n5,&n6);
	sprintf(port_ip,"%d.%d.%d.%d",n1,n2,n3,n4);
	port_port=n5*256+n6;

	strcpy(last_com,"PORT");
}

void do_pasv(SESSION *nowsession)
{
	// 客户端主动连接模式

	struct sockaddr_in s_addr, tempaddr;
	char buf[200];
	char tempip[30];
	uint16_t port1,port2;

	// 从配置文件中获取
	strcpy(tempip, LOCAL_IP);

	char *p=tempip;
	for ( ; *p != '\0'; p++)
	{
		if (*p == '.')
		{	
			*p = ',';
		}
	}

	memset(&s_addr, 0, sizeof(s_addr));
	pasv_connsock = socket(AF_INET, SOCK_STREAM, 0);

	s_addr.sin_family=AF_INET;
	s_addr.sin_port=htons(0);
	s_addr.sin_addr.s_addr=htonl(INADDR_ANY);

	// 设置地址重用
	int opt=1;
	int ret = setsockopt(pasv_connsock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	if (ret == -1)
		ERR_EXIT("setsockopt");
	// printf("正在获取 pasv_connsock 2\n");

	bind(pasv_connsock, (struct sockaddr*)&s_addr,sizeof(s_addr));
	listen(pasv_connsock, 10);

	// 获取端口
	socklen_t templen=sizeof(tempaddr);
	int res = getsockname(pasv_connsock,(struct sockaddr*)&tempaddr,(socklen_t*)&templen);

	if(res==-1)
		ERR_EXIT("getsockname");

	pasv_port=ntohs(tempaddr.sin_port);
	port1=pasv_port>>8;
	port2=pasv_port&0xFF;

	// 返回信息
	sprintf(buf, "227 Entering Passive Mode (%s,%d,%d)\r\n", tempip, port1, port2);
	write_loop(connfd, buf, strlen(buf));

	strcpy(last_com,"PASV"); //last_com用来记下数据传输命令的前一条命令是pasv还是PORT
}

int ipc_socket(int domain,int type,int protocol)
{
	int fd;
	fd = socket(domain,type,protocol);
	if (fd == -1)
		ERR_EXIT("ipc_socket");
	return fd;
}

int set_euid_egid(char *str)
{
	int res = setegid((gid_t)str);
	if(res!=0)
		ERR_EXIT("setegid");
    res = setegid((uid_t)str);
	if(res!=0)
		ERR_EXIT("seteuid");
}


int test_pasv_or_port()
{
	if (strcmp(last_com, "PORT") == 0) // port 模式
	{
		printf("PORT 模式正在连接...\n");
		perror("PORT 模式正在连接");

		struct sockaddr_in cli_addr;
		struct sockaddr_in ser_addr;
		int sockk;
		socklen_t addrlen = sizeof(cli_addr);

		// 客户端地址
		memset(&cli_addr,0,sizeof(cli_addr));
		cli_addr.sin_family=AF_INET;
		cli_addr.sin_port=htons(port_port);
		cli_addr.sin_addr.s_addr=inet_addr(port_ip);
		perror("PORT1");
		printf("客户端ip: %s 端口: %d\n",port_ip ,port_port);
		
		// 服务端地址
		memset(&ser_addr,0,sizeof(ser_addr));
		ser_addr.sin_family=AF_INET;
		ser_addr.sin_port=htons(20);
		ser_addr.sin_addr.s_addr=htonl(INADDR_ANY);
		perror("PORT2");
		printf("服务器ip: %s 端口: %d\n", inet_ntoa(ser_addr.sin_addr), 20);

		// 主动连接时bind需要root用户权限，被动连接不需要
		set_euid_egid("root");
		sockk = socket(AF_INET,SOCK_STREAM,0);
		perror("PORT3");

		// 设置地址重用		
		int opt=1;
		int ret = setsockopt(sockk,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
		if (ret == -1)
			ERR_EXIT("sockk_setsockopt");
		perror("PORT4");
		ret = bind(sockk,(struct sockaddr*)&ser_addr,sizeof(ser_addr));
		if (ret == -1)
			ERR_EXIT("sockk_bind");
		perror("PORT5");
		printf("绑定服务器端成功 socket id %d\n", socket);
		ret = connect(sockk,(struct sockaddr*)&cli_addr, addrlen);
		printf("%d\n", ret);
		if (ret == -1)					
			ERR_EXIT("DATA_CONN_SOCK");
		perror("PORT6");
		data_conn_sock=sockk;
		set_euid_egid(connuser);	
		perror("PORT3");
		printf("PORT 操作结束\n");

	}else if(strcmp( last_com, "PASV" )==0)  // pasv模式
	{
		struct sockaddr_in cliaddr;
		memset(&cliaddr,0,sizeof(cliaddr));
		socklen_t cliaddrlen = sizeof(cliaddr);
		data_conn_sock=accept(pasv_connsock,(struct sockaddr*)&cliaddr,&cliaddrlen);
		if(data_conn_sock==-1)
			ERR_EXIT("pasv sock");
	}
	return data_conn_sock;
}


void do_list(SESSION *nowsession)
{
	// 传递列表
	int res;
	data_conn_sock=test_pasv_or_port();// 建立数据连接通道
	write_loop(connfd,"150 start\r\n", strlen("150 start\r\n"));
	// 此处往data_conn_sock写list信息
	my_list(data_conn_sock, nowsession->dir);
	write_loop(connfd, "226 end\r\n", strlen("226 end\r\n"));	
	if (strcmp(last_com, "PASV")==0)
	{		
		close(pasv_connsock);
		memset(last_com, 0, sizeof(last_com));
	}
	close(data_conn_sock);
}

void do_cwd(SESSION *nowsession)
{
	// 切换目录
	char temp[100];
	memset(temp,0,sizeof(temp));
	char *p, *t=nowsession->dir;
	if((p=strchr(nowsession->param,'/'))==NULL)
	{
		printf("hello %s\n", p);
		while(*t++)
		{
			if ((*t == '/') && (*(t+1) == '\0') && (t != nowsession->dir))
			{
				*t = '\0';
			}
		}
		sprintf(temp,"%s/%s",nowsession->dir,nowsession->param);
	}else{
		strcpy(temp, nowsession->param);		
	}
	int res = chdir(temp);

	if(res==-1)
	{
		char *text1 = "550 Failed to change directory.\r\n";
		write_loop(connfd, text1, strlen(text1));
		return ;
	}

	strcpy(nowsession->dir, temp);
	strcpy(DirNOW, temp);

	char *text2 = "250 Directory successfully changed.\r\n";
	write_loop(connfd, text2, strlen(text2));	
}

void do_cdup(SESSION *nowsession)
{
	// 跳至上级目录
	char text[PATH_LEN];
	sprintf(text, "%s/../", nowsession->dir);
	chdir(text);
	char *text2 = "250 Directory successfully changed.\r\n";
	write_loop(connfd, text2, strlen(text2));	
}

void do_mkd(SESSION *nowsession)
{
	// 创建目录
	char text[PATH_LEN];
	if ( mkdir(nowsession->param, (mode_t)0755) ) // 创建目录 成功返回零 失败则
	{
		sprintf(text, "500 Failed to create directory [%s].\r\n", nowsession->param);
		write_loop(connfd, text, strlen(text));
		return;	
	}
	sprintf(text, "257 \"%s\" created.\r\n", nowsession->param);
	write_loop(connfd, text, strlen(text));
}

void do_rmd(SESSION *nowsession)
{
	char text[PATH_LEN];
	int res;

	if ( (res = rmdir(nowsession->param)) == -1 ) // 删除目录 成功返回零 失败则
	{
		sprintf(text, "500 Failed to delete directory [%s].\r\n", nowsession->param);
		write_loop(connfd, text, strlen(text));
		printf("删除目录失败！\n");
		perror("RMD");
		return;	
	}
	
	write_loop(connfd, "250 ok\r\n", strlen("250 ok\r\n"));
	printf("删除目录成功！\n");
}

void do_dele(SESSION *nowsession)
{			
	int res=remove(nowsession->param);
	if(res==-1)
		ERR_EXIT("REMOVE_ERR");
	write_loop(connfd,"250 Delete operation successful\r\n",strlen("250 Delete operation successful\r\n"));
}

char oldname[50];
void do_rnfr(SESSION *nowsession)
{
	
	char buf[1024];
	memset(buf,0,sizeof(buf));
	strcpy(oldname, nowsession->param);
	sprintf(buf,"350 Ready for RNTO.\r\n");
	write_loop(connfd,buf,strlen(buf));
}

void do_rnto(SESSION *nowsession)
{
	char buf[1024];
	memset(buf,0,sizeof(buf));
	rename(oldname, nowsession->param);
	sprintf(buf,"250 Rename successful.\r\n");
	write_loop(connfd,buf,strlen(buf));
}

void do_site(SESSION *nowsession)
{
	char buf[100],str[100],filename[100],temp[20],tempbuf[50];
	memset(buf,0,sizeof(buf));
	sprintf(str,"%s\r\n", nowsession->param);
	get_param(str, nowsession);
	strcpy(buf, nowsession->param);

	int mode;
	sscanf(buf,"%04o%s",&mode,filename);
	chmod(filename,(mode_t)mode);
	memset(buf,0,sizeof(buf));
	sprintf(buf,"200 SITE CHMOD command ok.\r\n");
	write_loop(connfd,buf,strlen(buf));
}

void do_stor(SESSION *nowsession)
{
	char text[PATH_LEN];
	data_conn_sock = test_pasv_or_port();// 建立数据连接通道
	strcpy( text, "150 Ok to send data.\r\n");
	write_loop(connfd,text, strlen(text));// 150

	speed_limit(LIMIT_SPEED);
	signal(SIGALRM, alrm_handl);

	// 此处读取客户端上传的文件
	int fd_src;
	char file_name[PATH_LEN]="" ;
	// 设置路径
	char *p;
	p = strchr(nowsession->param, '/');
	if ( p == NULL)
	{
		sprintf(file_name, "%s/%s", nowsession->dir, nowsession->param);
	}else{
		strcpy(file_name, nowsession->param);
	}

	if (rest_value != 0)
	{
		fd_src = open(file_name, O_WRONLY, 0666);
	}else{
		fd_src = open(file_name, O_CREAT | O_WRONLY, 0666);
	}

	if(fd_src == -1)
	{
		perror("open");
		exit(-1);
	}

	if (rest_value != 0)
	{
		lseek(fd_src, rest_value, SEEK_SET);
		printf("设置文件偏移~\n");
		rest_value = 0;
	}

	char buf[BUFSIZE]="";
	int n;

	while(read(data_conn_sock, buf, BUFSIZE) )
	{
		n = strlen(buf);
		write(fd_src, buf, n);
		TMP_UPLOAD_BYTES += n;
		transfer_data_now += n;
	}

	strcpy( text, "226 File receive OK.\r\n");
	write_loop(connfd, text, strlen(text)); // 226

	close_timer();
	
	if (strcmp(last_com, "PASV")==0)
	{		
		close(pasv_connsock);
		// printf("pasv_connsock 已关闭 \n");
		memset(last_com, 0, sizeof(last_com));
	}
	close(data_conn_sock);
}

void do_retr(SESSION *nowsession)
{
	// 下载文件
	char text[PATH_LEN];
	data_conn_sock=test_pasv_or_port();// 建立数据连接通道
	strcpy( text, "150 Opening BINARY mode data connection for stats.dat (119 bytes).\r\n");
	write_loop(connfd,text, strlen(text));// 150

	speed_limit(LIMIT_SPEED);
	signal(SIGALRM, alrm_handl);

	// 此处写客户端下载的文件
	int fd_src;
	char file_name[PATH_LEN]="" ;
	sprintf(file_name, "%s/%s", nowsession->dir, nowsession->param);
	// printf("下载文件路径： %s\n", file_name);
	fd_src = open(file_name, O_RDONLY);
	if(fd_src == -1)
	{
		perror("open");
		exit(-1);
	}

	if (rest_value != 0)
	{
		lseek(fd_src, rest_value, SEEK_SET);
		printf("设置续传文件偏移！\n");
		rest_value = 0;
	}

	char buf[BUFSIZE]="";
	int res=0, n;

	while( read(fd_src, buf, BUFSIZE) )
	{
		// printf("%s", buf);
		n = strlen(buf);
		write(data_conn_sock, buf, n);
		TMP_DOWNLOAD_BYTES += n;
		transfer_data_now += n;
	}

	if (res != 0)
	{		
		strcpy( text, "426 Failure writing network stream.\r\n");
		write_loop(connfd, text, strlen(text)); // 226
	}else{
		strcpy( text, "226 File send OK.\r\n");
		write_loop(connfd, text, strlen(text)); // 226
	}
	
	close_timer();

	if (strcmp(last_com, "PASV")==0)
	{		
		close(pasv_connsock);
		// printf("pasv_connsock 已关闭 \n");
		memset(last_com, 0, sizeof(last_com));
	}
	close(data_conn_sock);
}


static unsigned long get_file_size(const char *path)  
{  
    unsigned long filesize = -1;      
    struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
}  


void do_size(SESSION *nowsession)
{
	char text[PATH_LEN];
	unsigned long size=0;

	if(size = get_file_size(nowsession->param) == -1)
	{
		strcpy(text, "550 Could not get file size.\r\n");
		write_loop(connfd, text, strlen(text)); // 226
		perror("do_size");
		return;
	}

	sprintf(text, "213 %lu\r\n", size);
	write_loop(connfd, text, strlen(text)); // 226
}


void do_mdtm(SESSION *nowsession)
{
	char text[PATH_LEN];
	unsigned long size=0;

	if(size = get_file_size(nowsession->param) == -1)
	{
		strcpy(text, "550 Could not get file size.\r\n");
		write_loop(connfd, text, strlen(text)); // 226
		perror("do_size");
		return;
	}

	sprintf(text, "213 %lu\r\n", size);
	write_loop(connfd, text, strlen(text)); // 226
}


void statistics()
{
	// 上传下载统计
	union sigval value;	
	int flag=0;

	if ( TMP_UPLOAD_BYTES > 0 )
	{
		printf("当前上传 %d 字节\n", TMP_UPLOAD_BYTES);
		// 将其累加到总进程上
		Shm_Access( SHM_UP_BYTES, TMP_UPLOAD_BYTES);
		Shm_Access( SHM_UP_COUNT, 1);
		// 置空
		TMP_UPLOAD_BYTES = 0;
		flag = 1;
	}
	if ( TMP_DOWNLOAD_BYTES > 0 )
	{
		printf("当前下载 %d 字节\n", TMP_DOWNLOAD_BYTES);
		// 将其累加到总进程上
		Shm_Access( SHM_DOWN_BYTES, TMP_DOWNLOAD_BYTES)	;	
		Shm_Access( SHM_DOWN_COUNT, 1);
		// 置空
		TMP_DOWNLOAD_BYTES = 0;
		flag = 1;
	}

	if (flag == 1)
	{
		print_shm();
	}
}




