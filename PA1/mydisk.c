#include "mydisk.h"
#include <string.h>
#include <stdlib.h>

FILE *thefile;     /* the file that stores all blocks */
int max_blocks;    /* max number of blocks given at initialization */
int disk_type;     /* disk type, 0 for HDD and 1 for SSD */
int cache_enabled = 0; /* is cache enabled? 0 no, 1 yes */

int mydisk_init(char const *file_name, int nblocks, int type)
{

	//Open the disk file
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
	memset(block_ptr, 0, BLOCK_SIZE * nblocks);

	// 3. Fill the file with zeros
	fwrite(block_ptr, BLOCK_SIZE, max_blocks, thefile);

	free(block_ptr);

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
	int bytesToRead;

	// Check for incorrect parameters
 	if(block_id > max_blocks){
		printf("mydisk_read_block: The block ID is greater than the maximum number of blocks.\n");
		return 1;
	}

	//Seek to the correct location in the file
	fseek(thefile, block_id * BLOCK_SIZE, SEEK_SET);

	fread(buffer, BLOCK_SIZE, 1, thefile);

//	printf("Start address: %d\nBytes: %d\n", block_id, bytesToRead);
//	printf("Block ID: %d\nOffset: %d\n", block_id/BLOCK_SIZE, block_id%BLOCK_SIZE);

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
	int bytesToWrite;

	if(block_id > max_blocks){
		printf("mydisk_write_block: The block ID is greater than max blocks.\n");
		return 1;
	}

	//Seek to the correct location in the file
	fseek(thefile, block_id * BLOCK_SIZE, SEEK_SET);

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

	//Check parameters for errors
	if(checkParams(start_address, nbytes, buffer) == 1){
		return 1;
	}

	//Number of blocks currently read
	amount = 0; 		

	//Total number of blocks to read
	remaining = 1;

	//Determine the which block contains the start address
	block_id = start_address / BLOCK_SIZE;

	//Determine the offset of the first block to read
	offset = start_address % BLOCK_SIZE;

	if(nbytes + offset > BLOCK_SIZE){
		remaining = nbytes / BLOCK_SIZE;
		
		//Determine if the last block is partially filled
		if((offset + nbytes) % BLOCK_SIZE != 0){
			remaining++;
		}
	}	

	while(remaining > 0){
		printf("READ\n");
				
		//Allocate a temporary space in memory to hold the entire block
		//Move specified content into buffer
		if(amount == 0 && offset != 0){
			char *tempBlock = (char*)malloc(BLOCK_SIZE);
			memset(tempBlock, 0, BLOCK_SIZE);

			if(nbytes + offset > BLOCK_SIZE){
				nbytes = BLOCK_SIZE - offset;
			}

			mydisk_read_block(block_id, tempBlock);	
			memcpy(buffer+block_id*BLOCK_SIZE, tempBlock+offset, nbytes);

			free(tempBlock);

			offset = 0;
		} else{
			mydisk_read_block(block_id, buffer);	
		}
		
		amount++;
		remaining--;
		block_id++;
	}

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

	int offset, remaining, amount, block_id;
	int partialStart, partialEnd, remainderBytes;

	if(checkParams(start_address, nbytes, buffer) == 1){
		return 1;
	}

	//Set values to determine whether a partial write problem exists
	partialStart = 0;
	partialEnd = 0;

	//Number of blocks currently written
	amount = 0; 

	//Total number of blocks to write
	remaining = 1;

	//Offset of first block
	offset = start_address % BLOCK_SIZE;

	if(offset != 0){
		partialStart = 1;
	} 

	if(nbytes + offset > BLOCK_SIZE){
		remaining = nbytes / BLOCK_SIZE;
		
		//Determine if the last block is partially filled
		if((offset + nbytes) % BLOCK_SIZE != 0){
			remaining++;
			partialEnd = 1;
			remainderBytes = (offset + nbytes) % BLOCK_SIZE;
		}
	}

	block_id = start_address / BLOCK_SIZE;

	//If a block has to be partially written, read it first, 
	//modify the areas that need to be changed, and write the entire block back
	if(partialStart == 1){
		printf("PARTIAL START WRITE\n");

		char *tempBlock = (char*)malloc(BLOCK_SIZE);
		memset(tempBlock, 0, BLOCK_SIZE);

		mydisk_read_block(block_id, tempBlock);

		int bytesToWrite = BLOCK_SIZE-offset;

		memcpy(tempBlock+offset, buffer, bytesToWrite);

		memcpy(buffer, tempBlock, BLOCK_SIZE);
		//memcpy(//destination, source, size n bytes)

		mydisk_write_block(block_id, buffer);
		//printf("********\n%s\n***********\n", (char*)buffer);

		free(tempBlock);

		block_id++;
		remaining--;
		amount++;
	}

	while(remaining > 0){
			
		//Check if there is a partial write on the last block
		if(remaining == 1 && partialEnd == 1){
			printf("PARTIAL END WRITE\n");
			char *tempBlock = (char*)malloc(BLOCK_SIZE);
			memset(tempBlock, 0, BLOCK_SIZE);

			mydisk_read_block(block_id, tempBlock);

			memcpy(tempBlock, buffer+block_id*BLOCK_SIZE, remainderBytes);
			printf("remainderBytes: %d\n", remainderBytes);

			memcpy(buffer+block_id*BLOCK_SIZE, tempBlock, BLOCK_SIZE);
			//memcpy(//destination, source, size n bytes)

			free(tempBlock);
		} else{
			printf("WRITE\n");
		} 
		
		mydisk_write_block(block_id, buffer);			
		//printf("Buffer content: %s\n", (char*)buffer+block_id*BLOCK_SIZE);
		//printf("********\n%s\n***********\n", (char*)buffer);
		

		remaining--;
		amount++;
		block_id++;
	}

	return 0;
}

//Helper functions to check mydisk_read and mydisk_write parameters
int checkParams(int start_address, int nbytes, void* buffer)
{

	//Check for valid parameters
	if(start_address < 0 || start_address > (max_blocks * (BLOCK_SIZE-1))){
		printf("The start address [%d] is out of bounds.\n", start_address);
		return 1;
	}

	else if(nbytes < 0 || nbytes > max_blocks * BLOCK_SIZE){
		printf("nbytes [%d] is out of bounds.\n", nbytes);
		return 1;
	}

	else if(nbytes == 0){
		printf("The number of bytes to read or write is zero.\n");
		return 0;
	}

	else if((nbytes + start_address) > (max_blocks * BLOCK_SIZE)){
		printf("The maximum specified address [%d] is out of range.\n", nbytes+start_address);
		return 1;
	}

	else if(buffer == NULL){
		printf("The pointer to the buffer is null\n");
	}
}
