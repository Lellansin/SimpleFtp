#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"

HASHNODE *hash_pid_ip[HASH_MAX]={NULL};		// 客户端进程号与对应的IP地址
HASHNODE *hash_ip_count[HASH_MAX]={NULL};	// 同一IP的连接数

//哈希函数：算下标
unsigned int hash_fun(char *key)//取下标值
{
	unsigned int index=0;
	while(*key)
	{
		index = *key +4*index;
		key++;
	}
	return index % HASH_MAX;
}

HASHNODE * hash_search(HASHNODE *hash_table[],char *key)
{
	unsigned int index;
	index = hash_fun(key);
	HASHNODE *p = NULL;
	p = hash_table[index];
	for(;p!=NULL;p=p->next)
	{
		if(strcmp(p->key,key)==0)
			return p;
	}
	return p;
}


int hash_insert(HASHNODE *hash_table[],char *key, char *val,int way,HASH_FUN fun1)
{
	HASHNODE * p=hash_search(hash_table,key);
	int index,tempi=0;
	char tempstr[10];
	if(p==NULL)//hash表中没有该节点
	{
		p=(HASHNODE *)malloc(sizeof(HASHNODE));
		if(p==NULL)
		{
			perror("malloc");
			return -1 ;
		}
		memset(p,0,sizeof(HASHNODE));
		p->key = (char *)strdup(key);
		if(p->key==NULL)
		{
			free(p);
			return -1;
		}
		if(way==0)//hash_conf,hash_pid_ip
		{
			p->val =(char *) strdup(val);//(char *)
			if(p->val==NULL)
			{
				free(p->key);
				free(p);
				return -1;
			}
		}
		else if(way==1)//hash_ip_count
		{
			char str[2];
			sprintf(str,"%d",1);
			p->val=(char *)strdup(str);
			if(p->val==NULL)
			{
				free(p->key);
				free(p);
				return -1;
			}
		}
		else if(way==2)//hash_fun
		{
			p->fun=fun1;//(HASH_FUN)val
		}

		p->next = NULL;
		p->prev = NULL;

		index=hash_fun(key);
		if(hash_table[index]==NULL)//头节点
			hash_table[index]=p;
		else
		{
			p->next = hash_table[index];
			//hash_table[index]->prev = p;
			hash_table[index] = p;
		}
	}
	else //p不为空，插入hash_ip_count时 同一IP的数量加1
	{
		tempi=atoi(p->val);
		tempi+=1;
		free(p->val);
		sprintf(tempstr,"%d",tempi);
		p->val=(char *)strdup(tempstr);
	}
	return 0;
}

//way==1用于ip连接数的删除
int hash_del(HASHNODE *hash_table[],char *key,int way)
{
	int tempi=0;
	char tempstr[20];
	//
	HASHNODE *p = hash_search(hash_table,key);
	if(p == NULL)
		return -1;
	if(way==1)//如果ip连接数大于1,只是把数量减1,不删除节点
	{
		tempi=atoi(p->val);
		tempi-=1;
		free(p->val);
		sprintf(tempstr,"%d",tempi);
		p->val=(char *)strdup(tempstr);

	}
	else
	{
		unsigned int index = hash_fun(key);
		if(p==hash_table[index]  )//如果要删除的p结点是该
		{
			if(p->next==NULL)//第一个并且只有一个节点
				hash_table[index]=NULL;
			else  //是第一个结点但其后面还有节点
			{
				//p->next->prev=p->prev;
				hash_table[index]=p->next;

			}
		}
		else //要删除的不是第一个结点
		{
			p->prev->next=p->next;//不是最后一个结点
			if(p->next==NULL)//删除的是最后一个结点
				p->prev->next=NULL;
		}
		free(p);
		p=NULL;
	}
	return 0;
}

void* hash_getval(HASHNODE *hash_table[],char *key,int way)
{
	HASHNODE* p = hash_search(hash_table,key);
	if(p == NULL)
	{
		return NULL;
	}
	if(way == 1)
	{
		return (void *)p->val;
	}
	else if(way == 2)
	{
		return (void *)p->fun;
	}
	return NULL;

}


void free_hash_node(HASHNODE *hash_test[])
{
	int i;
	HASHNODE *p,*t;
	for(i=0;i<HASH_MAX;i++)
	{
		if(hash_test[i]!=NULL)
		{
			p=hash_test[i];
			while(p)
			{
				t=p;
				p=p->next;
				free(t);
			}
			hash_test[i]=NULL;
		}
	}
}

int get_hash_length(HASHNODE *hash_table[])
{
	int i, count=0;
	for(i=0; i<HASH_MAX; i++)
	{
		if(hash_table[i]!=NULL)
		{
			count++;
		}
	}
	return count;
}

int getIpConn(char *ip)
{	
	char *p = (char *)hash_getval(hash_ip_count, ip, H_GET_VAL);
	if(p != NULL)
	{
		return atoi(p);
	}
	else
	{
		return 0;
	}
}



