#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>

void print_shared_memory(void);
int get_id_shared_memory(key_t, int);
int *assign_shared_memory(key_t, int);
int is_numeric(char [], int);
int *assign_values_memory(int *, int);


#endif