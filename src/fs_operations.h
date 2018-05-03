#ifndef __EFS_FSOPS
#define __EFS_FSOPS

#define FUSE_USE_VERSION 31

#include <fuse3/fuse_lowlevel.h>

void efsOpen(fuse_req_t request, fuse_ino_t inode, 
	struct fuse_file_info* fileInfo);

void efsCreate(fuse_req_t request, fuse_ino_t parent, const char* name,
	mode_t mode, struct fuse_file_info* fileInfo);
	
void efsPoll(fuse_req_t request, fuse_ino_t inode, 
	struct fuse_file_info* fileInfo, struct fuse_pollhandle* pollHandle);

void efsRead(fuse_req_t request, fuse_ino_t inode, size_t size, 
	off_t offset, struct fuse_file_info* fileInfo);

void efsRelease(fuse_req_t request, fuse_ino_t inode, 
	struct fuse_file_info* fileInfo);

void efsOpenDir(fuse_req_t request, fuse_ino_t inode, 
	struct fuse_file_info* fileInfo);

void efsReadDir(fuse_req_t request, fuse_ino_t inode, size_t size, 
	off_t offset, struct fuse_file_info* fileInfo);

void efsReadDirPlus(fuse_req_t request, fuse_ino_t inode, size_t size, 
	off_t offset, struct fuse_file_info* fileInfo);
	
void efsReleaseDir(fuse_req_t request, fuse_ino_t inode, 
	struct fuse_file_info* fileInfo);

void efsStatFs(fuse_req_t request, fuse_ino_t inode);

void efsLookup(fuse_req_t request, fuse_ino_t parent, const char* name);

void efsGetAttr(fuse_req_t request, fuse_ino_t inode, 
	struct fuse_file_info* fileInfo);

void efsAccess(fuse_req_t request, fuse_ino_t inode, int mask);

void efsGetLock(fuse_req_t request, fuse_ino_t inode,
	struct fuse_file_info* fileInfo, struct flock* lock);

void efsGetXattr(fuse_req_t request, fuse_ino_t inode, const char* name,
	size_t size);
	
void efsSyncDir(fuse_req_t request, fuse_ino_t inode, int datasync,
	struct fuse_file_info* fileInfo);

#endif
