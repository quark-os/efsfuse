#ifndef __EFSFUSE_FREE_SPACE
#define __EFSFUSE_FREE_SPACE

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * A node in a linked-list containing information about regions of free
 * space in the filesystem. Each node contains the location and size of
 * a single region.
 */
typedef struct free_space_table_node
{
	/**
	 * Pointer to the table this node is contained in. If this is
	 * invalid, the node should not be used.
	 */
	struct free_space_table* table;
	
	/**
	 * Pointer to the next node in the table. NULL for the last node in
	 * the table.
	 */
	struct free_space_table_node* next;
	
	/**
	 * The page index of the region this node represents.
	 */
	uint64_t location;
	
	/**
	 * The size in pages of the region this node represents.
	 */
	uint64_t size;
	
} FreeSpaceTableNode;

/**
 * A linked list containing the locations and sizes of regions of free space
 * within the filesystem.
 */
typedef struct free_space_table
{
	
	/**
	 * A dummy node that always appears at the beginning of the list. It
	 * stores no useful data, and cannot be removed. It simplifies the
	 * implementation of the list.
	 */
	struct free_space_table_node* head;
	
	/**
	 * The last node in the list.
	 */
	struct free_space_table_node* last;
	
	/**
	 * The number of nodes in the list (excluding the head).
	 */
	size_t size;
	
} FreeSpaceTable;

/**
 * Allocates and constructs an empty \link FreeSpaceTable \endlink.
 * 
 * @returns A pointer to the new \link FreeSpaceTable \endlink, or a 
 * null pointer upon failure to allocate memory.
 */
FreeSpaceTable* constructFreeSpaceTable();

/**
 * Inserts a new node after the given location. Use the list head as the
 * location to insert an element to the beginning of the table.
 * 
 * @param table The table to insert into
 * @param location The node to insert after.
 * @param dataLocation The page index of the region this node represents
 * @param dataSize The size in pages of the resion this node represents
 * 
 * @returns A pointer to the new node. Returns null if a node could not 
 * be allocated or if the location given is invalid.
 */
FreeSpaceTableNode* freeSpaceTableInsert(FreeSpaceTable* table, 
	FreeSpaceTableNode* location, uint64_t dataLocation, uint64_t dataSize);
	
/**
 * Removes the provided node from the table, and deallocates it.
 * 
 * @param table The table to remove from
 * @param node The node to remove
 * 
 * @returns true if the specified node was actually contained in the
 * table. Otherwise, false.
 */
bool freeSpaceTableRemove(FreeSpaceTable* table, FreeSpaceTableNode* node);

/**
 * Deallocates the table and all nodes contained within it.
 * 
 * @param table The table to deallocate
 */
void destroyFreeSpaceTable(FreeSpaceTable* table);

#endif
