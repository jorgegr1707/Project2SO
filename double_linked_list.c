#include <stdio.h>
#include <stdlib.h>


typedef struct node
{
	int process_id;
	int space_id;
	int size;
	int is_free;
	struct node *next;
	struct node *prev;
}NODE;


NODE *head = NULL;
NODE *tail = NULL;

int max_size;			/*Max size of list (this variable should modify in initializer program)*/


int length()
{
	int length = 0;
	NODE *current;

	for(current = head; current != NULL; current = current->next)
	{
		length++;
	}

	return length;
}

int total_size()
{
	int size = 0;
	NODE *current;

	for(current = head; current != NULL; current = current->next)
	{
		size += current->size;
	}

	return size;
}

int is_empty()
{
	return head == NULL;
}

int is_full()
{
	return max_size <= length();
}

void display()
{
	struct node *ptr = head;
	printf("\n[");

	while(ptr != NULL)
	{
		printf("(%d, %d, %d)", ptr->process_id, ptr->space_id, ptr->size);
		ptr = ptr->next;
	}

	printf("]");
}


void insert_first(int process_id, int space_id, int size)
{

	if(is_full())
	{
		return;
	}

	NODE *link = (NODE*) malloc(sizeof(NODE));

	link->process_id = process_id;
	link->space_id = space_id;
	link->size = size;
	link->is_free = 0;

	if(is_empty())
	{
		tail = link;
	}
	else
	{
		head->prev = link;
	}

	link->next = head;
	head = link;
}

void insert_last(int process_id, int space_id, int size)
{
	if(is_full())
	{
		return;
	}

	NODE *link = (NODE*) malloc(sizeof(NODE));
	link->process_id = process_id;
	link->space_id = space_id;
	link->size = size;
	link->is_free = 0;

	if(is_empty())
	{
		tail = link;
	}
	else
	{
		tail->next = link;
		link->prev = tail;
	}

	tail = link;

}

void insert_at(int process_id, int space_id, int size, int pos)
{

	if(is_full())
	{
		return;
	}

	NODE *temp = head;

	if(is_empty())
	{
		return;
	}

	if(pos > length())
	{
		return;
	}

	if(pos == 0)
	{
		insert_first(process_id, space_id, size);
	}
	else if(pos == length())
	{
		insert_last(process_id, space_id, size);
	}
	else
	{
		NODE *link = (NODE*) malloc(sizeof(NODE));
		link->process_id = process_id;
		link->space_id = space_id;
		link->size = size;
		link->is_free = 0;


		while(pos)
		{
			temp = temp->next;
			pos--;
		}

		temp->prev->next = link;
		link->prev = temp->prev;
		temp->prev = link;
		link->next = temp;
	}

}

NODE* delete_first()
{
	NODE *temp = head;

	if(head->next == NULL)
	{
		tail = NULL;
	}
	else
	{
		head->next->prev = NULL;
	}

	head = head->next;
	return temp;
}

NODE* delete_last()
{
	NODE *temp = tail;

	if(head->next == NULL)
	{
		head = NULL;
	}
	else
	{
		tail->prev->next = NULL;
	}
	tail = tail->prev;
	return temp;

}

NODE* delete_at(int pos)
{
	NODE *temp = head;

	printf("%d, %d\n", pos, length());
	if(is_empty())
	{
		return NULL;
	}

	if(pos > length())
	{

		return NULL;
	}

	if(pos == 0)
	{
		delete_first();
	}
	else if(pos == length()-1)
	{
		delete_last();
	}
	else
	{
		while(pos)
		{
			temp = temp->next;
			pos--;
		}

		temp->next->prev = temp->prev;
		temp->prev->next = temp->next;
		return temp;
	}
}