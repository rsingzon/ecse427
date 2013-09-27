//Singzon, Ryan
//260397455

//ECSE427: Operating Systems
//Fall 2013
//Professor Liu

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mydisk.h"

/* The cache entry struct */
struct cache_entry
{
	int block_id;
	int is_dirty;
	char content[BLOCK_SIZE];
};

int cache_blocks;  	//Number of blocks for the cache buffer

int in; 		//Next free position
int out; 		//Oldest full position
char *block_ptr;  	//Pointer to the memory allocated for the buffer
int cache_size;		//Offset for pointer to the cache
struct cache_entry *queue;	//Pointer to the circular queue
struct cache_entry *entry_ptr;	//Pointer to the entries in the queue


//Helper function to find a cached entry
int find_cached_entry(block_id)
{
	//If entry found
	return 1;

	//else
	//return 0;
}

int init_cache(int nblocks)
{
	in = 0;
	out = 0;

	cache_blocks = nblocks;	
	cache_size = cache_blocks * sizeof(struct cache_entry);

	//Initialize an empty queue
	queue = (struct cache_entry*)malloc(cache_blocks*sizeof(struct cache_entry));
	memset(queue, 0, cache_blocks * sizeof(struct cache_entry));

	enable_cache();

	return 0;
}

int close_cache()
{
	/* TODO: 	1. Flush all dirty blocks
	 *		2. Free cache buffer
	 */


	//free(cache block);

	disable_cache();
	return 0;
}

void *get_cached_block(int block_id)
{	
	/* TODO: Find the entry, return the content 
	 *       or return NULL if not found 
	 */

	out++;
	out = out % cache_blocks;
	
	return NULL;
}

void *create_cached_block(int block_id)
{
	char emptyBlock[BLOCK_SIZE];

	/* TODO: 	
	 * 1. Remember to write dirty block back to disk 
	 * Note that: think if you can use mydisk_write_block() to 
	 * flush dirty blocks to disk
	 */

	//Insert a new cache entry into the queue
	struct cache_entry newBlock = {block_id, 0, *emptyBlock};

	//Check if there is a dirty block at the oldest location in the queue
//	if(*queue[out] != NULL){	}
	
	queue[in] = newBlock;

	in++;
	in = in % cache_blocks;

	return &queue[in];
}

void mark_dirty(int block_id)
{
	/* TODO: find the entry and mark it dirty */
//	*get_cached_block(block_id).is_dirty = 1;	

}

