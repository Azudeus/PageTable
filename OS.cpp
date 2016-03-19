#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "PageTable.h"

bool received=false;
int NumberOfFrames;

void AlocateRandom(int value)
{


}

void AlocateVictim(int value)
{

}

void request_handler(int pages, page_table_pointer P){
	bool found=false;
	int i=0;
	int tmp = -1;
	for (int i = 0; i < pages; ++i)
	{
		if (P[i].Requested != 0)
		{
			P[i].Valid = 1;
			if (NumberOfFrames)
			{
				AlocateRandom(i);
				NumberOfFrames--;
			}
			else
			{
				AlocateVictim(i);
			}
			printf("%d\n", P[i].Requested);
			
			tmp = P[i].Requested;

			P[i].Requested = 0;
			break;
		}
	}

	if (tmp != -1)
	{
		sleep(1);
		kill(tmp,SIGCONT);
	}
	else
	{
		exit(0);
	}
}

void sig_handler(int signo){
	if (signo == SIGUSR1){
		printf("received SIGUSR1\n");
		received = true;

	}
}

int main(int argc, char *argv[]){
	key_t key=getpid();
	int SegmentId;
	int NumberOfPages;
	int size;
	page_table_pointer PageTable;


	NumberOfFrames = atoi(argv[argc-1]);
	NumberOfPages=atoi(argv[1]);

	if((SegmentId = shmget(key,NumberOfPages*sizeof(page_table_entry),IPC_CREAT | 0)) == -1){
		perror("shmget");
		exit(1);
	}
	if ((PageTable=(page_table_pointer)shmat(SegmentId, NULL, 0)) == NULL){
		perror("shmat");
		exit(1);
	}

	printf ("Shared Memory Key : %d\n",key);
	for (int i=0;i<NumberOfPages;i++){
		// PageTable++;
		// printf("%d\n", PageTable);
		PageTable[i].Valid=0;
		PageTable[i].Frame=-1;
		PageTable[i].Dirty=0;
		PageTable[i].Requested=0;
	}

	while (true) {
		while(!received){
			if (signal(SIGUSR1,sig_handler) == SIG_ERR){
				printf("can't catch SIGUSR1\n");
			}
		}
		received = 0;
		request_handler(NumberOfPages,PageTable);
	}
	


}
