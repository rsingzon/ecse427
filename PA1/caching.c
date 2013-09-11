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

int cache_blocks;  /* number of blocks for the cache buffer */

/* TODO: some helper functions e.g. find_cached_entry(block_id) */

int init_cache(int nblocks)
{
	/* TODO: allocate proper data structure (ring buffer)
	 * initialize entry data so that the the ring buffer is empty
	 */

	return 0;
}

int close_cache()
{
	/* TODO: release the memory for the ring buffer */
	return 0;
}

void *get_cached_block(int block_id)
{
	/* TODO: find the entry, return the content 
	 * or return NULL if nut found 
	 */
	return NULL;
}

void *create_cached_block(int block_id)
{
	/* TODO: create a new entry, insert it into the ring buffer
	 * It might kick an exisitng entry.
	 * Remember to write dirty block back to disk
	 * Note that: think if you can use mydisk_write_block() to 
	 * flush dirty blocks to disk
	 */
	return NULL;
}

void mark_dirty(int block_id)
{
	/* TODO: find the entry and mark it dirty */
}

