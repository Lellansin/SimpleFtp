
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "sem.h"

SHARE_MEMORY common_memory;

int comm_semid;
int nnnshmid;

#define ERR_EXIT(m) (perror(m),exit(EXIT_FAILURE))

union semun {
             int val;                  /* value for SETVAL */
             struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
             unsigned short *array;    /* array for GETALL, SETALL */
                                       /* Linux specific part: */
             struct seminfo *__buf;    /* buffer for IPC_INFO */
};

int sem_create(key_t key)
{
	int semid = semget(key,1,0666|IPC_CREAT);//1代表信号量集合里只有一个信号量
	if (semid == -1)
		ERR_EXIT("semget");

	return semid;
}

int sem_open(key_t key)
{
	int semid = semget(key,0,0);//0，0代表打开信号量
	if (semid == -1)
		ERR_EXIT("semget");

	return semid;
}

int sem_p(int semid)
{
	struct sembuf sb = {0,-1,SEM_UNDO};
	int ret = semop(semid,&sb,1);
	if (ret == -1)
		ERR_EXIT("semop");

	return ret;
}

int sem_v(int semid)
{
	struct sembuf sb = {0,1,SEM_UNDO};
	int ret = semop(semid,&sb,1);
	if (ret == -1)
		ERR_EXIT("semop");

	return ret;
}

int sem_d(int semid)
{
	int ret = semctl(semid,0,IPC_RMID,0);
	if (ret == -1)
		ERR_EXIT("semctl");
	return ret;
}

int sem_setval(int semid,int val)
{
	union semun sem;
	sem.val = val;
	int ret = semctl(semid,0,SETVAL,sem);
	if (ret == -1)
		ERR_EXIT("semctl");

	return ret;
}

int sem_getval(int semid)
{
	int ret = semctl(semid,0,GETVAL,0);//此时函数的返回值就是现在的信号量的值
	if (ret == -1)
		ERR_EXIT("semctl");

	return ret;
}

int Shm_Access(int way, int value)
{
	int c;

	comm_semid = sem_create((key_t)SEM_INI_KEY);
	sem_setval(comm_semid, 1);//设信号量的初值为1
	sem_p(comm_semid);	// p 操作
	int myshmid = shmget((key_t)SHM_INI_KEY,sizeof(SHARE_MEMORY),IPC_CREAT|0666);
	if(myshmid == -1)
			ERR_EXIT("shmid_myshmid");

	SHARE_MEMORY *myshm = (SHARE_MEMORY *)shmat(myshmid,NULL,0);
	if(myshm == (SHARE_MEMORY *)-1)
			ERR_EXIT("shmmyshm");

	if (way == SHM_UP_COUNT){
		myshm->up_count += value;
	}else if (way == SHM_DOWN_COUNT){
		myshm->down_count += value;
	}else if (way == SHM_UP_BYTES){
		myshm->up_bytes += value;
	}else if (way == SHM_DOWN_BYTES){
		myshm->down_bytes += value;
	}	

	int res = shmdt(myshm);
	if(res==-1)
		ERR_EXIT("shmdt_mymyshm");

	sem_v(comm_semid);	// v操作
    sem_d(comm_semid);
	return c;
}



int print_shm()
{
	int c;

	comm_semid = sem_create((key_t)SEM_INI_KEY);
	sem_setval(comm_semid, 1);
	sem_p(comm_semid);
	int myshmid = shmget((key_t)SHM_INI_KEY,sizeof(SHARE_MEMORY),IPC_CREAT|0666);
	if(myshmid == -1)
		ERR_EXIT("shmid_myshmid");
	SHARE_MEMORY *myshm = (SHARE_MEMORY *)shmat(myshmid,NULL,0);
	if(myshm == (SHARE_MEMORY *)-1)
		ERR_EXIT("shmmyshm");

	printf("\n-----------------------------------\n");
	printf("上传文件数 : %d\n", myshm->up_count);
	printf("下载文件数 : %d\n", myshm->down_count);
	printf("上传字节数 : %lu\n", myshm->up_bytes);
	printf("下载字节数 : %lu\n", myshm->down_bytes);
	printf("\n-----------------------------------\n");

	int res = shmdt(myshm);
	if(res==-1)
		ERR_EXIT("shmdt_mymyshm");

	sem_v(comm_semid);	// v操作
    sem_d(comm_semid);
	return c;
}





