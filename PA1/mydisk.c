#include "mydisk.h"
#include <string.h>
#include <stdlib.h>

FILE *thefile;     /* the file that stores all blocks */
int max_blocks;    /* max number of blocks given at initialization */
int disk_type;     /* disk type, 0 for HDD and 1 for SSD */
int cache_enabled = 0; /* is cache enabled? 0 no, 1 yes */

int mydisk_init(char const *file_name, int nblocks, int type)
{

	// 1. Use the proper mode to open the disk file
	thefile = fopen(file_name, "wb+");
	
	//Check for errors opening or creating a new file
	if(thefile ==  NULL){
		printf("The specified file could not be opened or created.\n");
		return 1;
	}

	disk_type = type;
	max_blocks = nblocks;

	// 2. Create a block initialized with the value of zero
	char *block_ptr = (char*)malloc(BLOCK_SIZE * nblocks);
	memset(block_ptr, '0', BLOCK_SIZE * nblocks);

	// 3. Fill the file with zeros
	fwrite(block_ptr, BLOCK_SIZE, max_blocks, thefile);

	free(block_ptr);

	//Read file and verify its contents
	/*
	mydisk_close();	

	fopen(file_name, "wb+");
	fread(block_ptr, BLOCK_SIZE, 1, thefile);	

	printf("Test output:\n%c\n", block_ptr[0]);

	*/

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
	int bytesToRead = BLOCK_SIZE;

	// Check for incorrect parameters
 	if(block_id > (max_blocks * BLOCK_SIZE)){
		printf("mydisk_read_block: The block ID is greater than the maximum number of blocks.\n");
		return 1;
	}

	/*
	 * This section will read the contents of the specified block by:
	 * 1. Seek the ID of the block
	 * 2. Read the content of the block into the buffer
	 */

	fseek(thefile, block_id, SEEK_SET);

	//Truncate if the end address is in the middle of the block
	if(block_id % (BLOCK_SIZE-1) != 0){
		bytesToRead = block_id % (BLOCK_SIZE-1);
	}

	fread(buffer, bytesToRead, 1, thefile);

	if (cache_enabled) {
		/* TODO: 1. check if the block is cached
		 * 2. if not create a new entry for the block and read from disk
		 * 3. fill the requested buffer with the data in the entry 
		 * 4. return proper return code
		 */
		return 0;
	} else {
		/* TODO: use standard C functions to read from disk
		 */
		return 0;
	}
}

int mydisk_write_block(int block_id, void *buffer)
{
	if(block_id > (max_blocks * BLOCK_SIZE)){
		printf("mydisk_write_block: The block ID is greater than max blocks.\n");
		return 1;
	}

	fseek(thefile, block_id, SEEK_SET);
	fwrite(buffer, BLOCK_SIZE, 1, thefile);


	/* TODO: this one is similar to read_block() except that
	 * you need to mark it dirty
	 */
	return 0;
}

int mydisk_read(int start_address, int nbytes, void *buffer)
{
	int offset, remaining, amount, block_id;
	int cache_hit = 0, cache_miss = 0;

	if(checkReadWriteParams(start_address, nbytes) == 1){
		return 1;
	}

	//Number of blocks currently read
	amount = 0; 		

	//Total number of blocks to read
	remaining = 1;

	if(nbytes > BLOCK_SIZE){
		remaining = nbytes / BLOCK_SIZE;

		//Add an additional blocks if there are too many bytes
		if((nbytes - remaining * BLOCK_SIZE) % BLOCK_SIZE != 0){
			remaining++;
		}
	}

	block_id = start_address / BLOCK_SIZE;

	if(start_address % BLOCK_SIZE != 0){
		offset = start_address % BLOCK_SIZE;
	} else{
		offset = 0;
	}

	printf("Start address: %d\nBytes: %d\n", start_address, nbytes);
	printf("Block ID: %d\nOffset: %d\n", block_id, offset);

	while(amount < remaining){
		int targetAddress = block_id + offset;

		mydisk_read_block(targetAddress, buffer);
		amount++;
		
		printf("Executed read block\n");
	}

	//Perhaps check if the pointer is null?

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

	if(checkReadWriteParams(start_address, nbytes) == 1){
		return 1;
	}

	int offset = 0;

	for(offset; offset < nbytes; offset++){
		mydisk_write_block(start_address + offset, buffer);
	}

	/* TODO: similar to read, except the partial write problem
	 * When a block is modified partially, you need to first read the block,
	 * modify the portion and then write the whole block back
	 */
	return 0;
}

//Helper functions to check mydisk_read and mydisk_write parameters
int checkReadWriteParams(int start_address, int nbytes){

	//Check for valid parameters
	if(start_address < 0 || start_address > (max_blocks * (BLOCK_SIZE-1))){
		printf("mydisk_read: The start address [%d] is out of bounds.\n", start_address);
		return 1;
	}

	/**
	 * What is the maximum number of bytes that can be read?
	 */
	else if(nbytes < 0 || nbytes > max_blocks * BLOCK_SIZE){
		printf("mydisk_read: nbytes [%d] is out of bounds.\n", nbytes);
		return 1;
	}

	else if((nbytes + start_address) > (max_blocks * BLOCK_SIZE)){
		printf("mydisk_read: The maximum specified address [%d] is out of range.\n", nbytes+start_address);
		return 1;
	}
}
