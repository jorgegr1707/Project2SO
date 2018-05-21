#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "double_linked_list.h"

sem_t sem1, sem2;

pthread_mutex_t mutex;

int get_random(int lower, int upper)
{
	return (rand() % (upper - lower + 1)) + lower;
}



void allocate_page(void *param)
{
	PAGING_PROC *proc = (PAGING_PROC*)param;
	insert_first(proc->total_pages, proc->time, 1);
	printf("Buscando Memoria\n");
	display();

	/*int *value = (int *)param;
	printf("\nCantidad pags: %d\n", *value);
	pthread_mutex_lock(&mutex);
	int i = *value;
	while(i < 10)
	{
		insert_first(*value);
		i++;
		sleep(1);
	}
	display();
	pthread_mutex_unlock(&mutex);
	


	p_proc.total_pages = get_random(1,10);
	p_proc.time = get_random(20,60);

	int sleep_time = get_random(30,60);

	printf("Total pages: %d\n", p_proc.total_pages);
	printf("Time in memory: %d\n", p_proc.time);
	printf("Create process time: %d\n", sleep_time);
	sleep(sleep_time);*/
}

void free_page(void *param)
{
	printf("Liberando Memoria\n");
	//delete_first();
	//display();
	printf("\nLlega aqui\n");
}

void *thread_function(void *param)
{
	/* Looking for memory */
	sem_wait(&sem1);
	allocate_page(param);
	/*TODO: Write in blog*/
	sem_post(&sem1);

	/* Time to be store in memory */
	int sleep_time = get_random(20,60);
	printf("\nTiempo de uso: %d\n", sleep_time);
	sleep(sleep_time);

	/* Free memory */
	sem_wait(&sem2);
	free_page(param);
	/*TODO: Write in blog*/
	sem_post(&sem2);
}

int main(int argc, char* argv[])
{
	printf("Welcome to Process Generator\n\n"
			"Choose the algorithm to apply:\n\n"
			"1. Paging\n"
			"2. Segmentation\n"
			"3. Exit\n\n");

	int option;

	printf("Option: ");
	scanf("%d", &option);

	while((option != 1) && (option != 2) && (option != 3))
	{
		printf("\n\nNot valid option.Choose correctly\n\n");
		printf("Choose: ");
		scanf("%d", &option);
		printf("%d", option);
	}

	srand(time(0));

	/* Case paging */
	if(option == 1)
	{
		int i;
		PAGING_PROC p_proc;
		pthread_t threads_id[1000];

		max_size = 100;

		sem_init(&sem1, 0, 1);
		sem_init(&sem2, 0, 1);
		
		for(i = 0; i < 10; i++)
		{
			p_proc.total_pages = get_random(1,10);
			p_proc.time = get_random(20,60);
			//int value = get_random(1,10);
			pthread_create(&threads_id[i], NULL, thread_function, (void *)&p_proc);
			int sleep_time = get_random(30, 60);
			printf("Tiempo para crear proceso: %d\n", sleep_time);
			sleep(sleep_time);
			
		}

		for(i = 0; i < 10; i++)
		{
			pthread_join(threads_id[i], NULL);
		}
		
		exit(0);
	}

	/* Case segmentation */
	else if(option == 2)
	{
		SEGMENT_PROC s_proc;
		int times_segments = 10;
		while(times_segments)
		{
			s_proc.total_segments = get_random(1,5);
			s_proc.time = get_random(20, 60);
			int segment_values[s_proc.total_segments];
			for (int i = 0; i < s_proc.total_segments; i++)
			{
				segment_values[i] = get_random(1,3);
			}
			s_proc.size_segments = segment_values;

			printf("Total segments: %d\n", s_proc.total_segments);
			printf("Time in memory: %d\n", s_proc.time);

			for(int i = 0; i < s_proc.total_segments; i++)
			{
				printf("\tSegment %d: %d\n", i+1, s_proc.size_segments[i]);
			}

			int sleep_time = get_random(30, 60);

			printf("Create process time: %d\n", sleep_time);
			sleep(sleep_time);
			times_segments--;
		}
		exit(0);
	}

	/* Exit program */
	else if(option == 3)
	{
		exit(0);
	}
	

	/*while(1)
	{
		int num = get_random(30, 60);
		printf("%d\n", num);
		sleep(1);

	}
	return 0;*/
}