#ifndef __EFSFUSE_FILETABLE
#define __EFSFUSE_FILETABLE

#include <EFS/file_descriptor.h>

#include <stdbool.h>

/**
 * A single node in a linked list of file descriptors. Each node stores
 * a compact version of the file descriptor structure.
 */
typedef struct file_table_node
{
	/**
	 * Pointer to the file descriptor corresponding to this node.
	 */
	EFSCompactFileDescriptor* fileDescriptor;
	
	/**
	 * Pointer to the next node in the list. NULL if this is the last
	 * node.
	 */
	struct file_table_node* next;
	
	/**
	 * Pointer to the table this node is contained in. If this is
	 * invalid, the node should not be used.
	 */
	struct file_table* table;
	
} FileTableNode;

/**
 * A linked list of file descriptors.
 */
typedef struct file_table
{
	/**
	 * A dummy node that always appears at the beginning of the list. It
	 * stores no useful data, and cannot be removed. It simplifies the
	 * implementation of the list.
	 */
	FileTableNode* head;
	
	/**
	 * The last node in the list.
	 */
	FileTableNode* last;
	
	/**
	 * The number of nodes in the list (excluding the head).
	 */
	size_t size;
	
} FileTable;

/**
 * Allocates and constructs an empty \link FileTable \endlink.
 * 
 * @returns A pointer to the new \link FileTable \endlink, or a null
 * pointer upon failure to allocate memory.
 */
FileTable* constructFileTable();

/**
 * Searches the table for a file descriptor with the specified inode.
 * 
 * @param table The table to search
 * @param inode The inode to search for
 * 
 * @returns A pointer to the node in the table containing a descriptor
 * with the specified inode. Null if no such node was found.
 */
FileTableNode* fileTableSearchInode(FileTable* table, uint64_t inode);

/**
 * Inserts a new node after the given location. Use the list head as the
 * location to insert an element to the beginning of the table.
 * 
 * @param table The table to insert into
 * @param location The node to insert after.
 * @param data The data to store in the new node.
 * 
 * @returns A pointer to the new node. Returns null if a node could not 
 * be allocated or if the location given is invalid.
 */
FileTableNode* fileTableInsert(FileTable* table, FileTableNode* location, EFSCompactFileDescriptor* data);

/**
 * Removes the provided node from the table. Deallocates the node, but
 * NOT the file descriptor contained in it.
 * 
 * @param table The table to remove from
 * @param node The node to remove
 * 
 * @returns true if the specified node was actually contained in the
 * table. Otherwise, false.
 */
bool fileTableRemove(FileTable* table, FileTableNode* node);

/**
 * Deallocates the table and all nodes contained within it. Note that
 * this function does not deallocate the file descriptors themselves.
 * 
 * @param table The table to deallocate
 */
void destroyFileTable(FileTable* table);

#endif
