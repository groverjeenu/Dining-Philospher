// Assignment 5
// Q2. RAIL-MANAGER

// Objective
// Assume, you have the above rail-crossing scenario. Trains may come from all the four different directions. You have to implement a system 
// where it never happens that more than one train crosses the junction (shaded region) at the same time. Every train coming to the junction,
// waits if there is already a train at the junction from its own direction. Each train also gives a precedence to the train coming from its
// right side (for example, right of North is West) and waits for it to pass. You also have to check for deadlock condition if there is any.
// Each train is a separate process. Your task will be to create a manager which creates those
// processes and controls the synchronisation among them.

// Group Details
// Group No: 22
// Member 1: Jeenu Grover (13CS30042)
// Member 2: Ashish Sharma (13CS30043)

// Filename: train.cpp

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <iostream>
#include <sys/stat.h>

using namespace std;

#define NO_OF_DIRECTIONS 4
#define NORTH 0
#define WEST 1
#define SOUTH 2
#define EAST 3



int DIRECTION_KEY,TRAIN_KEY,MUTEX,NO_OF_TRAINS,MY_DIRECTION,MY_ID;
char s[200];
int pid[100],graph[100][100];

int main(int argc, char * argv[])
{

	string direction_str,right_str;
	DIRECTION_KEY = atoi(argv[4]);
	TRAIN_KEY = atoi(argv[5]);
	MUTEX = atoi(argv[6]);
	//int p = atof(argv[1]);

	if(!strcmp(argv[1],"N")){
		MY_DIRECTION = NORTH;
		direction_str = "North";
		right_str = "West";
	}
	else if(!strcmp(argv[1],"W")){
		MY_DIRECTION = WEST;
		direction_str = "West";
		right_str = "South";
	}
	else if(!strcmp(argv[1],"S")){
		MY_DIRECTION = SOUTH;
		direction_str = "South";
		right_str = "East";
	}
	else if(!strcmp(argv[1],"E")){
		MY_DIRECTION = EAST;
		direction_str = "East";
		right_str = "North";
	}

	MY_ID = atoi(argv[2]);
	NO_OF_TRAINS = atoi(argv[3]);
	printf("My Id = %d\n",MY_ID);

	umask(000);

	// Get the Semaphores
	int dirid = semget(DIRECTION_KEY,4,IPC_CREAT|0666);
	if(dirid == -1 )printf("Semaphore for DIRECTIONS could not be created\n");

	int trainid = semget(TRAIN_KEY,1,IPC_CREAT|0666);
	if(trainid == -1 )printf("Semaphore for CROSSING could not be created\n");

	int mutexid = semget(MUTEX,1,IPC_CREAT|0666);
	if(mutexid == -1 )printf("Mutex Semaphore could not be created\n");

	struct sembuf Sop,mutex;
	FILE *fptr;
	int i,j,k;

	//Start
	mutex.sem_num = 0;
	mutex.sem_op = -1;
	mutex.sem_flg = 0;
	semop(mutexid,&mutex,1);



	fptr = fopen("matrix.txt","r");

	for(i=0;i<NO_OF_DIRECTIONS;i++)
		for(j = 0 ; j<NO_OF_TRAINS;j++)
		{
			fscanf(fptr,"%d",&graph[i][j]);
		}

	fclose(fptr);

	graph[MY_DIRECTION][MY_ID] = 1;

	fptr = fopen("matrix.txt","w");
	for(i=0;i<NO_OF_DIRECTIONS;i++)
	{
		for(j = 0 ; j<NO_OF_TRAINS ;j++)
		{
			fprintf(fptr,"%d ",graph[i][j]);
		}
		fprintf(fptr, "\n");
	}
	fclose(fptr);

	mutex.sem_num = 0;
	mutex.sem_op = 1;
	mutex.sem_flg = 0;
	semop(mutexid,&mutex,1);
	//End


	cout<<"Train "<<MY_ID<<": Requests for "<<direction_str<<"-Lock"<<endl;
	Sop.sem_num = MY_DIRECTION;
	Sop.sem_op = -1;
	Sop.sem_flg = 0;
	semop(dirid, &Sop, 1);

	cout<<"Train "<<MY_ID<<": Acquires "<<direction_str<<"-Lock"<<endl;

	//Start
	mutex.sem_num = 0;
	mutex.sem_op = -1;
	mutex.sem_flg = 0;
	semop(mutexid,&mutex,1);

	fptr = fopen("matrix.txt","r");

	for(i=0;i<NO_OF_DIRECTIONS;i++)
		for(j = 0 ; j<NO_OF_TRAINS;j++)
		{
			fscanf(fptr,"%d",&graph[i][j]);
		}

	fclose(fptr);

	graph[MY_DIRECTION][MY_ID] = 2;

	fptr = fopen("matrix.txt","w");
	for(i=0;i<NO_OF_DIRECTIONS;i++)
	{
		for(j = 0 ; j<NO_OF_TRAINS ;j++)
		{
			fprintf(fptr,"%d ",graph[i][j]);
		}
		fprintf(fptr, "\n");
	}
	fclose(fptr);

	mutex.sem_num = 0;
	mutex.sem_op = 1;
	mutex.sem_flg = 0;
	semop(mutexid,&mutex,1);
	//End

	//Start
	mutex.sem_num = 0;
	mutex.sem_op = -1;
	mutex.sem_flg = 0;
	semop(mutexid,&mutex,1);

	fptr = fopen("matrix.txt","r");

	for(i=0;i<NO_OF_DIRECTIONS;i++)
		for(j = 0 ; j<NO_OF_TRAINS;j++)
		{
			fscanf(fptr,"%d",&graph[i][j]);
		}

	fclose(fptr);

	graph[(MY_DIRECTION+1)%4][MY_ID] = 1;

	fptr = fopen("matrix.txt","w");
	for(i=0;i<NO_OF_DIRECTIONS;i++)
	{
		for(j = 0 ; j<NO_OF_TRAINS ;j++)
		{
			fprintf(fptr,"%d ",graph[i][j]);
		}
		fprintf(fptr, "\n");
	}
	fclose(fptr);

	mutex.sem_num = 0;
	mutex.sem_op = 1;
	mutex.sem_flg = 0;
	semop(mutexid,&mutex,1);
	//End

	cout<<"Train "<<MY_ID<<": Requests for "<<right_str<<"-Lock"<<endl;
	Sop.sem_num = (MY_DIRECTION+1)%4;
	Sop.sem_op = -1;
	Sop.sem_flg = 0;
	semop(dirid, &Sop, 1);

	cout<<"Train "<<MY_ID<<": Acquires "<<right_str<<"-Lock"<<endl;

	//Start
	mutex.sem_num = 0;
	mutex.sem_op = -1;
	mutex.sem_flg = 0;
	semop(mutexid,&mutex,1);

	fptr = fopen("matrix.txt","r");

	for(i=0;i<NO_OF_DIRECTIONS;i++)
		for(j = 0 ; j<NO_OF_TRAINS;j++)
		{
			fscanf(fptr,"%d",&graph[i][j]);
		}

	fclose(fptr);

	graph[(MY_DIRECTION+1)%4][MY_ID] = 2;

	fptr = fopen("matrix.txt","w");
	for(i=0;i<NO_OF_DIRECTIONS;i++)
	{
		for(j = 0 ; j<NO_OF_TRAINS ;j++)
		{
			fprintf(fptr,"%d ",graph[i][j]);
		}
		fprintf(fptr, "\n");
	}
	fclose(fptr);

	mutex.sem_num = 0;
	mutex.sem_op = 1;
	mutex.sem_flg = 0;
	semop(mutexid,&mutex,1);
	//End




	cout<<"Train "<<MY_ID<<": Requests Junction-Lock"<<endl;
	
	Sop.sem_num = 0;
	Sop.sem_op = -1;
	Sop.sem_flg = 0;
	semop(trainid, &Sop, 1);

	cout<<"Train "<<MY_ID<<": Acquires Junction-Lock; Passing Junction"<<endl;

	sleep(2);

	Sop.sem_num = 0;
	Sop.sem_op = 1;
	Sop.sem_flg = 0;
	semop(trainid,&Sop,1);

	cout<<"Train "<<MY_ID<<": Releases Junction-Lock"<<endl;


	Sop.sem_num = (MY_DIRECTION+1)%4;
	Sop.sem_op = 1;
	Sop.sem_flg = 0;
	semop(dirid,&Sop,1);

	cout<<"Train "<<MY_ID<<": Releases "<<right_str<<"-Lock"<<endl;

	//Start
	mutex.sem_num = 0;
	mutex.sem_op = -1;
	mutex.sem_flg = 0;
	semop(mutexid,&mutex,1);

	fptr = fopen("matrix.txt","r");

	for(i=0;i<NO_OF_DIRECTIONS;i++)
		for(j = 0 ; j<NO_OF_TRAINS;j++)
		{
			fscanf(fptr,"%d",&graph[i][j]);
		}

	fclose(fptr);

	graph[(MY_DIRECTION+1)%4][MY_ID] = 0;

	fptr = fopen("matrix.txt","w");
	for(i=0;i<NO_OF_DIRECTIONS;i++)
	{
		for(j = 0 ; j<NO_OF_TRAINS ;j++)
		{
			fprintf(fptr,"%d ",graph[i][j]);
		}
		fprintf(fptr, "\n");
	}
	fclose(fptr);

	mutex.sem_num = 0;
	mutex.sem_op = 1;
	mutex.sem_flg = 0;
	semop(mutexid,&mutex,1);
	//End

	Sop.sem_num = MY_DIRECTION;
	Sop.sem_op = 1;
	Sop.sem_flg = 0;
	semop(dirid,&Sop,1);

	cout<<"Train "<<MY_ID<<": Releases "<<direction_str<<"-Lock"<<endl;

	//Start
	mutex.sem_num = 0;
	mutex.sem_op = -1;
	mutex.sem_flg = 0;
	semop(mutexid,&mutex,1);

	fptr = fopen("matrix.txt","r");

	for(i=0;i<NO_OF_DIRECTIONS;i++)
		for(j = 0 ; j<NO_OF_TRAINS;j++)
		{
			fscanf(fptr,"%d",&graph[i][j]);
		}

	fclose(fptr);

	graph[MY_DIRECTION][MY_ID] = 0;

	fptr = fopen("matrix.txt","w");
	for(i=0;i<NO_OF_DIRECTIONS;i++)
	{
		for(j = 0 ; j<NO_OF_TRAINS ;j++)
		{
			fprintf(fptr,"%d ",graph[i][j]);
		}
		fprintf(fptr, "\n");
	}
	fclose(fptr);

	mutex.sem_num = 0;
	mutex.sem_op = 1;
	mutex.sem_flg = 0;
	semop(mutexid,&mutex,1);
	//End

	return 0;
}