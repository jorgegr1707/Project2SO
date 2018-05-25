#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "functions.h"

#define TOTAL_MEMORY_PROCESS 2000

extern key_t shared_memory_key;
extern key_t request_memory_key;
extern key_t processes_key;
extern int *request_shared_memory;
extern int *request_size;


int main(int argc, char *argv[])
{   
    /* Case user doesn't pass the arguments or pass more than expected*/
    if(argc != 2)
    {
        printf("Error: Should run initializer in this way: ./initializer <memory space>.\n");
        exit(0);
    }

    char input[10];
    strcpy(input, argv[1]);
    int length = strlen(input);

    /* Case value is not an integer */
    if(!is_numeric(input, length))
    {
        printf("Error: Argument is not an integer\n");
        exit(0);
    }

    int memory_size = atoi(input);

    /* Case memory size is equal to or less than cero */
    if(memory_size < 1)
    {
        printf("Error: Value should be greater than cero.\n");
        exit(0);
    }

    remove("bitacora.txt");

    /* Assign key value */
    shared_memory_key = 1234;
    request_memory_key = 1235;
    processes_key = 1236;
    
    
    /*
    Attach shared memory identified by id
        Reference: http://man7.org/linux/man-pages/man2/shmget.2.html
    */

    request_size = assign_shared_memory(request_memory_key, sizeof(int));

    /*
    Get the values of the request size. (Important for finalizer program). Divided in:
        request size [0] = memory size
        request size [1] = Value to verify if processes finish (Important for finalizer)
        request size [2] = Value to verify if memory is empty (Important for finalizer)
    */

    if(request_size[0]==0 || memory_size <= request_size[0])
    {
        request_size[0] = memory_size;
        request_size[1]=0;
        request_size[2]=0;
        
        /* Get total size for the shared memory to use to simulate memory (Important for the first part of spy program) */
        int size_shared_memory = request_size[0]*sizeof(float);
        request_shared_memory = assign_shared_memory(shared_memory_key, size_shared_memory);
        
        if(request_shared_memory == NULL)
        {
            printf("\nError: Impossible to 'get' request memory, run ./finalizer to resolve problem.\n");
        }

        request_shared_memory = assign_values_memory(request_shared_memory, size_shared_memory);

        /* Attach shared memory for the process (Important for the second part of spy program) */
        int size_process = sizeof(int) * TOTAL_MEMORY_PROCESS;
        int *process_shared_memory = assign_shared_memory(processes_key, size_process); 

        if(process_shared_memory  == NULL)
        {
            printf("\nError: Impossible to assign memory for processes, run ./finalizer to resolve problem.\n");
            exit(0);
        }
        process_shared_memory = assign_values_memory(process_shared_memory, TOTAL_MEMORY_PROCESS);
        print_shared_memory();
        
    }
    else
    {
        printf("\nError: Integer greater than memory, run ./finalizer to resolve problem.\n");
        exit(0);
    }
    
    return 0;
}