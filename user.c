#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <shadow.h>
#include <crypt.h>   //编译时要加  -lcrypt
#define ERR_EXIT(m) (perror(m),exit(EXIT_FAILURE))

#include "user.h"


//验证登录用户的密码是否正确，如果正确切换有效用户和有效家目录    0777

//linux下所有用户信息放在 /etc/passwd

int setegid_euid(char *name,int ischdir)
{
	//必须先改有效组，才能改有效用户 study  oracle
	struct	passwd *p=getpwnam(name);
	if(p==NULL)
	{	//ERR_EXIT("getpwnam");
		return -1;
	}

	int res=setegid(p->pw_gid);
	if(res==-1)
		//ERR_EXIT("setegid");
		return -1;
	res=seteuid(p->pw_uid);
	if(res==-1)
		//	ERR_EXIT("seteuid");
		return -1;

	//获到执行程序的用户id,有效有户id
	printf("uid=%d   euid=%d\n",(int)getuid(),(int)geteuid());
	//获到执行程序的组id,有效有户组id
	printf("gid=%d egid=%d\n",(int)getuid(),(int)geteuid());

	//切换有效用户家目录
	if(ischdir==0)
	{
		chdir(p->pw_dir);
	}
	return 1;

}

int checkpass(char *name,char *pass)
{
	struct	spwd *m=getspnam(name);
	if(m==NULL)
	{
		printf("mima error!\n");
		return -1;
	}
	//加密后的密码   明文密码123456
	//1. 取出加密密码的前12位做为种子
	char sald[13]="";
	strncpy(sald,m->sp_pwdp,12);

	//2.用取出的前12位和明文密码去做md5校验
	//crypt("123456",sald);

	//3.比较这两个密码，相匹配说明密码正确
	if(strcmp((char *)crypt(pass,sald),m->sp_pwdp)==0)
		return 1;
	else
		return -1;

}


