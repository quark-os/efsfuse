#include "fs_operations.h"
#include "efsstate.h"
#include "file_table.h"
#include "util.h"

#include <fuse3/fuse_lowlevel.h>
#include <fuse3/fuse_common.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h> 

void efsOpen(fuse_req_t request, fuse_ino_t inode, 
	struct fuse_file_info* fileInfo)
{
	EFSState* fsState = fuse_req_userdata(request);
	FileTableNode* fileToOpen = fileTableSearchInode(fsState->fileTable, inode);
	if(fileToOpen == NULL)
	{
		fuse_reply_err(request, ENOENT);
		return;
	}
	else if(fileToOpen->fileDescriptor->isFile != 1)
	{
		fuse_reply_err(request, EISDIR);
		return;
	}
	else if(fileTableSearchInode(fsState->openFiles, inode) != NULL)
	{
		fuse_reply_err(request, EACCES);
	}
	
	if(fileInfo->flags & 3 == O_WRONLY || fileInfo->flags & 3 == O_WRONLY)
	{
		fileTableInsert(fsState->openFiles, fsState->openFiles->last, fileToOpen->fileDescriptor);
	}
	fuse_reply_open(request, fileInfo);
	printf("\tOpened successfully\n");
}

void efsCreate(fuse_req_t request, fuse_ino_t parent, const char* name,
	mode_t mode, struct fuse_file_info* fileInfo)
{
	
}

void efsPoll(fuse_req_t request, fuse_ino_t inode, 
	struct fuse_file_info* fileInfo, struct fuse_pollhandle* pollHandle)
{
	printf("poll called on inode %d. NOT YET IMPLEMENTED.\n", inode);
	fuse_reply_err(request, ENOSYS);
}

void efsRead(fuse_req_t request, fuse_ino_t inode, size_t size, 
	off_t offset, struct fuse_file_info* fileInfo)
{
	EFSState* fsState = fuse_req_userdata(request);
	FileTableNode* fileToOpen = fileTableSearchInode(fsState->fileTable, inode);
	if(fileToOpen == NULL)
	{
		printf("\tTried to read from nonexistant file\n");
		fuse_reply_err(request, ENOENT);
		return;
	}
	else if(fileToOpen->fileDescriptor->isFile != 1)
	{
		printf("\tTried to read from directory.\n");
		fuse_reply_err(request, EISDIR);
		return;
	}
	else if(fileToOpen->fileDescriptor->numFragments > 1)
	{
		printf("\tFragmented files not yet supported!\n");
		fuse_reply_err(request, ENOENT);
		return;
	}
	else if(offset >= fileToOpen->fileDescriptor->filesize)
	{
		printf("\tTried to read beyond end of file. Returning no data.\n");
		fuse_reply_buf(request, NULL, 0);
		return;
	}
	fseek(fsState->filesystemStream, fileToOpen->fileDescriptor->fragments[0].fragmentLocation * PAGE_SIZE + offset, SEEK_SET);
	size_t bytesToRead = offset + size <= fileToOpen->fileDescriptor->filesize ? size : fileToOpen->fileDescriptor->filesize - offset;
	printf("\tReading %d bytes at position %X.\n", bytesToRead, fileToOpen->fileDescriptor->fragments[0].fragmentLocation * PAGE_SIZE + offset);
	char* buffer = malloc(bytesToRead);
	fread(buffer, bytesToRead, 1, fsState->filesystemStream);
	fuse_reply_buf(request, buffer, bytesToRead);
	free(buffer);
}

void efsOpenDir(fuse_req_t request, fuse_ino_t inode, 
	struct fuse_file_info* fileInfo)
{
	EFSState* fsState = fuse_req_userdata(request);
	FileTableNode* fileToOpen = fileTableSearchInode(fsState->fileTable, inode);
	if(fileToOpen == NULL)
	{
		printf("Tried to opendir nonexistant inode %d\n", inode);
		fuse_reply_err(request, ENOENT);
		return;
	}
	else if(fileToOpen->fileDescriptor->isFile != 0)
	{
		fuse_reply_err(request, ENOTDIR);
		return;
	}
	if(fileInfo->flags & 3 != O_RDONLY)
	{
		// Only read-only access is supported at this point.
		fuse_reply_err(request, EROFS);
		return;
	}
	FileTable* directoryEntries = constructFileTable();
	FileTableNode* node = fsState->fileTable->head;
	while(node->next != NULL)
	{
		node = node->next;
		printf("\tchecking inode %d\n", node->fileDescriptor->fileID);
		if(node->fileDescriptor->parentID == inode)
		{
			printf("\t inode %d is child of %d\n", node->fileDescriptor->fileID, inode);
			fileTableInsert(directoryEntries, directoryEntries->last, node->fileDescriptor);
		}
	}
	printf("\tconstructed table of %d directory entries @ %d\n", directoryEntries->size, directoryEntries);
	fileInfo->fh = (uint64_t) directoryEntries;
	printf("\t%d\n", fuse_reply_open(request, fileInfo));
}

void efsReadDir(fuse_req_t request, fuse_ino_t inode, size_t size, 
	off_t offset, struct fuse_file_info* fileInfo)
{
	EFSState* fsState = fuse_req_userdata(request);
	FileTableNode* fileToOpen = fileTableSearchInode(fsState->fileTable, inode);
	
	if(fileToOpen == NULL)
	{
		printf("Tried to readdir nonexistant inode %d\n", inode);
		fuse_reply_err(request, ENOENT);
		return;
	}
	else if(fileToOpen->fileDescriptor->isFile != 0)
	{
		printf("Tried to readdir non-directory inode %d\n", inode);
		fuse_reply_err(request, ENOTDIR);
		return;
	}
	
	FileTable* stream = (FileTable*) fileInfo->fh;
	FileTableNode* streamPosition = (FileTableNode*) offset;
	printf("\treading dir at offset %d size %d\n", offset, size);
	if(offset == -1)
	{
		fuse_reply_buf(request, NULL, 0);
		return;
	}
	else if(offset == 0)
	{
		if(stream->head->table != stream)
		{
			printf("Tried to readdir with bad fh %d\n", fileInfo->fh);
			fuse_reply_err(request, EBADF);
			return;
		}
		streamPosition = stream->head;
	}
	else if(streamPosition->table != stream)
	{
		fuse_reply_err(request, EBADF);
		return;
	}
	char* buffer = NULL;
	size_t currentBufferSize = 0;
	while(streamPosition->next != NULL)
	{
		streamPosition = streamPosition->next;
		printf("\tdescriptor for inode %d found\n", streamPosition->fileDescriptor->fileID);
		size_t spaceNeededForEntry = fuse_add_direntry(request, 
			NULL, 0, streamPosition->fileDescriptor->filename, NULL, 0);
		if(currentBufferSize + spaceNeededForEntry > size)
		{
			break;
		}
		buffer = realloc(buffer, currentBufferSize + spaceNeededForEntry);
		struct stat st;
		genFileAttributes(streamPosition->fileDescriptor, &st);
		fuse_add_direntry(request, buffer + currentBufferSize, 
			spaceNeededForEntry, streamPosition->fileDescriptor->filename,
			&st, (off_t) streamPosition);
		currentBufferSize += spaceNeededForEntry;
	}
	fuse_reply_buf(request, buffer, currentBufferSize);
	if(buffer != NULL)
	{
		free(buffer);
	}
}

void efsReadDirPlus(fuse_req_t request, fuse_ino_t inode, size_t size, 
	off_t offset, struct fuse_file_info* fileInfo)
{
	//fuse_reply_err(request, ENOSYS);
	efsReadDir(request, inode, size, offset, fileInfo);
}

void efsReleaseDir(fuse_req_t request, fuse_ino_t inode, 
	struct fuse_file_info* fileInfo)
{
	if(((FileTable*) fileInfo->fh)->head->table == ((FileTable*) fileInfo->fh))
	{
		destroyFileTable((FileTable*) fileInfo->fh);
		fuse_reply_err(request, 0);
		return;
	}
	else
	{
		fuse_reply_err(request, EBADF);
		return;
	}
}

void efsStatFs(fuse_req_t request, fuse_ino_t inode)
{
	struct statvfs fsStats;
	memset(&fsStats, 0, sizeof(fsStats));
	EFSState* fsState = fuse_req_userdata(request);
	uint64_t freeBlocks = 0;
	FreeSpaceTableNode* node = fsState->freeSpaceTable->head;
	while(node->next != NULL)
	{
		node = node->next;
		printf("Checking free space node @ %d\n", node);
		freeBlocks += node->size;
	}
	
	fsStats.f_bsize = PAGE_SIZE;
	fsStats.f_frsize = PAGE_SIZE;
	fsStats.f_namemax = 1024;
	fsStats.f_files = fsState->fileTable->size;
	fsStats.f_blocks = fsState->filesystemSize;
	fsStats.f_bfree = freeBlocks;
	fsStats.f_bavail = freeBlocks;
	fuse_reply_statfs(request, &fsStats);
}

void efsLookup(fuse_req_t request, fuse_ino_t parent, const char* name)
{
	EFSState* fsState = fuse_req_userdata(request);
	struct fuse_entry_param directoryEntry;
	memset(&directoryEntry, 0, sizeof(directoryEntry));
	if(strcmp(name, ".") == 0)
	{
		directoryEntry.ino = parent;
		directoryEntry.generation = 1;
		directoryEntry.attr_timeout = 10000.0;
		directoryEntry.entry_timeout = 10000.0;
		directoryEntry.attr.st_ino = parent;
		directoryEntry.attr.st_size = 0;
		directoryEntry.attr.st_blksize = PAGE_SIZE;
		directoryEntry.attr.st_mode = S_IFDIR;
	}
	else if(strcmp(name, "..") == 0)
	{
		directoryEntry.ino = parent;
		directoryEntry.generation = 1;
		directoryEntry.attr_timeout = 10000.0;
		directoryEntry.entry_timeout = 10000.0;
		directoryEntry.attr.st_ino = parent;
		directoryEntry.attr.st_size = 0;
		directoryEntry.attr.st_blksize = PAGE_SIZE;
		directoryEntry.attr.st_mode = S_IFDIR;
	}
	else
	{
		printf("\tSearching for file %s\n", name);
		FileTableNode* node = fsState->fileTable->head;
		while(node->next != NULL)
		{
			node = node->next;
			printf("\tRead node with name %s, inode %d, and parent %d\n", node->fileDescriptor->filename, node->fileDescriptor->fileID, node->fileDescriptor->parentID);
			if(strcmp(node->fileDescriptor->filename, name) == 0 && node->fileDescriptor->parentID == parent)
			{
				printf("\t This node has a matching parent and name.\n");
				directoryEntry.ino = node->fileDescriptor->fileID;
				directoryEntry.generation = 1;
				directoryEntry.attr_timeout = 10000.0;
				directoryEntry.entry_timeout = 10000.0;
				genFileAttributes(node->fileDescriptor, &directoryEntry.attr);
			}
		}
	}
	fuse_reply_entry(request, &directoryEntry);
}

void efsGetAttr(fuse_req_t request, fuse_ino_t inode, 
	struct fuse_file_info* fileInfo)
{
	EFSState* fsState = fuse_req_userdata(request);
	FileTableNode* file = fileTableSearchInode(fsState->fileTable, inode);
	if(file != NULL)
	{
		struct stat fileAttributes;
		genFileAttributes(file->fileDescriptor, &fileAttributes);
		fuse_reply_attr(request, &fileAttributes, 3600.0);
		return;
	}
	fuse_reply_err(request, ENOENT);
}

void efsAccess(fuse_req_t request, fuse_ino_t inode, int mask)
{
	fuse_reply_err(request, /*ENOSYS*/0);
}

void efsGetLock(fuse_req_t request, fuse_ino_t inode,
	struct fuse_file_info* fileInfo, struct flock* lock)
{
	printf("getlock called on inode %d. NOT YET IMPLEMENTED.\n", inode);
	fuse_reply_err(request, ENOSYS);
}

void efsGetXattr(fuse_req_t request, fuse_ino_t inode, const char* name,
	size_t size)
{
	printf("Getxattr called on inode %d. NOT SUPPORTED.\n", inode);
	fuse_reply_err(request, EOPNOTSUPP);
}

void efsSyncDir(fuse_req_t request, fuse_ino_t inode, int datasync,
	struct fuse_file_info* fileInfo)
{
	printf("Syncdir called on inode %d. NOT YET IMPLEMENTED\n", inode);
	fuse_reply_err(request, ENOSYS);
}

void efsRelease(fuse_req_t request, fuse_ino_t inode, 
	struct fuse_file_info* fileInfo)
{
	EFSState* fsState = fuse_req_userdata(request);
	FileTableNode* node = fileTableSearchInode(fsState->openFiles, inode);
	if(node != NULL)
	{
		fileTableRemove(fsState->openFiles, node);
		fuse_reply_err(request, 0);
	}
	else
	{
		fuse_reply_err(request, EBADF);
	}
}
