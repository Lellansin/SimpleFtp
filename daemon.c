#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "daemon.h"
#include "error.h"

//与终端无关，在后台默默运行
void daemon_switch(int ischdir,int isclose)
{
	//要求调用setsid（）的不能是进程组组长，可是现在这个程序有可能是进程组组长
	pid_t pid = fork();
	if(pid<0)
		exit(-1);
	if(pid>0)
		exit(-1);
	//父进程退出，留下来的是子进程
	setsid();//创建一个新的会话期，从而脱离原有的会话期
	//此时子进程成为新会话期的领导，可能会通过fcntl去获到终端
	pid = fork();
	if(pid<0)
		exit(-1);
	if(pid>0)
		exit(-1);

	//此时留下来的是孙子进程,再也不能获取终端

	//守护进程应该工作在一个系统永远不会删除的目录下
	if(ischdir == 0)
	{
		chdir("/");
	}
	if(isclose == 0)
	{
		close(0);
		close(1);
		close(2);
	}

	//去掩码位
	umask((mode_t)0);//sys/stat.h
}

