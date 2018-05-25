#ifndef VARIABLES_H
#define VARIABLES_H

/*Keys to get shared memory*/
key_t shared_memory_key;
key_t request_memory_key; 
key_t processes_key;

/*Shared memory*/
int *request_size;
int * request_shared_memory;

#endif