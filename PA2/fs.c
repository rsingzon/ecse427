/**
 * Singzon, Ryan
 * 260397455
 * 
 * ECSE 427: Operating Systems
 * Programming Assignment 2
 * Fall 2013
 * Prof Liu
 */

#include "fs.h"
#include "ext.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* constant of how many bits in one freemap entry */
#define SFS_NBITS_IN_FREEMAP_ENTRY (sizeof(u32)*8)

/* in-memory superblock (in consistent with the disk copy) */
static sfs_superblock_t sb;
/* freemap, u32 array (in consistent with the disk copy) */
static u32 *freemap;
/* file descriptor table */
static fd_struct_t fdtable[SFS_MAX_OPENED_FILES];

/* 
 * Flush the in-memory freemap to disk 
 */
static void sfs_flush_freemap()
{
	size_t i;
	blkid bid = 1;
	char *p = (char *)freemap;


	/* TODO: write freemap block one by one */

	
}

/* 
 * Allocate a free block, mark it in the freemap and flush the freemap to disk
 */
static blkid sfs_alloc_block()
{
	u32 size = sb.nfreemap_blocks * BLOCK_SIZE / sizeof(u32);	
	//  size = 1 * 512 / 4 = 128 four-byte sections = 4096 bits

	//Renaming variables i and j to be more specific
	u32 fm_blk, fm_byte;
	blkid free_block_id;				
	
	//Search each freemap block
	for(fm_blk = 0; fm_blk < sb.nfreemap_blocks; fm_blk++){
		//Read the freemap block located at the block ID
		int count;
		int freemap_id = fm_blk + 1;

		sfs_read_block(freemap, freemap_id);
		
		/* TODO: find out which bit in the entry is zero,
		   set the bit, flush and return the bid
		*/
		//Search each index of the freemap block
		for(fm_byte = 0; fm_byte < BLOCK_SIZE; fm_byte++){

			u32 bitmap = freemap[fm_byte];

			if(bitmap & 0xFFFF != 0){
				//There is a free bit at this index in the freemap
				//BID = BLOCK ID
				printf("Free block at freemap[%d]\n", fm_byte);
				
				u32 mask = 0x1;
				u32 result;
				count = 0;
				
				//Search each bit of the freemap block
				while(count < SFS_NBITS_IN_FREEMAP_ENTRY){
					
					result = bitmap & mask;

					printf("Bitmap: %#08x\n", bitmap);
					printf("Result: %#08x\n", result);
					printf("Mask  : %#08x\n", mask);

					if(result == mask){
						printf("Block %d occupied\n", count);
						printf("\n");
						mask = mask << 1;
						count++;
						
					} else{
						free_block_id = fm_byte * SFS_NBITS_IN_FREEMAP_ENTRY + count;
						printf("Block %d free\n\n", count);

						//Set the bit and flush to disk
						u32 newBitmap = bitmap | mask;
						freemap[fm_byte] = newBitmap;

						//Flush freemap to disk
						sfs_write_block(freemap, freemap_id);
						
						/* TODO: FIGURE OUT FLUSH FREEMAP */
						//sfs_flush_freemap();

						return free_block_id;
					}
				}
			}
		}
	}
	

	return 0;
}

/*
 * Free a block, unmark it in the freemap and flush
 */
static void sfs_free_block(blkid bid)
{
	/* TODO find the entry and bit that correspond to the block */
	int entry_loc;
	int bit_loc;
	/* TODO unset the bit and flush the freemap */
}

/* 
 * Resize a file.
 * This file should be opened (in the file descriptor table). The new size
 * should be larger than the old one (not supposed to shrink a file)
 */
static void sfs_resize_file(int fd, u32 new_size)
{
	/* the length of content that can be hold by a full frame (in bytes) */
	int frame_size = BLOCK_SIZE * SFS_FRAME_COUNT;
	/* old file size */
	int old_size = fdtable[fd].inode.size;
	/* how many frames are used before resizing */
	int old_nframe = (old_size + frame_size -1) / frame_size;
	/* how many frames are required after resizing */
	int new_nframe = (new_size + frame_size - 1) / frame_size;
	int i, j;
	blkid frame_bid = 0;
	sfs_inode_frame_t frame;

	/* TODO: check if new frames are required */
	
	/* TODO: allocate a full frame */

	/* TODO: add the new frame to the inode frame list
	   Note that if the inode is changed, you need to write it to the disk
	*/
}

/*
 * Get the bids of content blocks that hold the file content starting from cur
 * to cur+length. These bids are stored in the given array.
 * The caller of this function is supposed to allocate the memory for this
 * array. It is guaranteed that cur+length<size
 * 
 * This function returns the number of bids being stored to the array.
 */
static u32 sfs_get_file_content(blkid *bids, int fd, u32 cur, u32 length)
{
	/* the starting block of the content */
	u32 start;
	/* the ending block of the content */
	u32 end;
	u32 i;
	sfs_inode_frame_t frame;

	/* TODO: find blocks between start and end.
	   Transverse the frame list if needed
	*/
	return 0;
}

/*
 * Find the directory of the given name.
 *
 * Return block id for the directory or zero if not found
 */
static blkid sfs_find_dir(char *dirname)
{
	blkid dir_bid = 0;
	sfs_dirblock_t dir;

	//While the value of next_dir != 0, continue traversing the 
	//list of directories until dirname is found

	//Need to get a function to return the directory at the block
	//specified
	
	/* TODO: start from the sb.first_dir, traverse the linked list */
	dir_bid = sb.first_dir;
	printf("ID of first dir: %d\n", sb.first_dir);
	char block_ptr[BLOCK_SIZE];

	while(dir_bid != 0){

		
		sfs_read_block(block_ptr, dir_bid);

		dir = *(sfs_dirblock_t*)block_ptr;

		if(dirname == dir.dir_name){
			return dir_bid;
		} else{
			//Go to the next directory in the list
			dir_bid = dir.next_dir;
		}
	}

	
	return 0;
}

/*
 * Create a SFS with one superblock, one freemap block and 1022 data blocks
 *
 * The freemap is initialized be 0x3(11b), meaning that
 * the first two blocks are used (sb and the freemap block).
 *
 * This function always returns zero on success.
 */
int sfs_mkfs()
{
	/* one block in-memory space for freemap (avoid malloc) */
	static char freemap_space[BLOCK_SIZE];
	int i;
	sb.magic = SFS_MAGIC;
	sb.nblocks = 1024;
	sb.nfreemap_blocks = 1;
	sb.first_dir = 0;
	for (i = 0; i < SFS_MAX_OPENED_FILES; ++i) {
		/* no opened files */
		fdtable[i].valid = 0;
	}
	sfs_write_block(&sb, 0);
	freemap = (u32 *)freemap_space;
	memset(freemap, 0, BLOCK_SIZE);
	/* just to enlarge the whole file */
	sfs_write_block(freemap, sb.nblocks);


	/* initializing freemap */
	freemap[0] = 0x3; /* 11b, freemap block and sb used*/
	sfs_write_block(freemap, 1);
	// Parameters(*buffer, blockId)

	memset(&sb, 0, BLOCK_SIZE);
	// Fills the first BLOCK_SIZE bytes at pointer &sb with 0s.
	
	return 0;
}

/*
 * Load the super block from disk and print the parameters inside
 */
sfs_superblock_t *sfs_print_info()
{
	sfs_read_block(&sb, 0);

	printf("\nSFS_PRINT_INFO\n");
	printf("Number of blocks: %d\n", sb.nblocks);
	printf("Num freemap blocks: %d\n\n", sb.nfreemap_blocks);

	return &sb;
}

/*
 * Create a new directory and return 0 on success.
 * If the dir already exists, return -1.
 */
int sfs_mkdir(char *dirname)
{
	//Check if the directory has a valid name
	if(sizeof(dirname) > 120){
		printf("Dir name exceeds max allowed characters.\n");
		return -1;
	}

	printf("Size of struct: %lx\n", sizeof(sfs_dirblock_t));

	/* TODO: test if the dir exists */
	if(sb.first_dir != 0){
		if(sfs_find_dir(dirname) != 0){
			return -1;
		} 
	} else{
		printf("The specified directory does not exist\n");
		
		//Initialize variables
		blkid next_dir;
		char block_ptr[BLOCK_SIZE];
		sfs_dirblock_t dir;

		//Insert a new directory into the linkedList
		//Helper functions: 
		//sfs_find_dir, sfs_alloc_block, sfs_free_block
		//sfs_flush_freemap
	
		/* TODO: insert a new dir to the linked list */

		//1. Allocate space for a new block
		blkid new_bid = sfs_alloc_block();

		//2. Set the next_dir in the last directory block
		next_dir = sb.first_dir;
		sfs_read_block(block_ptr, next_dir);
		dir = *(sfs_dirblock_t*)block_ptr;

		if(next_dir == 0){
			
			sb.first_dir = new_bid;
		} else{
			while(next_dir != 0){	
				sfs_read_block(block_ptr, next_dir);
				dir = *(sfs_dirblock_t*)block_ptr;
				next_dir = dir.next_dir;
				
				if(next_dir == 0){
					dir.next_dir = new_bid;
				}
			}
		}
		printf("Next directory block: %d\n", new_bid);

		//Create a temporary directory and initialize its members
		sfs_dirblock_t temp_dir;
		temp_dir.next_dir = 0;
		strcpy(temp_dir.dir_name, dirname);

		//Write the temporary block into the disk
		sfs_write_block(&temp_dir, new_bid);

		//Read block back for testing purposes
		sfs_read_block(block_ptr, new_bid);
		dir = *(sfs_dirblock_t*)block_ptr; 
		printf("New block name: %s\n", dir.dir_name);
	}
	
	return 0;
}

/*
 * Remove an existing empty directory and return 0 on success.
 * If the dir does not exist or still contains files, return -1.
 */
int sfs_rmdir(char *dirname)
{
	/* TODO: check if the dir exists */
	
	if(sb.first_dir == 0){
		//The superblock does not point to any directories
		return -1;
	} else{
		
		blkid dir_bid = sfs_find_dir(dirname);

		if(dir_bid == 0){
			//The directory does not exist in sfs
			return -1;
		} else{
			//Remove the specified bid from sfs

		}
	}

	/* TODO: check if no files */
	/* TODO: go thru the linked list and delete the dir*/
	return 0;
}

/*
 * Print all directories. Return the number of directories.
 */
int sfs_lsdir()
{
	/* TODO: go thru the linked list */
	int num_dir = 0;
	char block_ptr[BLOCK_SIZE];
	sfs_dirblock_t dir;

	blkid dir_bid = sb.first_dir;
	printf("lsdir - First block ID: %d\n", dir_bid);


	while(dir_bid != 0){
		sfs_read_block(block_ptr, dir_bid);

		dir = *(sfs_dirblock_t*)block_ptr;
		printf("lsdir: %s\n", dir.dir_name);
		num_dir++;

		dir_bid = dir.next_dir;
	}

	printf("lsdir - Num dirs: %d\n", num_dir);
	return num_dir;
}

/*
 * Open a file. If it does not exist, create a new one.
 * Allocate a file desriptor for the opened file and return the fd.
 */
int sfs_open(char *dirname, char *name)
{
	blkid dir_bid = 0, inode_bid = 0;
	sfs_inode_t *inode;
	sfs_dirblock_t dir;
	int fd;
	int i;

	/* TODO: find a free fd number */
	
	/* TODO: find the dir first */

	/* TODO: traverse the inodes to see if the file exists.
	   If it exists, load its inode. Otherwise, create a new file.
	*/

	/* TODO: create a new file */
	return fd;
}

/*
 * Close a file. Just mark the valid field to be zero.
 */
int sfs_close(int fd)
{
	/* TODO: mark the valid field */
	return 0;
}

/*
 * Remove/delete an existing file
 *
 * This function returns zero on success.
 */
int sfs_remove(int fd)
{
	blkid frame_bid;
	sfs_dirblock_t dir;
	int i;

	/* TODO: update dir */

	/* TODO: free inode and all its frames */

	/* TODO: close the file */
	return 0;
}

/*
 * List all the files in all directories. Return the number of files.
 */
int sfs_ls()
{
	/* TODO: nested loop: traverse all dirs and all containing files*/
	return 0;
}

/*
 * Write to a file. This function can potentially enlarge the file if the 
 * cur+length exceeds the size of file. Also you should be aware that the
 * cur may already be larger than the size (due to sfs_seek). In such
 * case, you will need to expand the file as well.
 * 
 * This function returns number of bytes written.
 */
int sfs_write(int fd, void *buf, int length)
{
	int remaining, offset, to_copy;
	blkid *bids;
	int i, n;
	char *p = (char *)buf;
	char tmp[BLOCK_SIZE];
	u32 cur = fdtable[fd].cur;

	/* TODO: check if we need to resize */
	
	/* TODO: get the block ids of all contents (using sfs_get_file_content() */

	/* TODO: main loop, go through every block, copy the necessary parts
	   to the buffer, consult the hint in the document. Do not forget to 
	   flush to the disk.
	*/
	/* TODO: update the cursor and free the temp buffer
	   for sfs_get_file_content()
	*/
	return 0;
}

/*
 * Read from an opend file. 
 * Read can not enlarge file. So you should not read outside the size of 
 * the file. If the read exceeds the file size, its result will be truncated.
 *
 * This function returns the number of bytes read.
 */
int sfs_read(int fd, void *buf, int length)
{
	int remaining, to_copy, offset;
	blkid *bids;
	int i, n;
	char *p = (char *)buf;
	char tmp[BLOCK_SIZE];
	u32 cur = fdtable[fd].cur;

	/* TODO: check if we need to truncate */
	/* TODO: similar to the sfs_write() */
	return 0;
}

/* 
 * Seek inside the file.
 * Loc is the starting point of the seek, which can be:
 * - SFS_SEEK_SET represents the beginning of the file.
 * - SFS_SEEK_CUR represents the current cursor.
 * - SFS_SEEK_END represents the end of the file.
 * Relative tells whether to seek forwards (positive) or backwards (negative).
 * 
 * This function returns 0 on success.
 */
int sfs_seek(int fd, int relative, int loc)
{
	/* TODO: get the old cursor, change it as specified by the parameters */
	return 0;
}

/*
 * Check if we reach the EOF(end-of-file).
 * 
 * This function returns 1 if it is EOF, otherwise 0.
 */
int sfs_eof(int fd)
{
	/* TODO: check if the cursor has gone out of bound */
	return 0;
}
