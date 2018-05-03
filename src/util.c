#include "util.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <EFS/file_descriptor.h>
#include <EFS/file_descriptor_node.h>
#include <EFS/free_space_node.h>

void genFileAttributes(EFSCompactFileDescriptor* file, 
	struct stat* attributes)
{
	if(file == NULL || attributes == NULL)
	{
		return;
	}
	memset(attributes, 0, sizeof(attributes));
	attributes->st_ino = file->fileID;
	/* 
	 * Currently, EFS does not support hard links. The filesystem assumes
	 * that any given descriptor is the only one corresponding to a
	 * particular inode.
	 */
	attributes->st_nlink = 1;
	attributes->st_uid = file->ownerUUID;
	attributes->st_gid = file->groupUUID;
	attributes->st_size = file->filesize;
	attributes->st_atime = file->lastAccessed;
	attributes->st_mtime = file->lastModified;
	/*
	 * EFS does not currently track the time of the last status change.
	 */
	attributes->st_ctime = 0;
	attributes->st_blksize = PAGE_SIZE;
	/*
	 * The total number of blocks a file should occupy is its filesize
	 * divided by the blocksize rounded up to the nearest integer.
	 */
	attributes->st_blocks =	  (file->filesize / PAGE_SIZE)
							+ (file->filesize % PAGE_SIZE > 0 ? 1 : 0) + 1;
	attributes->st_mode =	  file->isLink == 1 ? S_IFLNK 
								: (file->isFile == 1 ? S_IFREG 
								: S_IFDIR)
							| (file->ownerRead == 1 ? S_IRUSR : 0)
							| (file->ownerWrite == 1 ? S_IWUSR : 0)
							| (file->ownerExecute == 1 ? S_IXUSR : 0)
							| (file->groupRead == 1 ? S_IRGRP : 0)
							| (file->groupWrite == 1 ? S_IWGRP : 0)
							| (file->groupExecute == 1 ? S_IXGRP : 0)
							| (file->othersRead == 1 ? S_IROTH : 0)
							| (file->othersWrite == 1 ? S_IWOTH : 0)
							| (file->othersExecute == 1 ? S_IXOTH : 0);
}

void compactFileDescriptor(EFSFileDescriptor* src, 
	EFSCompactFileDescriptor* dest)

{
	dest->fileID = src->fileID;
	dest->isFile = src->isFile;
	dest->isLink = src->isLink;
	dest->ownerRead = src->ownerRead;
	dest->ownerWrite = src->ownerWrite;
	dest->ownerExecute = src->ownerExecute;
	dest->groupRead = src->groupRead;
	dest->groupWrite = src->groupWrite;
	dest->groupExecute = src->groupExecute;
	dest->othersRead = src->othersRead;
	dest->othersWrite = src->othersWrite;
	dest->othersExecute = src->othersExecute;
	dest->ownerUUID = src->ownerUUID;
	dest->groupUUID = src->groupUUID;
	dest->parentID = src->parentID;
	dest->lastAccessed = src->lastAccessed;
	dest->lastModified = src->lastModified;
	dest->filesize = src->filesize;
	dest->filename = malloc(strlen(src->filename + 1));
	memcpy(dest->filename, src->filename, strlen(src->filename) + 1);
	int fragmentCount = 0;
	while(src->fragments[fragmentCount].fragmentLocation != 0)
	{
		fragmentCount++;
	}
	dest->fragments = malloc(sizeof(EFSFragmentDescriptor) * fragmentCount);
	dest->numFragments = fragmentCount;
	memcpy(dest->fragments, src->fragments, sizeof(EFSFragmentDescriptor) * fragmentCount);
}

FileTable* readFileTable(EFSState* state)
{
	FileTable* table = constructFileTable();
	EFSFileDescriptorNode* node = malloc(sizeof(EFSFileDescriptorNode));
	int nextNode = state->fileDescriptorList;
	while(nextNode != 0)
	{
		printf("Block of file descriptors at page %d.\n", nextNode);
		fseek(state->filesystemStream, PAGE_SIZE * nextNode, SEEK_SET);
		fread(node, PAGE_SIZE, 1, state->filesystemStream);
		printf("There are %d entries in this block.\n", node->numFileDescriptors);
		for(int i = 1; i < 256; i++)
		{
			EFSFileDescriptor* descriptorPage = malloc(sizeof(EFSFileDescriptor));
			fseek(state->filesystemStream, PAGE_SIZE * nextNode + PAGE_SIZE * i, SEEK_SET);
			fread(descriptorPage, PAGE_SIZE, 1, state->filesystemStream);
			if(descriptorPage->fileID != 0)
			{
				EFSCompactFileDescriptor* descriptor = malloc(sizeof(EFSFileDescriptor));
				compactFileDescriptor(descriptorPage, descriptor);
				if(fileTableInsert(table, table->last, descriptor) == NULL)
				{
					return NULL;
				}
			}
			free(descriptorPage);
		}
		nextNode = node->next;
	}
	free(node);
	state->fileTable = table;
	return table;
}

FreeSpaceTable* readFreeSpaceTable(EFSState* state)
{
	FreeSpaceTable* table = constructFreeSpaceTable();
	if(table != NULL)
	{
		EFSFreeSpaceNode* node = malloc(sizeof(EFSFreeSpaceNode));
		if(node != NULL)
		{
			int nextNode = state->freeRegionList;
			while(nextNode != 0)
			{
				fseek(state->filesystemStream, PAGE_SIZE * nextNode, SEEK_SET);
				fread(node, PAGE_SIZE, 1, state->filesystemStream);
				printf("Region of free space at page %d with size %d\n", nextNode, node->size);
				if(freeSpaceTableInsert(table, table->last, nextNode, node->size) == NULL)
				{
					return NULL;
				}
				nextNode = node->next;
			}
			free(node);
			state->freeSpaceTable = table;
		}
		else
		{
			free(table);
			return NULL;
		}
	}
	return table;
}
