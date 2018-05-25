#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>

#include "functions.h"

#define PROCESS_SEMAPHORE "/semProcess"
#define MAIN_SEMAPHORE "/semMain"

#define TOTAL_MEMORY_PROCESS 2000

extern key_t shared_memory_key;
extern key_t request_memory_key; 
extern key_t processes_key;
extern int *request_size;
extern int * request_shared_memory;

sem_t * process_semaphore;
sem_t * main_semaphore;  

/****************************************************
*Function that print the shared memory. (Point 1)	*
*****************************************************/
void spy_memory()
{

	/* Attached shared memory */
	int shmid = get_id_shared_memory(request_memory_key, sizeof(int));
	request_size = shmat(shmid, (void *)0, 0);

	int shmI = get_id_shared_memory(shared_memory_key, request_size[0]*sizeof(float)); 
	request_shared_memory = shmat(shmI, (void *)0, 0);
	
	/* Initial configure of sempahore*/
	main_semaphore = sem_open(MAIN_SEMAPHORE, 0); 
	int value;
	sem_getvalue(main_semaphore, &value);
 
 	/*
 	See if semaphore is lock
	Reference: http://man7.org/linux/man-pages/man3/sem_getvalue.3.html
 	*/
    while(value == 0)
    {
    	sem_getvalue(main_semaphore, &value);
    	sleep(1); 
    }

	sem_wait(main_semaphore); 
	print_shared_memory(); 
	sem_post(main_semaphore); 

}

/****************************************************
*Function that print the state of each process   	*
*(point 2).											*
*****************************************************/
void spy_processes()
{

	/* Request shared memory of processes */
    int *process_shm; 
    int size_process = (int) sizeof(int) * TOTAL_MEMORY_PROCESS;

	process_shm = assign_shared_memory(processes_key, size_process);
  
    /*Initial configure semaphore*/
    process_semaphore = sem_open(PROCESS_SEMAPHORE, 0);
	int value;
	sem_getvalue(process_semaphore, &value);

 	/*
 	See if semaphore is lock
	Reference: http://man7.org/linux/man-pages/man3/sem_getvalue.3.html
 	*/
    while(value == 0)
    {
    	sem_getvalue(process_semaphore, &value);
    	sleep(1); 
    }

    sem_wait(process_semaphore);

    /* Print states */
    for(int i = 8; i < TOTAL_MEMORY_PROCESS; i += 8)
    {
    	if (process_shm[i] != 0)
    	{
    		switch(process_shm[i + 4])
    		{
	            case 0:
	                printf("PID: %i, State: %s\n", process_shm[i], "ACTIVE");
	                break;
	            case 1:
	                printf("PID: %i, State: %s\n", process_shm[i], "FINISH");
	                break;
	            case 2:
	                printf("PID: %i, State: %s\n", process_shm[i], "DEAD");
	                break;
	            case 3:
	                printf("PID: %i, State: %s\n", process_shm[i], "SEARCH");
	                break;
	            default:
	                break;
	        }
    	}
     }
     sem_post(process_semaphore); 
}

    

int main(int argc, char *argv[])
{

	shared_memory_key = 1234;
    request_memory_key = 1235;
    processes_key = 1236;


	if(argc < 2 || argc > 2)
	{
		printf("Enter correct parameter, \n->1 See memory\n->2 State of processes\n (:");
	}
	else if(atoi(argv[1])==1)
	{
		spy_memory();
	}
	else if(atoi(argv[1])==2)
	{
		spy_processes();
	}
	else
	{
		printf("Enter correct parameter, \n->1 See memory\n->2 State of processes\n (:");
	}
   return 0;
}