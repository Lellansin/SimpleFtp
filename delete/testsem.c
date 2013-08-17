
#include "sem.h"
#include "error.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	int myshmid = shmget((key_t)1133,sizeof(SHARE_MEMORY),IPC_CREAT|0666);

	if(myshmid == -1)
			ERR_EXIT("shmid_myshmid");

	printf("shmid=%d\n", myshmid);

	SHARE_MEMORY *myshm = (SHARE_MEMORY *)shmat(myshmid,NULL,0);

	if(myshm == (SHARE_MEMORY *)-1)
			ERR_EXIT("shmmyshm");


	printf("up_count : %d\n", myshm->up_count);
	printf("down_count : %d\n", myshm->down_count);
	printf("up_bytes : %d\n", myshm->up_bytes);
	printf("down_bytes : %d\n", myshm->down_bytes);

	int res = shmdt(myshm);
	if(res==-1)
		ERR_EXIT("shmdt_mymyshm");
}
