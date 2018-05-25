#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <unistd.h>

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
int type=0;
int *process_shm;
int count_segments[512];


/* Each process has this values */
struct proc_info
{
    int id_process;
    int size_amount_pages;
    int *space;
    int num_segment;       //0 if process is for paging
};

/****************************************************
*Function that returns a list of available space    *
*Parameters:                                        *
*   request: Shared memory                          *
*   number: Amount of pages or segments             *
*****************************************************/
int * search(int *request, int number)
{
    int * list=(int *) malloc(sizeof(int)*number);
    int pos=0;
    for( int i = 0;i < request_size[0]*sizeof(int);i += 4)
    {
        
        /* Return the list of positions of available spaces */
        if(pos==number)
        {
            return list;
        }

        /* Looking for space available in shared memory */
        else
        {
            /* Case is paging search */
            if(type == 0)
            {
                if (request[i]==0)
                {  
                    list[pos]=i;
                    pos++;
                }
            }

            /* Case is segmentation search */
            else
            {
                if(pos != 0 && i != 0)
                {
                    if( request[i - 4] == 0 )
                    {
                        if (request[i] == 0)
                        {
                            list[pos]=i;
                            pos++;
                        }
                    }
                    else
                    {
                        memset(list,0,sizeof(list));
                        pos=0;
                    }

                }
                else
                {
                    if (request[i]==0)
                    {
                        list[pos]=i;
                        pos++;
                    }

                }    
            }

        }
    }
    return NULL;
}


/****************************************************
*Function that records the progress of the process  *
*generator.                                         * 
*Parameters:                                        *
*   PID: id of process                              *
*   action: Action to include in log file           *
*   segment: Number of segment (only segmentation)  *
*   line: Number of space to get involved           *
*****************************************************/
void write_log (int PID, int action, int segment, int line)
{

    FILE *log;

    char filename[] = "bitacora.txt";
    
    log = fopen(filename, "a");
    
    if (log == NULL)
    {
        printf("ERROR: Couldn't open the file bitacora.txt\n"); 
    }
    
    /* Get the time */
    time_t time_process;
    time ( &time_process );
    struct tm * timeinfo = localtime ( &time_process );

    /* Case paging */
    if(type == 0)
    {
        /* Case of assign */
        if(action == 0)
        {
            fprintf(log, "PID: %i; Action: %s; Time: %i:%i:%i; Line: %i\n", PID, "assign", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, line);      
        }

        /* Case of unassign */
        else if (action == 1)
        {
            fprintf(log, "PID: %i; Action: %s; Time: %i:%i:%i; Line: %i\n", PID, "unassign", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, line);      
        } 

        /*Couldn't found space */
        else if (action == 2)
        {
            fprintf(log, "PID: %i; Dead: %s; Time: %i:%i:%i\n", PID, "Couldn't found memory", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);      
        }
    }

    /* Case segmentation */
    else
    {
        /* Case of assign */
        if(action == 0)
        {
            fprintf(log, "PID: %i; Action: %s; Time: %i:%i:%i; Segment: %i; Line: %i\n", PID, "assign", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, segment, line);      
        }

        /* Case of unassign */
        else if (action == 1)
        {
            fprintf(log, "PID: %i; Action: %s; Time: %i:%i:%i; Segment: %i; Line: %i\n", PID, "unassign", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, segment, line);      
        }

        /* Couldn't found space */
        else if (action == 2)
        {
            fprintf(log, "PID: %i; Dead: %s; Time: %i:%i:%i\n", PID, "couldn't found memory", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);      
        }  
    }
     
    
    fclose(log);
}

/****************************************************
*Function that make a replace of values in an array * 
*Parameters:                                        *
*   initial_value: Value to compare                 *
*   new_value: Value to change                      *
*   array: Array to make the change                 *
*   size: Total of elements in the array            *
*****************************************************/
void replace_element(int initial_value, int new_value, int* array, int size)
{
    for(int i = 0;i < size; i++)
    {
        if(array[i] == initial_value)
        {
            array[i] = new_value;
        }
    }
}

/****************************************************
*Function of the thread                             *
*****************************************************/
void* threadfunc(void *param)
{

        /* Get info of the process */
        struct proc_info *process = (struct proc_info*)param;

        int proc = (*process).id_process;
        int * space = process->space;
        int number = (*process).size_amount_pages;

        /* Assign each of the spaces of memory shared */
        for(int i = 0; i < number; i++)
        {
            int j = space[i];
            request_shared_memory[j] = proc;
          
            /* Write the progress in log*/
            if(type == 1)
            {
               write_log(proc, 0, (*process).num_segment , space[i]/4); 
            }
            else if (type == 0)
            {
                write_log(proc, 0, 0, space[i]/4);    
            }
            
        }
        /* Exit semaphore */
        sem_post(main_semaphore);
        printf("\nExit main semaphore: Process #%d \n",proc);

        /* Sleep time to simulate work */
        int time_sleep= get_random(30, 60);
        sleep(time_sleep);
        printf("\nRequest main semaphore: Process #%d \n",proc);
        sem_wait(main_semaphore);

        /* Process finish (case paging) */
        if(type==0)
        {

            sem_wait(process_semaphore);
            process_shm[(proc*8)+4] = 1;    // Assign value that process finish
            sem_post(process_semaphore);
            replace_element(proc, 0, request_shared_memory,request_size[0]*sizeof(int)); 

            /* Report on log */
            for(int i=0;i<number;i++)
            {
                write_log(proc, 1, 0, space[i]/4);    
            }
            
        }

        /* Process finish (case segmentation) */
        else if (type == 1)
        {
            /* Indicates that lines of segment finished */
            for(int i=0;i<number;i++)
            {
                int j=space[i];
                request_shared_memory[j]=0;
                write_log(proc, 1,  (*process).num_segment, space[i]/4); 
            }
            count_segments[proc] -= number;

            /* Assign value that process finish */
            if(count_segments[proc] == 0)
            {
                process_shm[(proc*8)+4] = 1;
            }
        }
        sem_post(main_semaphore);
        printf("\nExit main semaphore: Process #%d \n",proc);   
}

/****************************************************
*Function that find the available space, base on    *
*searching.                                         *
*Parameters:                                        *
*   PID: id of process                              *
*   segment: Number of segment                      *
*   line: Number of space to get involved           *
*****************************************************/
int * finding(int * request, int number, int type)
{
    int times=0;
    int* available_space;

    /* Maximum time to execute (31s)*/
    while(times<31)
    {
        available_space = search(request, number);


        /* Case it couldn't found available space */
        if(available_space==NULL)
        {
           sleep(1);
           times++;
           printf("Finding\n");
        }
        else
        {
            return available_space; 
        }
    }
    return NULL;
}

/************************************************
*Function to apply paging                       *
*************************************************/
void pagination()
{
    int id_process = 0; 
    struct proc_info *process;  

        while(request_size[1]==0)
        {

            /* Create thread and process */
            pthread_t *principal_thread;
            principal_thread = (pthread_t *)malloc(sizeof(*principal_thread));
            process = malloc(sizeof(struct proc_info));

            id_process++;

            /* Get number of pages */
            int count_pages= get_random(1,10);
            printf("\nPages to assign: %d.\n", count_pages);
            printf("\nRequest main semaphore: Process# %d\n",id_process);


            sem_wait(main_semaphore);
            sem_wait(process_semaphore);

            /* Looking for space available*/
            process_shm[id_process*8] = id_process;
            process_shm[(id_process*8)+4] = 3;
            printf("\nWaiting: Process # %d\n",id_process);
            sem_post(process_semaphore);
            int * space=finding(request_shared_memory,count_pages,type);
            printf("\nFinish waiting: Process # %d\n",id_process);

            /* Store information of process */
            (*process).id_process = id_process;
            (*process).size_amount_pages = count_pages;
            process->space = space;
            (*process).num_segment = 0;

            /* Case that space is available */
            if(space != NULL)
            {
                printf("\nRequest process semaphore: Process # %d\n",id_process);
                sem_wait(process_semaphore); 
                process_shm[(id_process*8)+4] = 0; // Assign value that process is active 
                sem_post(process_semaphore); 
                printf("\nExit process semaphore: Process # %d\n",id_process);
                pthread_create(principal_thread, NULL,threadfunc, (void *)process);
            }

            /* Case that space is not available (process die) */
            else
            {
                printf("\nDie: Process # %d\n",id_process);
                printf("\nRequest process semaphore: Process # %d\n",id_process); 
                sem_wait(process_semaphore);
                process_shm[(id_process*8)+4] = 2; // Assign value that process die because he couldn't found space
                sem_post(process_semaphore); 
                printf("\nExit process semaphore: Process # %d\n",id_process);
                write_log(id_process, 2, 0, 0);
                sem_post(main_semaphore);
                printf("\nExit main semaphore: Process # %d \n",id_process);
            }

            /*Waiting time to generate new process */
            int waiting_time= get_random(20, 60);
            
            sleep(waiting_time);
        }

}

/************************************************
*Function to apply segmentation                 *
*************************************************/
void segmentation()
{
    int id_process = 0;
    struct proc_info *process;

        while(request_size[1]==0)
        {
            /* Create thread */
            pthread_t *principal_thread;
            principal_thread = (pthread_t *)malloc(sizeof(*principal_thread));
            id_process++;
            
            /* Number of segments */
            int total_segments= get_random(1, 5);
            
            /* Assign number of lines for each segment */
            for(int i=0;i<total_segments;i++)
            {
                process = malloc(sizeof(struct proc_info));

                /* Get size of segment */
                int size_segment= get_random(1, 3);
                count_segments[id_process] += size_segment; 
                printf("Should be assign %d línes.\n", size_segment);        
                int *space=(int *) malloc(sizeof(int)*size_segment);
                printf("\nRequest semaphore: Process # %d\n",id_process);
                sem_wait(main_semaphore);
                sem_wait(process_semaphore);

                /* Looking for available space */
                process_shm[id_process*8] = id_process;
                process_shm[(id_process*8)+4] = 3;  
                printf("\nWaiting: Process # %d\n",id_process);
                sem_post(process_semaphore);
                space=finding(request_shared_memory,size_segment,type);
                printf("\nFinish waiting: Process # %d\n",id_process);

                /* Store information of process */
                (*process).id_process = id_process;
                (*process).size_amount_pages = size_segment;
                process->space = space;
                (*process).num_segment = i;
                if(space!=NULL)
                {
                    printf("\nRequest process semaphore: Process # %d \n",id_process);
                    sem_wait(process_semaphore);
                    process_shm[(id_process*8)+4] = 0; 
                    sem_post(process_semaphore);
                    printf("\nExit process semaphore: Process # %d \n",id_process);
                    pthread_create(principal_thread, NULL,threadfunc, (void *)process);
                    
                }
                else
                { 
                    printf("\nDie: Process # %d\n",id_process);
                    printf("\nRequest process semaphore: Process # %d \n",id_process);
                    sem_wait(process_semaphore);
                    process_shm[(id_process*8)+4] = 2; 
                    sem_post(process_semaphore);
                    printf("\nExit process semaphore: Process # %d \n",id_process);
                    write_log(id_process, 2, i, 0); 
                    sem_post(main_semaphore);
                    printf("\nExit main semaphore: Process # %d \n",id_process);
                }
                
            }
            int waiting_time= get_random(20, 60);
            sleep(waiting_time);
        }

}


/*void print_list(int* list,int number)
{
    for(int i=0; i<number;i++)
    {
        printf("Element %d: %d\n",i, list[i]);
    }
}*/


int main(int argc, char *argv[])
{
    if(argc==2)
    {

        shared_memory_key = 1234;
        request_memory_key = 1235;
        processes_key = 1236;

        srand(time(NULL));
        int shmid = get_id_shared_memory(request_memory_key, sizeof(int));
        request_size = shmat(shmid, (void *)0, 0);
        request_size[2]=1;
       
        int shmI = get_id_shared_memory(shared_memory_key, request_size[0]*sizeof(int)); 
        request_shared_memory = shmat(shmI, (void *)0, 0);
        printf("\nStart semaphores.\n");
        main_semaphore = sem_open(MAIN_SEMAPHORE, O_CREAT, 0644, 1); 
        sem_init(main_semaphore, 0, 1);
        process_semaphore = sem_open(PROCESS_SEMAPHORE, O_CREAT, 0644, 1); 
        sem_init(process_semaphore, 0, 1);
        
        int sizeProcess = (int) sizeof(int) * TOTAL_MEMORY_PROCESS;
        int process_shm_id = get_id_shared_memory(processes_key, sizeProcess); 
        process_shm = shmat(process_shm_id, NULL, 0);  


        if(atoi(argv[1]) == 0)
        {
            type=0;
            printf("Paging\n");
            pagination();
            
        }
        else if(atoi(argv[1]) == 1)
        {
            type=1;
            printf("Segmentation\n");
            segmentation();
        }
        else
        {
            printf("ERROR: Only two values are possible:\n\t0. Paging\n\t1. Segmentation\n");
        }
        request_size[1]=2;
        printf("Finish assign\n");
    }
    else
    {
        printf("ERROR: Only two values are possible:\n\t0. Paging\n\t1.Segmentation\n");
    }
    return 0;   

}



