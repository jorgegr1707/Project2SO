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
*Function that looks for space for the segment      *
*Parameters:                                        *
*   request: Shared memory                          *
*   number: Amount of pages or segments             *
*****************************************************/
int * search_segmentation(int *request, int number)
{
    int * list=(int *) malloc(sizeof(int)*number);
    int pos=0;
    for( int i = 0;i < request_size[0]*sizeof(int);i=i+4)
    {
        
        /* Return the list of positions of available spaces */
        if(pos==number)
        {
            return list;
        }

        /* Looking for space available in shared memory */
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
    return NULL;
}

/****************************************************
*Function that looks for space for pages            *
*Parameters:                                        *
*   request: Shared memory                          *
*   number: Amount of pages or segments             *
*****************************************************/
int * search_paging(int *request, int number)
{
    int * list=(int *) malloc(sizeof(int)*number);
    int pos=0;
    for( int i = 0;i < request_size[0]*sizeof(int);i=i+4)
    {
        /* Return the list of positions of available spaces*/
        if(pos==number)
        {
            return list;
        }

        /* Looking for space available in shared memory */
        else
        {
            if (request[i]==0)
            {  
                list[pos]=i;
                pos++;
            }
        }
    }
    return NULL;
}

/****************************************************
*Function that writes in the file bitacora to record*
*the progress of the process generator.             * 
*Parameters:                                        *
*   PID: id of process                              *
*   action: Action to include in log file           *
*   line: Number of space to get involved           *
*****************************************************/
void write_log_paging (int PID, int action, int line)
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

    /* Case of assign */
    if(action == 0)
    {
        fprintf(log, "Producer\n PID: %i; Action: %s; Time: %i:%i:%i; Line: %i\n", PID, "assign", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, line);      
    }

    /* Case of unassign */
    else if (action == 1)
    {
        fprintf(log, "Producer\n PID: %i; Action: %s; Time: %i:%i:%i; Line: %i\n", PID, "unassign", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, line);      
    } 

    /*Couldn't found space */
    else if (action == 2)
    {
        fprintf(log, "Producer\n PID: %i; Dead: %s; Hora: %i:%i:%i\n", PID, "Couldn't found memory", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);      
    } 
    
    fclose(log);
}

/****************************************************
*Function that writes in the file bitacora to record*
*the progress of the process generator.             * 
*Parameters:                                        *
*   PID: id of process                              *
*   action: Action to include in log file           *
*   segment: Number of segment                      *
*   line: Number of space to get involved           *
*****************************************************/
void write_log_segmentation (int PID, int action, int segment, int line)
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

    /* Case of assign */
    if(action == 0)
    {
        fprintf(log, "Producer\n PID: %i; Action: %s; Time: %i:%i:%i; Segment: %i; Line: %i\n", PID, "assign", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, segment, line);      
    }

    /* Case of unassign */
    else if (action == 1)
    {
        fprintf(log, "Producer\n PID: %i; Action: %s; Time: %i:%i:%i; Segment: %i; Line: %i\n", PID, "unassign", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, segment, line);      
    }

    /* Couldn't found space */
    else if (action == 2)
    {
        fprintf(log, "Producer\n PID: %i; Dead: %s; Time: %i:%i:%i\n", PID, "couldn't found memory", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);      
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
               write_log_segmentation(proc, 0, (*process).num_segment , space[i]/4); 
            }
            else if (type == 0)
            {
                write_log_paging(proc, 0, space[i]/4);    
            }
            
        }
        print_shared_memory();

        /* Exit semaphore */
        sem_post(main_semaphore);
        printf("\n\nExit main semaphore: Process #%d \n\n",proc);

        /* Sleep time to simulate work */
        int time_sleep= 30 + rand() % (60+1 - 30);
        sleep(time_sleep);
        printf("\n\nRequest main semaphore: Process #%d \n\n",proc);
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
                write_log_paging(proc, 1, space[i]/4);    
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
                write_log_segmentation(proc, 1,  (*process).num_segment, space[i]/4); 
            }
            count_segments[proc] -= number;

            /* Assign value that process finish */
            if(count_segments[proc] == 0)
            {
                process_shm[(proc*8)+4] = 1;
            }
        }
        print_shared_memory();
        sem_post(main_semaphore);
        printf("\n\nExit main semaphore: Process #%d \n\n",proc);   
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
    int i=0;
    int* n;

    /* Maximum time to execute (31s)*/
    while(i<31)
    {
        /* Case paging */
        if(type==0)
        {
           n=search_paging(request, number);

        }

        /* Case segmentation */
        else
        {
            n=search_segmentation(request,number);
        }

        /* Case it couldn't found available space */
        if(n==NULL)
        {
           sleep(1);
           i++;
           printf("Finding\n");
        }
        else
        {
            return n; 
        }
    }
    return NULL;
}

/************************************************
*Function to apply paging                       *
*************************************************/
void pagination()
{
    int idprocess = 0; 
    struct proc_info *process;  

        while(request_size[1]==0)
        {

            /* Create thread and process */
            pthread_t *mythread;
            mythread = (pthread_t *)malloc(sizeof(*mythread));
            process = malloc(sizeof(struct proc_info));

            idprocess++;

            /* Get number of pages */
            int number= 1 + rand() % (10+1 - 1);
            printf("\n\nPages to assign: %d.\n\n", number);
            printf("\n\nRequest main semaphore: Process# %d\n\n",idprocess);


            sem_wait(main_semaphore);
            sem_wait(process_semaphore);

            /* Looking for space available*/
            process_shm[idprocess*8] = idprocess;
            process_shm[(idprocess*8)+4] = 3;
            printf("\n\nWaiting (LOCK): Process # %d\n\n",idprocess);
            sem_post(process_semaphore);
            int * space=finding(request_shared_memory,number,type);
            printf("\n\nExit from waiting (UNLOCK): Process # %d\n\n",idprocess);

            /* Store information of process */
            (*process).id_process = idprocess;
            (*process).size_amount_pages = number;
            process->space = space;
            (*process).num_segment = 0;

            /* Case that space is available */
            if(space != NULL)
            {
                printf("\n\nRequest process semaphore: Process # %d\n\n",idprocess);
                sem_wait(process_semaphore); 
                process_shm[(idprocess*8)+4] = 0; // Assign value that process is active 
                sem_post(process_semaphore); 
                printf("\n\nExit process semaphore: Process # %d\n\n",idprocess);
                pthread_create(mythread, NULL,threadfunc, (void *)process);
            }

            /* Case that space is not available (process die) */
            else
            {
                printf("\n\nDie: Process # %d\n\n",idprocess);
                printf("\n\nRequest process semaphore: Process # %d\n\n",idprocess); 
                sem_wait(process_semaphore);
                process_shm[(idprocess*8)+4] = 2; // Assign value that process die because he couldn't found space
                sem_post(process_semaphore); 
                printf("\n\nExit process semaphore: Process # %d\n\n",idprocess);
                write_log_paging(idprocess, 2, 0);
                sem_post(main_semaphore);
                printf("\n\nExit main semaphore: Process # %d \n\n",idprocess);
            }

            /*Waiting time to generate new process */
            int waitb= 20 + rand() % (60+1 - 20);
            
            sleep(waitb);
        }

}

/************************************************
*Function to apply segmentation                 *
*************************************************/
void segmentation()
{
    int idprocess = 0;
    struct proc_info *process;

        while(request_size[1]==0)
        {
            pthread_t *mythread;
            mythread = (pthread_t *)malloc(sizeof(*mythread));
            idprocess++;
            
            /* Number of segments */
            int segs= 1 + rand() % (5+1 - 1);
            
            /* Assign number of lines for each segment */
            for(int i=0;i<segs;i++)
            {
                /*Get */
                process = malloc(sizeof(struct proc_info));
                int number= 1 + rand() % (3+1 - 1);
                count_segments[idprocess] += number; 
                printf("\n\nShould be assign %d línes.\n\n", number);        
                int *space=(int *) malloc(sizeof(int)*number);
                printf("\n\nRequest semaphore: Process # %d\n\n",idprocess);
                sem_wait(main_semaphore);
                sem_wait(process_semaphore);
                process_shm[idprocess*8] = idprocess;
                process_shm[(idprocess*8)+4] = 3; // buscando (: 
                printf("\n\nWaiting(LOCK): Process # %d\n\n",idprocess);
                sem_post(process_semaphore);
                space=finding(request_shared_memory,number,type);
                printf("\n\nExit from waiting (UNLOCK): Process # %d\n\n",idprocess);


                (*process).id_process = idprocess;
                (*process).size_amount_pages = number;
                process->space = space;
                (*process).num_segment = i;
                if(space!=NULL)
                {
                    printf("\n\nRequest process semaphore: Process # %d \n\n",idprocess);
                    sem_wait(process_semaphore);
                    process_shm[(idprocess*8)+4] = 0; // vivo (: 
                    sem_post(process_semaphore);
                    printf("\n\nExit process semaphore: Process # %d \n\n",idprocess);
                    pthread_create(mythread, NULL,threadfunc, (void *)process);
                    
                }
                else
                { 
                    printf("\n\nMuere: Proceso # %d\n\n",idprocess);
                    printf("\n\nRequest process semaphore: Process # %d \n\n",idprocess);
                    sem_wait(process_semaphore);
                    process_shm[(idprocess*8)+4] = 2; // murio porq no encontro :( 
                    sem_post(process_semaphore);
                    printf("\n\nExit process semaphore: Process # %d \n\n",idprocess);
                    write_log_segmentation(idprocess, 2, i, 0); 
                    sem_post(main_semaphore);
                    printf("\n\nExit main semaphore: Process # %d \n\n",idprocess);
                }
                
            }
            int waitb= 20 + rand() % (60+1 - 20);
            sleep(waitb);
        }

}


void print_list(int* list,int number)
{
    for(int i=0; i<number;i++)
    {
        printf("Element %d: %d\n",i, list[i]);
    }
}


int main(int argc, char *argv[])
{
    if(argc==2)
    {

        shared_memory_key = 1234;
        request_memory_key = 1235;
        processes_key = 1236;

        int shmid = get_id_shared_memory(request_memory_key, sizeof(int));
        request_size = shmat(shmid, (void *)0, 0);
        request_size[2]=1;
       
        int shmI = get_id_shared_memory(shared_memory_key, request_size[0]*sizeof(int)); 
        request_shared_memory = shmat(shmI, (void *)0, 0);
        printf("\n\nStart semaphores.\n\n");
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



