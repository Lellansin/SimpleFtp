#include "sem.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

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
	int semid = semget(key,1,0666|IPC_CREAT);
	if (semid == -1)
		ERR_EXIT("semget");

	return semid;
}

int sem_open(key_t key)
{
	int semid = semget(key,0,0);
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
	int ret = semctl(semid,0,GETVAL,0);
	if (ret == -1)
		ERR_EXIT("semctl");

	return ret;
}

int main(void)
{
	int pause_time;
	char op_char = 'O';

	srand((int)getpid());//做随机数的种子

	int semid = sem_open((key_t)1234);
	sem_setval(semid,1); //设信号量的初值为1
	/*int i;
	for (i=0;i<10;i++)
	{
		sem_p(semid);

		printf("%c",op_char);
		fflush(stdout);
		pause_time = rand() % 3;//0 1 2
		sleep(pause_time);
		printf("%c",op_char);
		fflush(stdout);
		
		sem_v(semid);

		pause_time = rand() % 2;// 0 1
		sleep(pause_time);
		printf("----");
	}*/

	sem_p(semid);

	int myshmid = shmget((key_t)1133,sizeof(SHARE_MEMORY),0);
	if(myshmid == -1)
			ERR_EXIT("shmid_myshmid");
	SHARE_MEMORY *mymyshm = (SHARE_MEMORY *)shmat(myshmid,NULL,0);
	if(mymyshm == (SHARE_MEMORY *)-1)
			ERR_EXIT("shmmymyshm");

	printf("%d\n", mymyshm->up_count);
	
	int res=shmdt(mymyshm);
		if(res==-1)
			ERR_EXIT("shmdt_mymyshm");
	
	sem_v(semid);

	printf("\n%d - finished\n",(int)getpid());
	return 0;
}	
