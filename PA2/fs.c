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

/* in-memory superblock (inconsistent with the disk copy) */
static sfs_superblock_t sb;
/* freemap, u32 array (inconsistent with the disk copy) */
static u32 *freemap;
/* file descriptor table */
static fd_struct_t fdtable[SFS_MAX_OPENED_FILES];

/* 
 * Flush the in-memory freemap to disk 
 */
static void sfs_flush_freemap()
{
	blkid bid = 1;
	char *p = (char *)freemap;

	do{
		sfs_write_block(p, bid);
		bid++;
	} while(bid < sb.nfreemap_blocks);
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
		
		//Search each index of the freemap block
		for(fm_byte = 0; fm_byte < BLOCK_SIZE; fm_byte++){

			u32 bitmap = freemap[fm_byte];

			if(bitmap & 0xFFFF != 0){
				//There is a free bit at this index in the freemap
				//BID = BLOCK ID
							
				u32 mask = 0x1;
				u32 result;
				count = 0;
				
				//Search each bit of the freemap block
				while(count < SFS_NBITS_IN_FREEMAP_ENTRY){
					
					result = bitmap & mask;

					if(result == mask){
						mask = mask << 1;
						count++;
						
					} else{
						free_block_id = fm_byte * SFS_NBITS_IN_FREEMAP_ENTRY + count;

						//Set the bit and flush to disk
						u32 newBitmap = bitmap | mask;
						freemap[fm_byte] = newBitmap;

						sfs_flush_freemap();

						//Clear the data stored in the block
						char empty_blk[BLOCK_SIZE];
						memset(empty_blk, 0, BLOCK_SIZE);

						sfs_write_block(empty_blk, free_block_id);

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
	printf("\nFREEING BLOCK %d\n", bid);
	/* TODO find the entry and bit that correspond to the block */

	/* TODO unset the bit and flush the freemap */
	u32 fm_block;
	u32 bitmap;
	u32 newBitmap;
	u32 mask; 

	//Find the freemap block and index where the block is
	fm_block = bid / (BLOCK_SIZE * 8);
	fm_block = fm_block + 1;	//Freemap blocks start at BID 1

	sfs_read_block(freemap, fm_block);

	bitmap = freemap[bid/32];
	printf("Freemap byte: %d\n", bid/32);

	printf("BID: %d\n", bid);
	printf("BITMAP VALUE BEFORE: %#x\n", bitmap);
	bitmap = bitmap & ~(1<<(bid%32));
	printf("BITMAP VALUE AFTER : %#x\n", bitmap);

	freemap[bid/32] = bitmap;

	sfs_flush_freemap();

	//Remove the content of the block
/*	
	char empty_blk[BLOCK_SIZE];
	memset(empty_blk, 0, BLOCK_SIZE);

	sfs_write_block(empty_blk, bid);
*/
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
	char block_ptr[BLOCK_SIZE];

	while(dir_bid != 0){		
		sfs_read_block(block_ptr, dir_bid);

		dir = *(sfs_dirblock_t*)block_ptr;

		if(strcmp(dirname,dir.dir_name) == 0){
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
	printf("First dir: %d\n", sb.first_dir);
	printf("Num freemap blocks: %d\n\n", sb.nfreemap_blocks);

	printf("FREEBLOCK INFO\n");
	sfs_read_block(freemap, 1);
	printf("freemap[0] = %#x\n\n", freemap[0]);

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

	//Test if the dir exists
	if(sfs_find_dir(dirname) != 0){
		return -1;
	} else{	
		//Initialize variables
		blkid next_dir_id;
		char block_ptr[BLOCK_SIZE];
		sfs_dirblock_t dir;
	
		//Allocate space for a new block
		blkid new_bid = sfs_alloc_block();

		//Set the next_dir in the last directory block
		next_dir_id = sb.first_dir;
		sfs_read_block(block_ptr, next_dir_id);
		dir = *(sfs_dirblock_t*)block_ptr;

		if(next_dir_id == 0){
			sb.first_dir = new_bid;
			sfs_write_block(&sb, 0);
		} else{
			while(next_dir_id != 0){	
				sfs_read_block(block_ptr, next_dir_id);
				dir = *(sfs_dirblock_t*)block_ptr;

				//Next dir is zero, set it to the next available block
				if(dir.next_dir == 0){
					blkid modified_dir_id = next_dir_id;

					sfs_read_block(block_ptr, modified_dir_id);
					dir = *(sfs_dirblock_t*)block_ptr;
										
					dir.next_dir = new_bid;

					//A value was changed, so this must be flushed to disk
					sfs_write_block(&dir, modified_dir_id);
					next_dir_id = 0;

				} else{
					next_dir_id = dir.next_dir;	
				}
			}
		}

		//Create a temporary directory and initialize its members
		//Set the next dir, name, and set all its inodes to zero
		sfs_dirblock_t temp_dir;
		temp_dir.next_dir = 0;
		strcpy(temp_dir.dir_name, dirname);
		memset(&temp_dir.inodes[0], 0, SFS_DB_NINODES * sizeof(blkid));

		//Write the temporary block into the disk
		sfs_write_block(&temp_dir, new_bid);
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
	blkid dir_bid = sfs_find_dir(dirname);

	if(dir_bid == 0){
		printf("ERROR sfs_rmdir: Specified directory does not exist\n");
		return -1;
	}

	

	/* TODO: check if no files 
		DO THIS
		DO THIS
		DO THIS
		DO THIS
	 	DO THIS */

	if(sb.first_dir == 0 || dir_bid == 0){
		//The directory does not exist
		return -1;
	} else{
		//Unset the corresponding bit in the freemap

/*****************************************************
	TO DELETE

		u32 newBitmap;
		u32 mask; 

		//Find the freemap block and index where the block is
		fm_block = dir_bid / (BLOCK_SIZE * 8);
		fm_block = fm_block + 1;	//Freemap blocks start at BID 1

		sfs_read_block(freemap, fm_block);

		bitmap = freemap[dir_bid/32];
		printf("DIR BID: %d\n", dir_bid);
		printf("BITMAP VALUE: %#x\n", bitmap);
		bitmap = bitmap & ~(1<<(dir_bid%32));

		freemap[dir_bid/32] = bitmap;

		sfs_flush_freemap();
******************************************************/
		sfs_free_block(dir_bid);
	}

	/* TODO: go thru the linked list and delete the dir*/
	//Find the directories that come before and after the 
	//directory we want to delete
	blkid next_dir = sb.first_dir;		//First block 
	blkid current_dir;					//Current block is sb
	blkid previous_dir = 0;				//No previous blocks
	char block_ptr[BLOCK_SIZE];
	sfs_dirblock_t dir;

	if(next_dir == dir_bid){
		sfs_read_block(block_ptr, next_dir);
		dir = *(sfs_dirblock_t*)block_ptr;

		blkid block_to_copy = dir.next_dir;

		sfs_read_block(&sb, 0);
		sb.first_dir = block_to_copy;
		sfs_write_block(&sb, 0);		
	}

	//General case
	while(next_dir != dir_bid){
		
		previous_dir = current_dir;	
		
		current_dir = next_dir;
		
		sfs_read_block(block_ptr, next_dir);
		dir = *(sfs_dirblock_t*)block_ptr;

		next_dir = dir.next_dir;

		//The next block is what we want to remove
		if(next_dir == dir_bid){
			//Set the next dir of the current block to the next+1 block
			sfs_read_block(block_ptr, next_dir);
			dir = *(sfs_dirblock_t*)block_ptr;
			blkid block_to_copy = dir.next_dir;

			sfs_read_block(block_ptr, current_dir);
			dir = *(sfs_dirblock_t*)block_ptr;
			dir.next_dir = block_to_copy;

			sfs_write_block(&dir, current_dir);				
						
		}
	}

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

	sfs_read_block(&sb, 0);
	blkid dir_bid = sb.first_dir;

	printf("\n***********LSDIR****************\n");
	printf("0: sb\t\t %d\n", dir_bid);

	if(dir_bid != 0){
		do{
		sfs_read_block(block_ptr, dir_bid);

		dir = *(sfs_dirblock_t*)block_ptr;
		printf("%d: %s", dir_bid, dir.dir_name);
		num_dir++;

		dir_bid = dir.next_dir;
		printf(" \t %d\n", dir_bid);
		} while(dir_bid != 0);	
	}
	

	printf("Number of directories: %d\n", num_dir);
	printf("***********END LSDIR************\n\n");
	return num_dir;
}

/*
 * Open a file. If it does not exist, create a new one.
 * Allocate a file desriptor for the opened file and return the fd.
 */
int sfs_open(char *dirname, char *name)
{
	blkid dir_bid = 0, inode_bid = 0;
	char inode_ptr[BLOCK_SIZE];
	sfs_inode_t inode;
	sfs_dirblock_t dir;

	fd_struct_t *fd_ptr;
	fd_struct_t fd;

	//Finds a free file descriptor number
	int fd_num = 0;	

	while(fd_num < SFS_MAX_OPENED_FILES){
		fd = fdtable[fd_num];

		if(fd.valid == 0){
			fd.valid = 1;
			fd.cur = 1;
			
			break;

		} else if(fd_num == SFS_MAX_OPENED_FILES - 1){ 
			printf("The maximum number of files are already in use\n");
			return -1;

		} else{
			fd_num++;	
		}
	}

	
	//Find the directory in which the specified file is contained
	dir_bid = sfs_find_dir(dirname);
	sfs_read_block(&dir, dir_bid);
	fd.dir_bid = dir_bid;

	/* TODO: traverse the inodes to see if the file exists.
	   If it exists, load its inode. Otherwise, create a new file.
	*/

	//Iterate through all the indices of the inodes to find the file
	int count = 0;
	int fd_set = 0;
	int free_inode = -1;
	char *file_name;
	
	while(count < SFS_DB_NINODES){
	
		inode_bid = dir.inodes[count];

		if(inode_bid == 0){
			if(free_inode == -1){
				free_inode = count;
			}
			count++;
			continue;
		}

		//Read the inode at the specified index
		sfs_read_block(inode_ptr, inode_bid);
		inode = *(sfs_inode_t*)inode_ptr;

		file_name = inode.file_name;

		if(strcmp(file_name, name) == 0){
	
			fd.inode = inode;
			fd.inode_bid = count;
			fd_set = 1;

			break;
		}
		count++;
	}

	/* TODO: create a new file */
	//Allocate a block for the inode
	//Put the inode bid into the directory inode array
	//Allocate a block for the first frame
	//Point the inode to the frame
	//Set a name for the inode
	//Place the inode in the file descriptor array

	if(fd_set == 0){
		blkid new_inode_bid = sfs_alloc_block();

		dir.inodes[free_inode] = new_inode_bid;
		sfs_write_block(&dir, dir_bid);

		strcpy(inode.file_name, name);
		
		inode.first_frame = sfs_alloc_block();

		fd.inode = inode;
		fd.inode_bid = new_inode_bid;

		sfs_write_block(&inode, new_inode_bid);

		sfs_read_block(&inode, new_inode_bid);

		printf("FIRST FRAME: %d\n", inode.first_frame);
		printf("FILE NAME: %s\n", inode.file_name);
	}

	//Set the file descriptor in the file descriptor table	
	fdtable[fd_num] = fd;
	printf("\nfd: %d\nInode bid: %d\nDir bid: %d\n", fd_num, fd.inode_bid, fd.dir_bid);

	return fd_num;
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
int sfs_remove(int fd_num)
{
	blkid frame_bid;
	sfs_dirblock_t dir;
	char block_ptr[BLOCK_SIZE];
	int i;

	fd_struct_t fd = fdtable[fd_num];

	printf("fd num: %d\n", fd_num);

	/* TODO: update dir */
	//Set the index of the inode to zero
	blkid dir_bid = fd.dir_bid;
	blkid inode_bid = fd.inode_bid;

	printf("File in directory: %d\n", dir_bid);
	printf("Inode to remove: %d\n", inode_bid);

	//Remove the inode from the directory
	sfs_read_block(block_ptr, dir_bid);
	dir = *(sfs_dirblock_t*)block_ptr;

	//Find the index at which the inode is stored
	int count = 0;
	while(count < SFS_DB_NINODES){
		if(dir.inodes[count] == inode_bid){
			dir.inodes[count] = 0;
			break;
		}
		count++;
	}

	dir.inodes[inode_bid] = 0;
	sfs_write_block(&dir, dir_bid);

	/* TODO: free inode and all its frames */
	sfs_inode_t inode;
	sfs_read_block(block_ptr, inode_bid);
	inode = *(sfs_inode_t*)block_ptr;

	char frame_ptr[BLOCK_SIZE];
	sfs_inode_frame_t frame;
	blkid next_frame = inode.first_frame;

	//Iterate through each of the nonzero content blocks
	//sfs_free_block() on all of them
	while(next_frame != 0){
		printf("Frame number: %d\n", next_frame);

		int current_frame = next_frame;

		sfs_read_block(frame_ptr, next_frame);
		frame = *(sfs_inode_frame_t*)frame_ptr;

		//Check each of the entries of the frame and free nonzero blocks
		int i;
		for(i = 0; i < SFS_FRAME_COUNT; i++){
			blkid content_blk = frame.content[i];
			printf("%d: %d\n", i, content_blk);
			if(content_blk != 0){
				sfs_free_block(content_blk);
			}
		}

		next_frame = frame.next;
		frame.next = 0;

		sfs_write_block(&frame, current_frame);
	}

	/* TODO: close the file */

	//Reset the file description to default
	
	fd.valid = 0;
	memset(&fd.inode, 0, sizeof(sfs_inode_t));
	fd.dir_bid = 0;
	fd.inode_bid = 0;

	return 0;
}

/*
 * List all the files in all directories. Return the number of files.
 */
int sfs_ls()
{
	/* TODO: nested loop: traverse all dirs and all containing files*/
	sfs_read_block(&sb, 0);
	blkid next_dir = sb.first_dir;

	char block_ptr[BLOCK_SIZE];
	sfs_dirblock_t dir;
	int num_files = 0;

	//Traverse all directories
	while(next_dir != 0){

		sfs_read_block(block_ptr, next_dir);
		dir = *(sfs_dirblock_t*)block_ptr;

		next_dir = dir.next_dir;

		//Traverse all files within the directory
		sfs_inode_t inode;
		int count = 0;
		printf("\n%s:\n", dir.dir_name);
		while(count < SFS_DB_NINODES){
			
			if(dir.inodes[count] != 0){
				sfs_read_block(block_ptr, dir.inodes[count]);
				inode = *(sfs_inode_t*)block_ptr;

				printf("%s\n", inode.file_name);
				num_files++;
			}
			count++;
		}
	}
	printf("\n");
	return num_files;
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