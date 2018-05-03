#ifndef __EFSFUSE_UTIL
#define __EFSFUSE_UTIL

#define FUSE_USE_VERSION 31

#include <EFS/file_descriptor.h>
#include <fuse3/fuse_lowlevel.h>
#include <sys/types.h>

#include "file_table.h"
#include "free_space_table.h"
#include "efsstate.h"

/**
 * Read file attributes from the given descriptor, and writes them to a
 * stat structure. Sets st_dev, st_rdev, and st_ctime to 0. If either
 * parameter is a null pointer, this function returns without doing
 * anything.
 * 
 * @param file The descriptor to read from
 * @param attributes The stat structure to write to
 */
void genFileAttributes(EFSCompactFileDescriptor* file, 
	struct stat* attributes);

/**
 * Copies all data from a file descriptor into a more compact structure
 * which stores similar data, but does not match the format of a file
 * descriptor as it is stored on disk.
 * 
 * @param src The padded version to read from
 * @param dest The compact version to write to
 */
void compactFileDescriptor(EFSFileDescriptor* src, 
	EFSCompactFileDescriptor* dest);
	
/**
 * Constructs a table containing the file descriptor of every file in the
 * filesystem, and sets the file table pointer the the filesystem state.
 * 
 * @param state The current filesystem state
 * 
 * @returns A pointer to the file table. Null if the function ran out of
 * memory or encountered an error in the filesystem.
 */
FileTable* readFileTable(EFSState* state);

/**
 * Constructs a table containing the size and location of every region of free
 * space in the filesystem, and set the free space pointer in the filesystem
 * state.
 * 
 * @param state The current filesystem state
 * 
 * @returns A pointer to the free space table. Null upon failure.
 */
FreeSpaceTable* readFreeSpaceTable(EFSState* state);

#endif
