#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>

#include "variables.h"

/****************************************************
*Function that prints the shared memory actually.   *
*****************************************************/
void print_shared_memory()
{
    int i = 0;

    printf("[");
    while(i < request_size[0]*sizeof(int))
    {
        printf("%d", request_shared_memory[i]);
        i = i + 4;
        if(i != request_size[0]*sizeof(int))
        {
            printf(", ");
        }
    }
    printf("]\n");
}

/****************************************************
*Function that returns the id of the shared memory  *
*Parameters:                                        *
*   key: Variable of type key_t that represents the *
*        key to use to obtain shared memory         *
*   size: Variable of type int that represents the  *
*         total size to use in the shared memory    *
*****************************************************/
int get_id_shared_memory(key_t key, int size)
{
    
    /* 
    Create the space necessary for the shared memory
        If assign already exist, then return the id of that space.
        IPC_CREAT: Create segment
            Reference: https://www.tldp.org/LDP/lpg/node69.html
    */
    int  id = shmget(key, size, 0644 | IPC_CREAT);

    /*Case of error*/
    if (id == -1)
    {
        perror("shmat");
        return -1; 
    }
    return id;
}

/****************************************************
*Function that attach the shared memory with the id.*
*Parameters:                                        *
*   key: Variable of type key_t that represents the *
*        key to use to obtain shared memory         *
*   size: Variable of type int that represents the  *
*         total size to use in the shared memory    *
*****************************************************/
int *assign_shared_memory(key_t key, int size)
{
    int shmid = get_id_shared_memory(key, size);
    return shmat(shmid, (void *)0, 0);
}


/****************************************************
*Function that verify if string is a number         *
*Parameters:                                        *
*   string: Array of chars                          *
*   length: Length of the string                    *
*****************************************************/
int is_numeric(char string[], int length)
{   

    for(int i = 0; i < length; i++)
    {
        if(!isdigit(string[i]))
        {
            return 0;
        }
    }
    return 1;
}

/****************************************************
*Function that return the shared memory already a-  *
*ssign.                                             *
*Parameters:                                        *
*   shared_memory: Integer pointer to modify the    *
        shared memory.                              *
*   size: Variable of type int that represents the  *
*         total size to use in the shared memory    *
*****************************************************/
int *assign_values_memory(int *shared_memory, int size)
{
    for(int i = 0; i < size; i += 4)
    {
        shared_memory[i] = 0;
    }
    return shared_memory;
}

/*****************************************************
*Function that return a random value from the range  *
*[lower, upper].                                     *
*Parameters:                                         *
*   lower: Integer value that represents the minimum *
*           value to get.                            *
*   upper: Integer value that represents the maximum *
*           value to get.                            *
******************************************************/
int get_random(int lower, int upper)
{
    return (rand() % (upper - lower + 1)) + lower;
}