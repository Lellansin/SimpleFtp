#ifndef _SEM_SHM_H_
#define _SEM_SHM_H_

#define SHM_UP_COUNT 1
#define SHM_DOWN_COUNT 2
#define SHM_UP_BYTES 3
#define SHM_DOWN_BYTES 4
#define SEM_INI_KEY 1234
#define SHM_INI_KEY 1133

typedef struct my_shm
{
	int up_count;
	int down_count;
	unsigned long up_bytes;
	unsigned long down_bytes;
} SHARE_MEMORY;

extern SHARE_MEMORY common_memory;

extern int nnnsemid;
extern int nnnshmid;


int Shm_Access(int way, int value);
int print_shm();

#endif
