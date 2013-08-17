
#ifndef _HASH_H_
#define _HASH_H_

#define H_PID_IP 0
#define H_IP_COUNT 1 
#define H_FUN 2
#define H_GET_VAL 1
#define H_GET_FUN 2

#define H_DEL_MINUS 1
#define H_DEL_DESTROY 0

#define HASH_MAX 101

typedef struct	_sess
{
	char *com;		//客户端传过来的命令
	char *charm;	//客户端传过来的命令后跟的参数
	char *dir;		//当前操作的目录 如果需要，再添加即可
}CONNSESSION;

typedef void (*HASH_FUN)(CONNSESSION *);

typedef struct _ftp_com
{
	char *key;
	HASH_FUN fun;
}FTP_COM;

typedef struct _hash
{
	char *key;
	char *val;
	HASH_FUN fun;
	struct _hash *next;
	struct _hash *prev;
}HASHNODE;

int hash_insert(HASHNODE *hash_table[],char *key, char *val,int way,HASH_FUN fun1);
int hash_del(HASHNODE *hash_table[],char *key,int way);

#endif

