#include <stdio.h>
#include <stdlib.h>
#include "list.h"
//tested
struct nodeStruct* List_createNode(void* block, int size){
	struct nodeStruct* swag = (struct nodeStruct*) malloc(sizeof(struct nodeStruct));
	swag->block = block;
	swag->size = size;
	return swag;
}
//tested
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node){
	node->next = *headRef;
	*headRef = node;
}

void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node){
	if (*headRef == NULL)
	{
		List_insertHead(headRef, node);
		return;
	}
	struct nodeStruct* cur = *headRef;
	while(cur->next != NULL)
	{
		cur = cur->next;
	}
	node->next = NULL;
	cur->next = node;
}
//tested
int List_countNodes (struct nodeStruct *head){
	int n = 0;
	if (head == NULL)
	{
		return n;
	}
	struct nodeStruct* cur = head;
	while(cur != NULL){
		n++;
		cur = cur->next;
	}
	return n;
}
//tested
struct nodeStruct* List_findNode(struct nodeStruct *head, void* block){
	struct nodeStruct* cur = head;
	while(cur != NULL){
		if (cur->block == block)
		{
			return cur;
		}
		cur = cur->next;
	}
	return cur;
}
//tested
void List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node){
	if (List_countNodes(*headRef) == 1)
	{
		free(node);
		*headRef = NULL;
		return;
	}
	struct nodeStruct* cur = *headRef;
	if (cur == node)
	{
		*headRef = (*headRef)->next;
		free(node);
		return;
	}
	while(cur != node && cur->next != node && cur->next != NULL)
	{
		cur = cur->next;
	}
	cur->next = node->next;

	free(node);
}

void List_sort (struct nodeStruct **headRef){
	struct nodeStruct* cur = *headRef;
	struct nodeStruct* sorted_list = NULL;
	while(cur != NULL)
	{
		struct nodeStruct* cur_next = cur->next;
		if (sorted_list == NULL || cur->block <= sorted_list->block)
		{
			List_insertHead(&sorted_list, cur);
		}
		else
		{
			struct nodeStruct* cur_sorted = sorted_list;
			// Point of Insertion
			while(cur_sorted->next != NULL && cur->block > cur_sorted->next->block)
			{
				cur_sorted = cur_sorted->next;
			}
			cur->next = cur_sorted->next;
			cur_sorted->next = cur;
		}
		cur = cur_next;
	}
	*headRef = sorted_list;
}
