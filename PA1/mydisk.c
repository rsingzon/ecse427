#include "mydisk.h"
#include <string.h>

FILE *thefile;     /* the file that stores all blocks */
int max_blocks;    /* max number of blocks given at initialization */
int disk_type;     /* disk type, 0 for HDD and 1 for SSD */
int cache_enabled = 0; /* is cache enabled? 0 no, 1 yes */

int mydisk_init(char const *file_name, int nblocks, int type)
{

	// 1. Use the proper mode to open the disk file
	thefile = fopen(file_name, "w+");
	
	//Check for errors opening or creating a new file
	if(thefile ==  NULL){
		printf("The specified file could not be opened or created.\n");
		return 1;
	}

	disk_type = type;

	// 2. Create a block initialized with the value of zero
	char emptyBlock[BLOCK_SIZE] = {[0 ... BLOCK_SIZE-1] = 0};
	char *block_ptr = emptyBlock;

	// 3. Fill the file with zeros
	//fwrite parameters:
	//1: Pointer to the array of elements to be written
	//2: Size in bytes of the elements to be written
	//3: Number of elements to be written
	//4: Pointer to the FILE object
	fwrite(block_ptr, BLOCK_SIZE, nblocks, thefile);

	return 0;
}

void mydisk_close()
{
	/* TODO: clean up whatever done in mydisk_init()*/
	int isClosed = fclose(thefile);

	if(isClosed != 0){
		printf("An error occurred closing the disk file.\n");
	} else{
		printf("The disk file was closed successfully.\n");
	}
}

int mydisk_read_block(int block_id, void *buffer)
{
	if (cache_enabled) {
		/* TODO: 1. check if the block is cached
		 * 2. if not create a new entry for the block and read from disk
		 * 3. fill the requested buffer with the data in the entry 
		 * 4. return proper return code
		 */
		return 0;
	} else {
		/* TODO: use standard C functiosn to read from disk
		 */
		return 0;
	}
}

int mydisk_write_block(int block_id, void *buffer)
{
	/* TODO: this one is similar to read_block() except that
	 * you need to mark it dirty
	 */
	return 0;
}

int mydisk_read(int start_address, int nbytes, void *buffer)
{
	int offset, remaining, amount, block_id;
	int cache_hit = 0, cache_miss = 0;

	/* TODO: 1. first, always check the parameters
	 * 2. a loop which process one block each time
	 * 2.1 offset means the in-block offset
	 * amount means the number of bytes to be moved from the block
	 * (starting from offset)
	 * remaining means the remaining bytes before final completion
	 * 2.2 get one block, copy the proper portion
	 * 2.3 update offset, amount and remaining
	 * in terms of latency calculation, monitor if cache hit/miss
	 * for each block access
	 */
	return 0;
}

int mydisk_write(int start_address, int nbytes, void *buffer)
{
	/* TODO: similar to read, except the partial write problem
	 * When a block is modified partially, you need to first read the block,
	 * modify the portion and then write the whole block back
	 */
	return 0;
}
