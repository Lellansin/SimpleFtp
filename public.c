#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <shadow.h>

#include "public.h"

int login(char uid[],char pwd[])
{	
	//获取现在执行这个程序的用户id,组id  
	// printf("uid=%d gid=%d\n",(int)getuid(),(int)getgid());
	//执行程序的有效用户的id和组id
	// printf("euid=%d egid=%d\n",(int)geteuid(),(int)getegid());



	//写程序实现登录过程：所有用户登录时都是用root用户登录的
	/*然后用root用户的身份去对现在的登录用户做身份和密码的校验，
		如果正确，切换有效用户身份为登录用户的
		          切换工作目录为登录用户的家目录  oracle /home/oracle

				  getspnam 可以得到加密后的密码
	*/
 //明文密码
    int res;
	char filepath[20];
	struct spwd *p=	getspnam(uid);
	if((strcmp(uid,"anonymous")==0))
	{
		sprintf(filepath,"/ftp");
		/*res=chdir(filepath);
			if(res==-1)
				ERR_EXIT("chdir");
				return 1;*/
	}
	else if((strcmp(uid,"root")==0))
		sprintf(filepath,"/%s",uid);
	else
		sprintf(filepath,"/home/%s",uid);

	if(p != NULL)
	{
		//p->sp_pwdp 加密后的密码
		//把明文密码经过MD5加密后与加密的密码进行校验，如果匹配，说明密ok
		//加密后的密码的前12位当种子数与明文密码进行md5加密得到密文密码
		char s[13]="";
		strncpy(s,p->sp_pwdp,12);//得到密码的种子
		if(strcmp((char *)crypt(pwd,s),p->sp_pwdp)==0)
		{
			res = chdir(filepath);
			if(res==-1)
				ERR_EXIT("chdir");			
			// printf("pass is right!\n");
			return 1;
			
			
		}
		else
		{	
			printf("pass is error!\n");
			return 0;
		}

	}
}
	//记住要先设有效的组id才能设有效的用户id
	

		//获取现在执行这个程序的用户id,组id  
//	printf("uid=%d gid=%d\n",(int)getuid(),(int)getgid());
	//执行程序的有效用户的id和组id
//	printf("euid=%d egid=%d\n",(int)geteuid(),(int)getegid());

