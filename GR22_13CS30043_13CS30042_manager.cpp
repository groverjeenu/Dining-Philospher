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

// Filename: manager.c

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
#include <list>
#include <vector>
#include <sys/stat.h>
#include <sstream>

using namespace std;

// #define KEY 1236
// #define Q0 332
// #define Q1 443 
// #define MUTEX 2237


#define NO_OF_DIRECTIONS 4
#define NORTH 0
#define WEST 1
#define SOUTH 2
#define EAST 3



int DIRECTION_KEY,TRAIN_KEY,MUTEX,NO_OF_TRAINS;
char s[200];
int pid[100],graph[100][100];


int V ;
int Graph[100][100] ;
vector<int> nodes;


//Standard Code taken from GEEKS FOR GEEKS

bool isCyclicUtil(int v, bool visited[], bool *recStack)
{
	nodes.push_back(v);
    if(visited[v] == false)
    {
        // Mark the current node as visited and part of recursion stack
        visited[v] = true;
        recStack[v] = true;
 
        // Recur for all the vertices adjacent to this vertex
        int i;
        for(i = 0; i < V; ++i)
        {
        	if(Graph[v][i] !=0)
        	{
	            if ( !visited[i] && isCyclicUtil(i, visited, recStack) )
	                return true;
	            else if (recStack[i])
	            {
	            	nodes.push_back(i);
	                return true;
	            }
        	}
        }
 
    }
    recStack[v] = false;  // remove the vertex from recursion stack
    nodes.pop_back();
    return false;
}
 
// Returns true if the graph contains a cycle, else false.
bool isCyclic()
{
    // Mark all the vertices as not visited and not part of recursion
    // stack
    bool *visited = new bool[V];
    bool *recStack = new bool[V];
    nodes.clear();
    for(int i = 0; i < V; i++)
    {
        visited[i] = false;
        recStack[i] = false;
    }
 
    // Call the recursive helper function to detect cycle in different
    // DFS trees
    for(int i = 0; i < V; i++)
        if (isCyclicUtil(i, visited, recStack))
            return true;
 
    return false;
}


int main(int argc, char * argv[])
{

	if(argc<5)
	{
		printf("USAGE: %s p DIRECTION_KEY TRAIN_KEY MUTEX\n",argv[0]);
		printf("Try Again\n");
		exit(1);
	}
	DIRECTION_KEY = atoi(argv[2]);
	TRAIN_KEY = atoi(argv[3]);
	MUTEX = atoi(argv[4]);
	int p = atof(argv[1]);
	umask(000);

	// Create the Semaphores
	int dirid = semget(DIRECTION_KEY,4,IPC_CREAT|0666);
	if(dirid == -1 )printf("Semaphore for DIRECTIONS could not be created\n");
	int trainid = semget(TRAIN_KEY,1,IPC_CREAT|0666);
	if(trainid == -1 )printf("Semaphore for CROSSING could not be created\n");

	int mutexid = semget(MUTEX,1,IPC_CREAT|0666);
	if(mutexid == -1 )printf("Mutex Semaphore could not be created\n");

	// Set the Values
	if(semctl(dirid,NORTH,SETVAL,1) == -1)printf("Value of NORTH subsemphore could not be set\n");
	if(semctl(dirid,WEST,SETVAL,1) == -1)printf("Value of WEST subsemphore could not be set\n");
	if(semctl(dirid,SOUTH,SETVAL,1) == -1)printf("Value of SOUTH subsemphore could not be set\n");
	if(semctl(dirid,EAST,SETVAL,1) == -1)printf("Value of EAST subsemphore could not be set\n");

	if(semctl(trainid,0,SETVAL,1) == -1)printf("Value of Crossing subsemphore could not be set\n");

	if(semctl(mutexid,0,SETVAL,1) == -1)printf("Value of mutex subsemphore could not be set\n");

	scanf("%s",s);
	int NO_OF_TRAINS = strlen(s);
	printf("%d\n",NO_OF_TRAINS);

	V = NO_OF_TRAINS + NO_OF_DIRECTIONS;

	int i,j,k,val,flg,tt,ex;
	stringstream ss,ps;

    struct sembuf mutex;

	FILE * fptr;

	// Initializing the Files
	//Start
		mutex.sem_num = 0;
		mutex.sem_op = -1;
		mutex.sem_flg = 0;
		semop(mutexid,&mutex,1);

		
		close(open("matrix.txt", O_RDWR|O_CREAT, 0666));
		fptr = fopen("matrix.txt","w");
		for(i=0;i<NO_OF_DIRECTIONS;i++)
		{
			for(j = 0 ; j< NO_OF_TRAINS;j++)
			{
				fprintf(fptr,"%d ",0);
			}
			fprintf(fptr, "\n");
		}
		fclose(fptr);
		//sleep(100000);

		mutex.sem_num = 0;
		mutex.sem_op = 1;
		mutex.sem_flg = 0;
		semop(mutexid,&mutex,1);
		//End

		tt = 0;
		int rnd, low , high;

		low = (1000  - (int)p*1000)/2;
		high = 1000 - low;


		while(1)
		{
			// If all the Trains have not been Created
			if(tt != NO_OF_TRAINS)
			{
				srand(time(NULL));
				rnd = rand()%1000;
				if(rnd >= low && rnd <= high)
				{
					// Check Deadlock
					//Start
					mutex.sem_num = 0;
					mutex.sem_op = -1;
					mutex.sem_flg = 0;
					semop(mutexid,&mutex,1);

					fptr = fopen("matrix.txt","r");

					for(i=0;i<NO_OF_DIRECTIONS;i++)
						for(j = 0 ; j<NO_OF_TRAINS ;j++)
						{
							fscanf(fptr,"%d",&graph[i][j]);
						}

					fclose(fptr);

					for( i = 0 ;i < NO_OF_TRAINS + NO_OF_DIRECTIONS ; i++)
						for ( j=0 ; j < NO_OF_TRAINS + NO_OF_DIRECTIONS;j++)
							Graph[i][j] = 0;

					for( i= 0;i<NO_OF_DIRECTIONS;i++)
						for(j = 0; j< NO_OF_TRAINS; j++)
						{

							if(graph[i][j] == 1)
								Graph[j][NO_OF_TRAINS + i] = 1;
							else if (graph[i][j] == 2)
								Graph[NO_OF_TRAINS + i][j] = 1;
						}

					if(isCyclic())
					{
						printf("Cycle Detected\n");
						val = *(nodes.end() -1) ;
						flg = 0;

						for( i = 0 ;i < nodes.size()-1;i++)
							{
								if(nodes[i] == val)
								{
									flg = 1;
								}
								if(flg)
								{
									if(nodes[i] < NO_OF_TRAINS)
										printf("%d --> ",nodes[i]);
									else 
									{
										ex = nodes[i]-NO_OF_TRAINS;
										if(ex == 0)
											printf("NORTH --> ");
										else if(ex == 1)
											printf("WEST --> ");
										else if(ex == 2)
											printf("SOUTH --> ");
										else if(ex == 3)
											printf("EAST --> ");

									}

								}
									
							}

							if(val < NO_OF_TRAINS)
								printf("%d\n",val);
							else 
							{
								ex = val-NO_OF_TRAINS;
								if(ex == 0)
									printf("NORTH\n");
								else if(ex == 1)
									printf("WEST\n");
								else if(ex == 2)
									printf("SOUTH\n");
								else if(ex == 3)
									printf("EAST\n");

							}

							printf("\n\n");

							return 0;
					}

					

					

					mutex.sem_num = 0;
					mutex.sem_op = 1;
					mutex.sem_flg = 0;
					semop(mutexid,&mutex,1);
					//End

					//sleep(2);

				}

				else
				{
					// Create Next Train Process
					ss.str(std::string());
			    	ss<<tt;
			    	ps.str(std::string());
			    	ps<<NO_OF_TRAINS;
			    	char str[2] = "\0"; /* gives {\0, \0} */
					str[0] = s[tt];
			    	pid[tt] = fork();
			    	
			    	if( pid[tt] == 0)
			    	{
				    	execlp("xterm" ,"xterm" ,"-hold","-e","./train",str,ss.str().c_str(),ps.str().c_str(),argv[2],argv[3],argv[4],(const char *)NULL);
				    	
						exit(1);
					}
					tt++;

				}
			}
			else 
			{
				// All Trains Created--Just check for deadlock

				//Start
				mutex.sem_num = 0;
				mutex.sem_op = -1;
				mutex.sem_flg = 0;
				semop(mutexid,&mutex,1);

				fptr = fopen("matrix.txt","r");

				for(i=0;i<NO_OF_DIRECTIONS;i++)
					for(j = 0 ; j<NO_OF_TRAINS ;j++)
					{
						fscanf(fptr,"%d",&graph[i][j]);
					}

				fclose(fptr);

				for( i = 0 ;i < NO_OF_TRAINS + NO_OF_DIRECTIONS ; i++)
					for ( j=0 ; j < NO_OF_TRAINS + NO_OF_DIRECTIONS;j++)
						Graph[i][j] = 0;

				for( i= 0;i<NO_OF_DIRECTIONS;i++)
					for(j = 0; j< NO_OF_TRAINS; j++)
					{

						if(graph[i][j] == 1)
							Graph[j][NO_OF_TRAINS + i] = 1;
						else if (graph[i][j] == 2)
							Graph[NO_OF_TRAINS + i][j] = 1;
					}

				if(isCyclic())
				{
					printf("Cycle Detected\n");
					val = *(nodes.end() -1) ;
					flg = 0;

					for( i = 0 ;i < nodes.size()-1;i++)
						{
							if(nodes[i] == val)
							{
								flg = 1;
							}
							if(flg)
							{
								if(nodes[i] < NO_OF_TRAINS)
									printf("%d --> ",nodes[i]);
								else 
								{
									ex = nodes[i]-NO_OF_TRAINS;
									if(ex == 0)
										printf("NORTH --> ");
									else if(ex == 1)
										printf("WEST --> ");
									else if(ex == 2)
										printf("SOUTH --> ");
									else if(ex == 3)
										printf("EAST --> ");

								}

							}
								
						}

						if(val < NO_OF_TRAINS)
							printf("%d\n",val);
						else 
						{
							ex = val-NO_OF_TRAINS;
							if(ex == 0)
								printf("NORTH\n");
							else if(ex == 1)
								printf("WEST\n");
							else if(ex == 2)
								printf("SOUTH\n");
							else if(ex == 3)
								printf("EAST\n");

						}

						printf("\n\n");

						return 0;
					
				}

				

				

				mutex.sem_num = 0;
				mutex.sem_op = 1;
				mutex.sem_flg = 0;
				semop(mutexid,&mutex,1);
				//End

				sleep(1);
			}
		}






	return 0;
}