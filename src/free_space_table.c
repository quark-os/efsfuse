#include "free_space_table.h"

#include <stdlib.h>

FreeSpaceTable* constructFreeSpaceTable()
{
	FreeSpaceTable* table = malloc(sizeof(FreeSpaceTable));
	FreeSpaceTableNode* head = malloc(sizeof(FreeSpaceTableNode));
	table->head = head;
	table->last = head;
	table->size = 0;
	head->table = table;
	head->next = NULL;
	head->location = 0;
	head->size = 0;
	return table;
}

FreeSpaceTableNode* freeSpaceTableInsert(FreeSpaceTable* table, 
	FreeSpaceTableNode* location, uint64_t dataLocation, uint64_t dataSize)
{
	if(location->table == table && table != NULL)
	{
		FreeSpaceTableNode* newNode = malloc(sizeof(FreeSpaceTableNode));
		if(newNode != NULL)
		{
			newNode->table = table;
			newNode->next = location->next;
			newNode->location = dataLocation;
			newNode->size = dataSize;
			location->next = newNode;
			table->size++;
			if(table->last == location)
			{
				table->last = newNode;
			}
			return newNode;
		}
	}
	return NULL;
}

bool freeSpaceTableRemove(FreeSpaceTable* table, FreeSpaceTableNode* node)
{
	if(node->table == table && node != table->head && table != NULL)
	{
		FreeSpaceTableNode* prev = NULL;
		FreeSpaceTableNode* next = table->head;
		do
		{
			prev = next;
			next = next->next;
		} while(next != node && next != NULL);
		
		if(next == node)
		{
			prev->next = next->next;
			if(node == table->last)
			{
				table->last = prev;
			}
			table->size--;
			free(node);
			return true;
		}
	}
	return false;
}

void destroyFreeSpaceTable(FreeSpaceTable* table)
{
	FreeSpaceTableNode* prev = NULL;
	FreeSpaceTableNode* next = table->head;
	do
	{
		prev = next;
		next = next->next;
		free(prev);
	} while(next != NULL);
	free(table);
}
