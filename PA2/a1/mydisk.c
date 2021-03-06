//Singzon, Ryan
//260397455

//ECSE427: Operating Systems
//Fall 2013
//Professor Liu

#include "mydisk.h"
#include <string.h>
#include <stdlib.h>

/* The cache entry struct */
struct cache_entry
{
	int block_id;
	int is_dirty;
	char content[BLOCK_SIZE];
};

FILE *thefile;     /* the file that stores all blocks */
int max_blocks;    /* max number of blocks given at initialization */
int disk_type;     /* disk type, 0 for HDD and 1 for SSD */
int cache_enabled = 0; /* is cache enabled? 0 no, 1 yes */

int mydisk_init(char const *file_name, int nblocks, int type)
{

	thefile = fopen(file_name, "wb+");
	
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
	int isClosed = fclose(thefile);

	if(isClosed != 0){
		printf("An error occurred closing the disk file.\n");
	} else{
		printf("The disk file was closed successfully.\n");
	}
}

int mydisk_read_block(int block_id, void *buffer)
{
	struct cache_entry *entry_ptr;
	int blockOffset = block_id * BLOCK_SIZE;

	// Check for incorrect parameters
 	if(block_id > max_blocks){
		printf("mydisk_read_block: The block ID is greater than the maximum number of blocks.\n");
		return 1;
	}

	if (cache_enabled) {
		/* TODO: 1. check if the block is cached
		 * 2. if not create a new entry for the block and read from disk
		 * 3. fill the requested buffer with the data in the entry 
		 * 4. return proper return code
		 */
		
		//Check if the block is cached
		//if(find_cached_entry(block_id)){}
		
		//If not, create a new entry for the block
		entry_ptr = create_cached_block(block_id);
		
		char *tempBlock = (char*)malloc(BLOCK_SIZE);
		memset(tempBlock, 0, BLOCK_SIZE);

		fseek(thefile, blockOffset, SEEK_SET);
		fread(tempBlock, BLOCK_SIZE, 1, thefile);

		struct cache_entry entry = *(struct cache_entry*)entry_ptr;
		//Assign the content of the block to its corresponding entry in the queue

		free(tempBlock);
		return 0;
	} else {
		

		//Seek to the correct location in the file
		fseek(thefile, blockOffset, SEEK_SET);
		fread(buffer, BLOCK_SIZE, 1, thefile);

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
	int bytesToRead, remainderBytes;
	int cache_hit = 0, cache_miss = 0;

	//Check parameters for errors
	if(checkParams(start_address, nbytes, buffer) == 1){
		return 1;
	}

	amount = 0; 		
	remaining = 1;
	block_id = start_address / BLOCK_SIZE;
	offset = start_address % BLOCK_SIZE;
	bytesToRead = nbytes;
	remainderBytes = nbytes % BLOCK_SIZE;

	if(nbytes + offset > BLOCK_SIZE){
		remaining = nbytes / BLOCK_SIZE;
		bytesToRead = BLOCK_SIZE - offset;
		
		//Determine if the last block is partially filled
		if((offset + nbytes) % BLOCK_SIZE != 0){
			remaining++;
			remainderBytes = (offset + nbytes) % BLOCK_SIZE;
		}
	}

	while(remaining > 0){
		int blockOffset = block_id * BLOCK_SIZE;
		char *tempBlock = (char*)malloc(BLOCK_SIZE);
		memset(tempBlock, 0, BLOCK_SIZE);
		mydisk_read_block(block_id, tempBlock);	
			
		if(amount == 0 && remaining == 1){
			//Content is contained in just one block
			memcpy(buffer, tempBlock+offset, remainderBytes);
		} else if(amount == 0 && offset != 0){
			//First block is partially full
			memcpy(buffer, tempBlock+offset, bytesToRead);
		} else if(remaining == 1 && remainderBytes != 0){
			//Last block is partially full
			memcpy(buffer+bytesToRead, tempBlock, remainderBytes);
		} else{	
			//All in between blocks
			memcpy(buffer, tempBlock, BLOCK_SIZE);
		}

		free(tempBlock);		

		amount++;
		remaining--;
		block_id++;
	}

	return 0;
}

int mydisk_write(int start_address, int nbytes, void *buffer)
{
	int offset, remaining, amount, block_id;
	int partialStart, partialEnd, remainderBytes, bytesToWrite;

	if(checkParams(start_address, nbytes, buffer) == 1){
		return 1;
	}

	//Set values to determine whether a partial write problem exists
	partialStart = 0;
	partialEnd = 0;

	amount = 0; 
	remaining = 1;
	offset = start_address % BLOCK_SIZE;
	bytesToWrite = BLOCK_SIZE-offset;

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

	while(remaining > 0){
		int blockOffset = block_id * BLOCK_SIZE;
				
		//If a block has to be partially written, read it first, 
		//modify the areas that need to be changed, and write the entire block back
		if(amount == 0 && partialStart == 1){
			//The first block is partially full
			char *tempBlock = (char*)malloc(BLOCK_SIZE);
			memset(tempBlock, 0, BLOCK_SIZE);
			mydisk_read_block(block_id, tempBlock);
			memcpy(tempBlock + offset, buffer, bytesToWrite);
			mydisk_write_block(block_id, tempBlock);
			free(tempBlock);

		} else if(remaining == 1 && partialEnd == 1){
			//The last block is partially full
			char *tempBlock = (char*)malloc(BLOCK_SIZE);
			memset(tempBlock, 0, BLOCK_SIZE);
			mydisk_read_block(block_id, tempBlock);
			memcpy(tempBlock, buffer + bytesToWrite, remainderBytes);
			mydisk_write_block(block_id, tempBlock);
			free(tempBlock);

		} else{
			//Write all in between blocks
			mydisk_write_block(block_id, buffer + bytesToWrite);
		} 
		 
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

void enable_cache()
{
	cache_enabled = 1;
}

void disable_cache()
{
	cache_enabled = 0;
}


