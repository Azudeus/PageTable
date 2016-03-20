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
int NumberOfPages,NumberOfFrames,AvailableFrames;
int discAccessCounter=0;

int AlocateRandom()
{
	AvailableFrames--;
	return NumberOfFrames-AvailableFrames-1;
}

int AlocateVictim(page_table_pointer P)
{
	clock_t minimumTimeLastAccessed = clock();
	int victimIndex = -1;

//----Find Victim
	for (int i = 0; i < NumberOfPages; ++i)
	{
		if (P[i].Valid)
		{
			if (minimumTimeLastAccessed>P[i].timeLastAccessed)
			{
				minimumTimeLastAccessed=P[i].timeLastAccessed;
				victimIndex=i;
			}
		}
	}

	printf("Choose a victim page %d\n",victimIndex);

	if (P[victimIndex].Dirty)
	{
		printf("Victim is dirty, write out\n");
		discAccessCounter++;
		sleep(1);
	}
	int ret=P[victimIndex].Frame;
	P[victimIndex].Valid=0;
	P[victimIndex].Frame=-1;
	P[victimIndex].Dirty=0;
	P[victimIndex].Requested=0;
	return ret;
}

void request_handler(int pages, page_table_pointer P){
	bool found=false;
	int i=0;
	int newFrame = -1;
	int tmp = -1;
	for (int i = 0; i < pages; ++i)
	{
		if (P[i].Requested != 0)
		{
			printf("Process %d has requested page %d\n", P[i].Requested, i);
			if (AvailableFrames)
			{
				newFrame=AlocateRandom();
				printf("Put it in free frame %d\n",newFrame);
			}
			else
			{
				newFrame=AlocateVictim(P);
				printf("Put in victim's frame %d\n",newFrame);
			}

			tmp = P[i].Requested;
			P[i].Valid = 1;
			P[i].Frame = newFrame;
			P[i].Dirty = 0;
			P[i].Requested = 0;
			break;
		}
	}

	if (tmp != -1)
	{
		discAccessCounter++;
		sleep(1);
		printf("Unblock MMU\n");
		kill(tmp,SIGCONT); //send SIGCONT to MMU
	}
	else
	{
  // 	if (shmdt(PageTable) == -1) {
  //       	perror("ERROR: Error detaching segment");
  //       	exit(EXIT_FAILURE);
  //   	}
  //   	if (shmctl(SegmentId, IPC_RMID, NULL) == -1) {
  //   		perror("ERROR: Error removing segment");
  //       	exit(EXIT_FAILURE);
  //   	}
    	printf("%d\n",discAccessCounter);
		exit(EXIT_SUCCESS);
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
	int size;
	page_table_pointer PageTable;


	NumberOfFrames = atoi(argv[argc-1]);
	NumberOfPages  = atoi(argv[1]);
	AvailableFrames = NumberOfFrames;

	if((SegmentId = shmget(key,NumberOfPages*sizeof(page_table_entry),IPC_CREAT | 0660)) == -1){
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
