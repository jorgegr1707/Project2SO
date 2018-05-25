#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>

#include "functions.h"

#define PROCESS_SEMAPHORE "/semProcess"
#define MAIN_SEMAPHORE "/semMain"

#define TOTAL_MEMORY_PROCESS 2000

extern key_t shared_memory_key;
extern key_t request_memory_key;
extern key_t processes_key;
extern int *request_shared_memory;
extern int *request_size;

sem_t * main_semaphore;
sem_t * process_semaphore;


int main(int argc, char *argv[])
{

    shared_memory_key = 1234;
    request_memory_key = 1235;
    processes_key = 1236;

    printf("Running Finalizer\n");
    
    /* Get shared memory for the initial values (initializer) */
    request_size = assign_shared_memory(request_memory_key, sizeof(int));

    /* Get shared memory for the simulation of memory */
    int shared_memory_size = request_size[0]*sizeof(float);    
    request_shared_memory = assign_shared_memory(shared_memory_key, shared_memory_size);    

    request_size[1]=1;  // Value to verify if all processes finish

    /* Get size of shared memory for processes */
    int sizeProcess = sizeof(int) * TOTAL_MEMORY_PROCESS;

    /* Verify if all processes finish */
    if(request_size[2]==1)
    {

        printf("Waiting to close processes...\n");
        while( request_size[1]==1);
    }

    /* Get identifiers of shared memory */
    int request_memory_id = get_id_shared_memory(request_memory_key, sizeof(int));
    int shared_memory_id = get_id_shared_memory(shared_memory_key, shared_memory_size);
    int proccess_shm_id = get_id_shared_memory(processes_key, sizeProcess);

    /* 
    Remove shared memory (using IPC_RMID) 
        Reference: https://www.ibm.com/support/knowledgecenter/en/ssw_i5_54/apis/ipcshmct.htm
    */

    shmctl(request_memory_id, IPC_RMID, NULL);
    shmctl(shared_memory_id, IPC_RMID, NULL);
    shmctl(proccess_shm_id, IPC_RMID, NULL);

    /* Finish semaphore */
    main_semaphore = sem_open(MAIN_SEMAPHORE, O_CREAT, 0644, 1); 
    process_semaphore = sem_open(PROCESS_SEMAPHORE, O_CREAT, 0644, 1); 
    sem_destroy(main_semaphore); 
    sem_destroy(process_semaphore); 

    printf("End Finalizer\n");
 

    return 0;
}
