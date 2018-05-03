/**
 * @author Nathanial Giddings
 */

#define FUSE_USE_VERSION 31

#define FT_NODE_SIZE 256

#include <fuse3/fuse_lowlevel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <EFS/superblock.h>
#include <EFS/file_descriptor.h>
#include <EFS/file_descriptor_node.h>

#include "efsstate.h"
#include "file_table.h"
#include "fs_operations.h"
#include "util.h"

static struct fuse_lowlevel_ops operations = {
	.open		= efsOpen,
	.opendir	= efsOpenDir,
	.readdir	= efsReadDir,
	.releasedir	= efsReleaseDir,
	.statfs		= efsStatFs,
    .getattr	= efsGetAttr,
    .release	= efsRelease,
    .read		= efsRead,
    .getxattr	= efsGetXattr,
    .fsyncdir	= efsSyncDir,
    .poll		= efsPoll,
    .access		= efsAccess,
    .getlk		= efsGetLock,
    .lookup		= efsLookup,
    //.readdirplus= efsReadDirPlus,
};

/**
 * Prints instructions on using the program. Called when the program
 * gets invalid arguments.
 */
void printUsage()
{
	printf("Usage: efsfuse [OPTIONS] MOUNTPOINT FILESYSTEM\n");
	fuse_cmdline_help();
	fuse_lowlevel_help();
}

int parseArguments(int argc, char** args, EFSState* fsState)
{
	if(argc < 3 || args[argc - 2][0] == '-' || args[argc - 1][0] == '-')
	{
		printUsage();
		return -1;
	}
	else
	{
		fsState->filesystemStream = fopen(args[argc - 1], "r+");
		if(fsState->filesystemStream == NULL)
		{
			perror("Failed to open provided filesystem");
			return -1;
		}
	}
	return 0;
}

/**
 * Usage:
 * 		efsfuse [OPTIONS] MOUNTPOINT FILESYSTEM
 */
int main(int argc, char** args)
{
	EFSState* fsState = malloc(sizeof(EFSState));
	if(parseArguments(argc, args, fsState) != 0)
	{
		printf("bye");
		return -1;
	}
	int argCount = argc - 1;
		
	struct fuse_args fuseArgs = FUSE_ARGS_INIT(argCount, args);
	struct fuse_session* session;
	struct fuse_cmdline_opts options;
	
	if(fuse_parse_cmdline(&fuseArgs, &options) != 0)
	{
		printf("Failed to parse command line.\n");
		return -1;
	}
	
	if(options.show_help)
	{
		printUsage();
		free(options.mountpoint);
		fuse_opt_free_args(&fuseArgs);
		return 0;
	}
	else if(options.show_version)
	{
		printf("FUSE library version %s\n", fuse_pkgversion());
		fuse_lowlevel_version();
		free(options.mountpoint);
		fuse_opt_free_args(&fuseArgs);
		return 0;
	}
		
	bool err = false;
	session = fuse_session_new(&fuseArgs, &operations, sizeof(operations), (void*) fsState);
	if(session != NULL)
	{
		if(fuse_set_signal_handlers(session) == 0)
		{
			EFSSuperblock* superblock = malloc(sizeof(EFSSuperblock));
			fread(superblock, sizeof(EFSSuperblock), 1, fsState->filesystemStream);
			if(memcmp(superblock->magicNumber, EFS_MAGIC_NUMBER, 16) == 0)
			{
				printf("This filesystem contains %d pages.\n", superblock->filesystemSize);
				fsState->fileDescriptorList = superblock->fileDescriptorTable;
				fsState->freeRegionList = superblock->freeSpaceTable;
				fsState->filesystemSize = superblock->filesystemSize;
				if(fuse_session_mount(session, options.mountpoint) == 0)
				{
					printf("Reading file table: %d\n", readFileTable(fsState));
					printf("Reading free space table: %d\n", readFreeSpaceTable(fsState));
					fsState->openFiles = constructFileTable();
					if(fsState->fileTable != NULL)
					{
						printf("Mounted sucessfully! Filesystem has %d files.\n", fsState->fileTable->size);
						fuse_daemonize(options.foreground);
						if(options.singlethread)
						{
							printf("Running singlethreaded session...\n");
							err = fuse_session_loop(session) == 0 ? 0 : 1;
						}
						else
						{
							printf("Running multithreaded session...\n");
							err = fuse_session_loop_mt_31(session, options.clone_fd) == 0 ? 0 : 1;
						}
					}
					else
					{
						printf("Failed to create file table. ");
					}
					printf("Exiting.\n");
					fuse_session_unmount(session);
				}
				else
				{
					printf("Failed to mount filesystem\n");
					err = true;
				}
			}
			else
			{
				printf("Argument does not contain a valid filesystem. Aborting.\n");
				err = true;
			}
			fuse_remove_signal_handlers(session);
		}
		else
		{
			printf("Failed to set signal handlers\n");
			err = true;
		}
		fuse_session_destroy(session);
	}
	else
	{
		printf("Failed to create FUSE session\n");
		err = true;
	}
	
	free(options.mountpoint);
	fuse_opt_free_args(&fuseArgs);
	if(err)
	{
		printf("Exiting with error.\n");
	}
	return !err ? 0 : -1;
}
