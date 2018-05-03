#ifndef __EFS_STATE
#define __EFS_STATE

#include <stdio.h>

#include "file_table.h"
#include "free_space_table.h"

/**
 * Private data that must persist between calls to the filesystem.
 */
typedef struct efs_state
{
	/**
	 * The stream used to read and write data to the filesystem. Must not
	 * be null while the filesystem is mounted.
	 */
	FILE* filesystemStream;
	
	/**
	 * The first page of the first chunk of file descriptors. Descriptors
	 * are stored in an unrolled linked list. The list is not sorted, so
	 * this may not point to the 'first' node in terms of location within
	 * the filesystem.
	 */
	uint64_t fileDescriptorList;
	
	/**
	 * The first page of the first region of available space. Regions of
	 * available space are treated as a linked list sorted by page index.
	 */
	uint64_t freeRegionList;
	
	/**
	 * The size of the filesystem in pages
	 */
	uint64_t filesystemSize;
	
	/**
	 * A linked list containing descriptors for all files in the
	 * filesystem
	 */
	FileTable* fileTable;
	
	/**
	 * A linked list containing the location and size of every region of free
	 * space in the filesystem.
	 */
	FreeSpaceTable* freeSpaceTable;
	
	/**
	 * A linked list containing descriptors for each file currently being
	 * written to.
	 */
	FileTable* openFiles;
	
} EFSState;

#endif
