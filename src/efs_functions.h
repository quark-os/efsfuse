#ifndef __EFS_FUNCTIONS
#define __EFS_FUNCTIONS

#include <stdbool.h>
#include <stdint.h>

#include <EFS/file_descriptor.h>

/**
 * Create a new empty file with the specified parent, and write its descriptor to
 * disk.
 * 
 * @param parent The parent of the new file
 * 
 * @returns The inode of the new file
 */
uint64_t createFile(uint64_t parent);

/**
 * Free all space allocated to the specified inode, and clear its file
 * descriptor. If this is called on a directory, it will orphan all of the
 * directory's children.
 * 
 * Fails if the inode does not exist, the inode corresponds to the root 
 * directory, or if an I/O error occured.
 * 
 * @param inode The inode to delete
 * 
 * @returns True upon success, false upon failure.
 */
bool deleteFile(uint64_t inode);

/**
 * Free all space allocated to the specified inode, and clear its file
 * descriptor. If this is called on a directory, it will orphan all of the
 * directory's children.
 * 
 * Fails if the descriptor on disk does not match the provided descriptor, the 
 * inode corresponds to the root directory, or if an I/O error occured.
 * 
 * @param descriptor The inode to delete
 * 
 * @returns True upon success, false upon failure.
 */
bool deleteFile(EFSCompactFileDescriptor* descriptor);

/**
 * Reads the file descriptor with the specified inode. Fails if the specified inode
 * does not exist, or if an I/O error occurs.
 * 
 * @param inode The inode to load the file descriptor of
 * 
 * @returns A pointer to the file descriptor. NULL upon failure.
 */
EFSCompactFileDescriptor* readDescriptor(uint64_t inode);

/**
 * Overwrites a file descriptor on disk with the one specified. Uses the fileID
 * stored in the provided descriptor to search for the descriptor on disk to
 * overwrite. Fails if descriptor->fileID does not match any descriptor on disk,
 * or if an I/O error occurs.
 * 
 * @param descriptor The descriptor to write onto disk. descriptor->fileID
 * specifies which desciptor on disk is to be overwritten.
 * 
 * @returns True upon success, false upopn failure.
 */
bool updateDescriptor(EFSCompactFileDescriptor* descriptor);

/**
 * @file efs_functions.cpp
 * 
 * Read up to the specified number of bytes from a file, starting at the specified
 * offset. Buffer is assumed to be allocated to sufficient size beforehand. 
 * 
 * If EOF is reached before the specified number of bytes have been read, the 
 * function stops and returns the number of bytes that could be read. 
 * 
 * If an I/O error occurs, the function returns 0 and the contents of buffer are 
 * undefined.
 * 
 * @param descriptor The inode from which to read.
 * @param offset The offset to start reading from.
 * @param size The maximum number of bytes to read.
 * @param buffer The location to read into
 * 
 * @returns The number of bytes actually read. 0 upon I/O error.
 */
uint64_t readFile(EFSCompactFileDescriptor* descriptor, uint64_t offset, 
	uint64_t size, char* buffer);

/**
 * Read up to the specified number of bytes from a file, starting at the specified
 * offset. Buffer is assumed to be allocated to sufficient size beforehand. 
 * 
 * If EOF is reached before the specified number of bytes have been read, the 
 * function stops and returns the number of bytes that could be read. 
 * 
 * If an I/O error occurs, the function returns 0 and the contents of buffer are 
 * undefined. If the file does not exist, returns 0 and does not modify the
 * provided buffer.
 * 
 * @param inode The inode from which to read.
 * @param offset The offset to start reading from.
 * @param size The maximum number of bytes to read.
 * @param buffer The location to read into
 * 
 * @returns The number of bytes actually read. 0 upon I/O error.
 */
uint64_t readFile(uint64_t inode, uint64_t offset, uint64_t size, char* buffer);

/**
 * Overwrite the region in the provided inode starting at offset and spanning
 * size bytes with the data stored in buffer. 
 * 
 * If EOF is reached before all data has been written, the function will write 
 * data up to the point but no further.
 * 
 * Fails upon I/O error or of the file does not exist. Does NOT fail if EOF is 
 * reached. To test this, the caller should always compare this function's 
 * return value with size.
 * 
 * @param inode The inode to update.
 * @param offset The position to start writing inside the specified inode.
 * @param size The number of bytes to write
 * @param buffer The data to write
 * 
 * @returns The number of bytes actually written. 0 upon I/O error.
 */
uint64_t updateFile(uint64_t inode, uint64_t offset, uint64_t size, 
	const char* buffer);

/**
 * Overwrite the region in the provided inode starting at offset and spanning
 * size bytes with the data stored in buffer. 
 * 
 * If EOF is reached before all data has been written, the function will write 
 * data up to the point but no further.
 * 
 * Fails upon I/O error or if the file does not exist. Does NOT fail if EOF is 
 * reached. To test this, the caller should always compare this function's 
 * return value with size.
 * 
 * @param descriptor The inode to update.
 * @param offset The position to start writing inside the specified inode.
 * @param size The number of bytes to write
 * @param buffer The data to write
 * 
 * @returns The number of bytes actually written. 0 upon I/O error.
 */
uint64_t updateFile(EFSCompactFileDescriptor* descriptor, uint64_t offset, 
	uint64_t size, const char* buffer);

/**
 * Append the specified data the the end of the specified inode, and update its
 * file descriptor accordingly. This function will create new file fragments if
 * there are not enough free blocks directly after the end of the file.
 * 
 * Only imformation regarding filesize and fragmentation will be updated inside 
 * the descriptor. Last accessed and last modified dates should be updated after 
 * the file is released. 
 * 
 * Fails if an I/O error occurs, if there is not enough space in the filesystem, 
 * if the file cannot be fragmented further, or if the file does not exist.
 * 
 * @param inode The inode to append data to
 * @param size The number of bytes to append
 * @param buffer The data to append to the inode
 * 
 * @returns True upon success, false upon failure.
 */
bool appendFile(uint64_t inode, uint64_t size, const char* buffer);

/**
 * Append the specified data the the end of the specified inode, and update its
 * file descriptor accordingly. This function will create new file fragments if
 * there are not enough free blocks directly after the end of the file.
 * 
 * Only imformation regarding filesize and fragmentation will be updated inside 
 * the descriptor. Last accessed and last modified dates should be updated after 
 * the file is released. 
 * 
 * Fails if an I/O error occurs, if there is not enough space in the filesystem, 
 * if the file cannot be fragmented further, or if the file does not exist.
 * 
 * @param descriptor The inode to append data to
 * @param size The number of bytes to append
 * @param buffer The data to append to the inode
 * 
 * @returns True upon success, false upon failure.
 */
bool appendFile(EFSCompactFileDescriptor* descriptor, uint64_t size, 
	const char* buffer);

/**
 * Adjusts the filesize of the specified inode. Allocates or deallocates blocks at
 * the end of the inode accordingly, and updates filesize and fragmentation
 * information in the file descriptor. 
 * 
 * Fails if an I/O error occurs, if there is not enough space in the filesystem, 
 * if the file could not be fragmented further., or if the file does not exis.
 * 
 * @param inode The inode to resize
 * @param newSize The new size of the inode in bytes
 * 
 * @returns True upon success, false upon failure.
 */
bool resizeFile(uint64_t inode, uint64_t newSize);

/**
 * Adjusts the filesize of the specified inode. Allocates or deallocates blocks at
 * the end of the inode accordingly, and updates filesize and fragmentation
 * information in the file descriptor. 
 * 
 * Fails if an I/O error occurs, if there is not enough space in the filesystem, 
 * if the file could not be fragmented further, or if the file does not exist.
 * 
 * @param descriptor The inode to resize
 * @param newSize The new size of the inode in bytes
 * 
 * @returns True upon success, false upon failure.
 */
bool resizeFile(EFSCompactFileDescriptor* descriptor, uint64_t newSize);

#endif
