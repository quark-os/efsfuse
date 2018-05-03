#include <stdlib.h>

#include "file_table.h"

FileTable* constructFileTable()
{
	FileTable* table = malloc(sizeof(FileTable));
	FileTableNode* head = malloc(sizeof(FileTableNode));
	head->fileDescriptor = NULL;
	head->next = NULL;
	head->table = table;
	table->head = head;
	table->last = head;
	table->size = 0;
	return table;
}

FileTableNode* fileTableSearchInode(FileTable* table, uint64_t inode)
{
	if(table != NULL)
	{
		FileTableNode* node = table->head;
		while(node->next != NULL)
		{
			node = node->next;
			if(node->fileDescriptor->fileID == inode)
			{
				return node;
			}
		}
	}
	return NULL;
}

FileTableNode* fileTableInsert(FileTable* table, FileTableNode* location, EFSCompactFileDescriptor* data)
{
	if(location->table == table && table != NULL)
	{
		FileTableNode* newNode = malloc(sizeof(FileTableNode));
		if(newNode != NULL)
		{
			newNode->table = table;
			newNode->next = location->next;
			newNode->fileDescriptor = data;
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

bool fileTableRemove(FileTable* table, FileTableNode* node)
{
	if(node->table == table && node != table->head && table != NULL)
	{
		FileTableNode* prev = NULL;
		FileTableNode* next = table->head;
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

void destroyFileTable(FileTable* table)
{
	FileTableNode* prev = NULL;
	FileTableNode* next = table->head;
	do
	{
		prev = next;
		next = next->next;
		free(prev);
	} while(next != NULL);
	free(table);
}
