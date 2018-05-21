#ifndef DOUBLE_LINKED_LIST_H
#define DOUBLE_LINKED_LIST_H

typedef struct node
{
	/*
	process_id, space_id, size

	*/
	int process_id;
	int space_id;
	int size;
	int is_free;
	struct node *next;
	struct node *prev;
}NODE;

typedef struct paging_proc
{
	int id;
	int total_pages;
	int time;
}PAGING_PROC;

typedef struct segment_proc
{
	int id;
	int total_segments;
	int *size_segments;
	int time;
}SEGMENT_PROC;

struct process
{
	union
	{
		PAGING_PROC *p_proc;
		SEGMENT_PROC *s_proc;
	};
};

extern int max_size;

int is_empty(void);
int is_full(void);
int length(void);
void insert_first(int, int, int);
void insert_last(int, int, int);
void insert_at(int, int, int, int);
void display(void);
NODE* delete_first(void);
NODE* delete_last(void);
NODE* delete_at(int);
#endif