#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "io.h"
#include "list.h"
#include "user.h"
#include "com.h"
#include "config.h"
#include "net.h"
#include "hash.h"
#include "sem.h"
#include "error.h"

void handle_recover(int sig);
int serAddrReuse(int listenfd);
int start();
int init();

int MAX_COUNT;			// 最大连接数
int MAX_IP_COUNT;		// 同ip最大连接数
int PORT_LISTEN;		// 命令监听端口
char LOCAL_IP[20];		// 本机ip 从配置文件读取
int LIMIT_SPEED; 		// 限速

extern HASHNODE *hash_pid_ip[HASH_MAX];		//客户端进程号与对应的IP地址
extern HASHNODE *hash_ip_count[HASH_MAX];	//同一IP的连接数

pid_t MAIN_PID;		// 总进程的id

int connfd;
char DirNOW[50];

// 数据传输命令数组
char DATA_COM[][10] = {
	"PASV",
	"LIST",
	"STOR",
	"RETR",
};

/* 客户端退出处理 */
void handle_recover(int sig)
{
	int i;
	if (sig == SIGCHLD)
	{
		pid_t pid = wait(0);
		printf("退出的子进程 pid = [%d]\n\n", pid);

		char tempIP[20]="",tempPid[10]="";
		sprintf(tempPid, "%d", pid);
		// 通过进程pid获取ip
		strcpy(tempIP, (char *)hash_getval(hash_pid_ip, tempPid, H_GET_VAL));
		if(tempIP != NULL)
		{
			printf("开始重新统计链接数：\n");
			printf("退出客户端IP=%s\n", tempIP);
			//得到指定IP的已连接数
			i = atoi((char *)hash_getval(hash_ip_count, tempIP, H_GET_VAL));
			if ( i > 1 )
			{
				// 连接数至少是2 则减1
				hash_del(hash_ip_count, tempIP, H_DEL_MINUS);//删除 1代表数量大于1
			}else
			{
				// 链接数为1 删除节点
				hash_del(hash_ip_count, tempIP, H_DEL_DESTROY);
			}
			printf("哈希表 IP连接数 【删除】 %s\n", tempIP);
			hash_del(hash_pid_ip, tempPid, H_DEL_DESTROY);
			printf("哈希表 进程IP联系 【删除】 %s\n", tempPid);
		}else
		{
			printf("哈希表 连接数统计失败！\n");
		}

		printf("\n当前客户端列表\n");
		for(i=0;i<HASH_MAX;i++)
		{
			if(hash_ip_count[i]!=NULL)
			{
				printf("客户端ip=%-15s  连接数=%-20s\n",hash_ip_count[i]->key,hash_ip_count[i]->val);
			}
		}
		printf("总计 %d 个链接\n", get_hash_length(hash_pid_ip) );
	}
}

char * itoa(char *number, int n)
{
	sprintf(number, "%d", n);
	return number;
}

/* ftp运行函数 */
int start()
{
	//1.socket
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd==-1) ERR_EXIT("socket");

	//2.设置监听 ip/port
	struct sockaddr_in servaddr;
	memset(&servaddr, 0,sizeof(servaddr));

	servaddr.sin_family = PF_INET;//AF_INET
	servaddr.sin_port = htons(PORT_LISTEN);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t seraddrlen =sizeof(servaddr);

	//设置地址重用
	int rel = serAddrReuse(listenfd);
	if (rel == -1) ERR_EXIT("setsockopt");

	int res = bind(listenfd, (struct sockaddr*)&servaddr, seraddrlen);
	if(res==-1) ERR_EXIT("bind");

	//3.listen侦听客户端的连接
	res=listen(listenfd, 5);
	if(res==-1) ERR_EXIT("listen");

	printf("-------------------------------------\n");
	printf("|       服务器端配置文件读取成功\n");
	printf("-------------------------------------\n");
	printf("|   服务器端ip	  =[%s]\n", LOCAL_IP);
	printf("|   监听端口port  =[%d]\n", PORT_LISTEN);
	printf("|   最大连接数	  =[%d]\n", MAX_COUNT);
	printf("|   同ip最大连接数=[%d]\n", MAX_IP_COUNT);
	printf("|   限制下载速度  =[%dkb/s]\n", LIMIT_SPEED*10);
	printf("-------------------------------------\n");

	//4.accept:接受客户端的连接，如果没有客户端连接进来, accept会阻塞，如果客户端连接进来了，accept会返回一个新的套接口，这个新的套接口就是与客户端进行数据通信的通道, 客户端的网络地址会填充进accept的第二个参数，第三个参数会是客户端地址的长度
	pid_t pid;
	MAIN_PID = getpid();

	// 安装进程回收信号
	signal(SIGCHLD, handle_recover);

	while(1)
	{
		// 阻塞，等待接收
		struct sockaddr_in cliaddr;
		memset(&cliaddr, 0,sizeof(cliaddr));
		socklen_t cliaddrlen =sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
		if(connfd==-1) ERR_EXIT("accept");

		char now_ip[20] = "";
		strcpy(now_ip, inet_ntoa(cliaddr.sin_addr));

		printf("\n建立新连接！\n");
		printf("ip=%s\n", now_ip);
		printf("port=%d\n\n", ntohs(cliaddr.sin_port));

		// 发送welcome至客户端 getpeername(connfd, )
		write_loop(connfd, "220 welcome\r\n", strlen("220 welcome\r\n"));

		int conn_count=0, len=0;
		// 判断连接数控制
		if ( (conn_count = getIpConn(now_ip)) >= MAX_IP_COUNT )
		{
			printf("超过同ip连接数，当前为 %d 最大为 %d\n", conn_count, MAX_IP_COUNT);
			// 提示超过连接数
			write_loop(connfd, "421 Connection refused: too many sessions for this address.\r\n", strlen("421 Connection refused: too many sessions for this address.\r\n"));
			continue;
		}else if ( (len = get_hash_length(hash_pid_ip)) >= MAX_COUNT )
		{
			printf("超过总连接数，当前 %d 个链接\n",  len);
			write_loop(connfd, "421 Connection refused: too many sessions\r\n", strlen("421 Connection refused: too many sessions\r\n"));
		}

		printf("---------------------------------\n");
		printf("| 链接检测                     |\n");
		printf("|       目前同ip连接数 %2d      |\n", conn_count);
		printf("|       总计 %2d 个链接         |\n", len);
		printf("---------------------------------\n");

		pid = fork();
		switch(pid)
		{
		case -1:
			exit(-1);
		case 0://子进程
			close(listenfd);
			client_session();
			exit(EXIT_SUCCESS);
			break;
		default:
			close(connfd);
			printf("\n新客户端接入： pid=[%d] id=[%s]\n", pid, now_ip);

			char str_pid[10]="";
			itoa(str_pid, pid);
			hash_insert(hash_pid_ip, str_pid, now_ip, H_PID_IP, NULL);
			hash_insert(hash_ip_count, now_ip, NULL, H_IP_COUNT, NULL);

			int i;
			printf("-------------------------------------------------------\n");
			for(i=0;i<HASH_MAX;i++)
			{
				if(hash_ip_count[i]!=NULL)
				{
			printf("|       客户端ip=%-20s 连接数=%-30s  |\n",hash_ip_count[i]->key,hash_ip_count[i]->val);
				}
			}
			printf("|       总计 %2d 个链接                                \n", get_hash_length(hash_pid_ip));
			printf("-------------------------------------------------------\n");

			break;
		}
	}
}

/* 检查命令是否为大量数据传输命令 */
int check_data_com(char *com)
{
	int i, len;
	len = sizeof(DATA_COM) / sizeof(DATA_COM[0]);
	for (i = 0; i < len; ++i)
	{
		if ( strcmp(com, DATA_COM[i]) == 0 )
		{
			return 1;
		}
	}
	return 0;
}

/* 客户端会话 */
int client_session()
{
	char recvbuf[1024];
	int connsocketpair[2];
	SESSION nowsession;
	int res = socketpair(PF_UNIX, SOCK_STREAM, 0, connsocketpair);
	if( res == -1 ) ERR_EXIT("socketpair");

	res = chdir("/root/");
	if(res == -1) ERR_EXIT("chdir");

	switch(fork())
	{
		case -1:
			ERR_EXIT("socketpair");
			break;
		/* 数据传输端 (孙进程) */
		case 0:
		{
			close(connsocketpair[0]);
			
			while(1)
			{
				memset(recvbuf, 0, sizeof(recvbuf));
				// 接受控制端的数据传输命令
				read_line(connsocketpair[1], recvbuf, sizeof(recvbuf));
				
				if (strlen(recvbuf) > 0)
				{
					memset( &nowsession, 0, sizeof(nowsession) );
					get_com(recvbuf, &nowsession);
					get_param(recvbuf, &nowsession);
					get_dir(recvbuf, &nowsession);

					auto_switch(&nowsession);
				}
			}
			break;
		}
		/* 命令控制端 (子进程) */
		default :
		{
			close(connsocketpair[1]);			

			char buf[200], tmpDir[50];
			getcwd(buf, sizeof(buf)-1);
			printf("当前目录为 %s\n", buf);			

			while(1)
			{
				memset(recvbuf, 0, sizeof(recvbuf));
				// 接受命令
				int n = read_line(connfd, recvbuf,sizeof(recvbuf));
				if(n<=0) break;

				memset( &nowsession, 0, sizeof(nowsession) );
				get_com(recvbuf, &nowsession);
				get_param(recvbuf, &nowsession);
				get_dir(NULL, &nowsession);

				if (check_data_com(nowsession.com))
				{
					strcpy(tmpDir, nowsession.dir);
					memset(buf, 0, sizeof(buf));
					sprintf(buf, "%s %s\r%s\r\n", nowsession.com, nowsession.param, nowsession.dir);
					write_loop(connsocketpair[0], buf, strlen(buf));
				}else{
					auto_switch(&nowsession);
				}				
			}
			break;
		}		
	}
}


// 程序初始化
int init()
{
	int status;
	pid_t pid;

	// 读取配置文件
	config();

	// 创建控制子进程
	pid = fork();

	switch(pid)
	{
		case -1:
			exit(EXIT_SUCCESS);
			break;
		case 0: // 子进程
			start();
			break;
		default:
			printf("\n监听线程已开启 pid=%d\n", pid);
			status = 1;
			break;
	}

	return status;
}

int main(int argc,  char *argv[])
{
	// 切换守护进程
	// daemon_switch();

	// 程序初始化
	init();

	// 主进程休眠
	while(1)
	{
		// console 控制台
		sleep(10);
	}

	exit(EXIT_SUCCESS);
}


