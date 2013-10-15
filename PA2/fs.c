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
	u32 start;		//Starting block of content
	u32 end;		//Ending block of content
	sfs_inode_t inode;
	sfs_inode_frame_t frame;
	blkid frame_bid;
	char tmp[BLOCK_SIZE];
	char frame_ptr[BLOCK_SIZE];
	u32 num_bids = 0;			//Counts the number of bids in the array

	/* TODO: find blocks between start and end.
	   Traverse the frame list if needed
	*/

	//Get the bid of the desired frame
	fd_struct_t fd_struct = fdtable[fd];
	sfs_read_block(tmp, fd_struct.inode_bid);
	inode = *(sfs_inode_t*)tmp;

	frame_bid = inode.first_frame;

	start = cur / BLOCK_SIZE;
	end = (cur + length) / BLOCK_SIZE;

	u32 numBlocks = end - start + 1;
	u32 blocks_remaining = numBlocks;

	//Start searching at an offset determined by the start position
	u32 block_count = start % SFS_FRAME_COUNT;	

	//This loop will iterate through the frames
	while(blocks_remaining > 0){

		sfs_read_block(frame_ptr, frame_bid);
		frame = *(sfs_inode_frame_t*)frame_ptr;

		//Iterate through the content array of the frame
		while(block_count < SFS_FRAME_COUNT && blocks_remaining > 0){
			//printf("\nBID: %d\n", frame.content[block_count]);
			bids[num_bids] = frame.content[block_count];
			num_bids++;
			block_count++;
			blocks_remaining--;
		}

		if(blocks_remaining != 0){
			frame_bid = frame.next;
		}
	
	}

	return num_bids;
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
			fd.cur = 0;
			
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
			printf("FILE FOUND AT INODE BID %d\n", inode_bid);
			fd.inode = inode;
			fd.inode_bid = inode_bid;
			fd_set = 1;

			break;
		}
		count++;
	}

	/* TODO: create a new file */
	
	if(fd_set == 0){
		printf("CREATING A NEW FILE\n");
		//Allocate a block for the inode
		blkid new_inode_bid = sfs_alloc_block();

		//Put the inode bid into the directory inode array
		dir.inodes[free_inode] = new_inode_bid;
		sfs_write_block(&dir, dir_bid);

		strcpy(inode.file_name, name);
	
		//Allocate a block for the first frame	
		blkid new_frame_bid = sfs_alloc_block();
		inode.first_frame = new_frame_bid;

		//Place the inode in the file descriptor array
		fd.inode = inode;
		fd.inode_bid = new_inode_bid;

		sfs_write_block(&inode, new_inode_bid);

		sfs_read_block(&inode, new_inode_bid);

		printf("FIRST FRAME: %d\n", inode.first_frame);
		printf("FILE NAME: %s\n", inode.file_name);

		//Allocate a block for the first content block of the frame
		char tmp[BLOCK_SIZE];
		sfs_read_block(tmp, new_frame_bid);
		sfs_inode_frame_t new_frame = *(sfs_inode_frame_t*)tmp;
		
		blkid new_content_bid = sfs_alloc_block();
		new_frame.content[0] = new_content_bid;

		sfs_write_block(&new_frame, new_frame_bid);
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
	fdtable[fd].valid = 0;
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
	int frame_start, frame_end, content_num;
	char *p = (char *)buf;
	char tmp[BLOCK_SIZE];
	char inode_ptr[BLOCK_SIZE];
	u32 cur = fdtable[fd].cur;
	u32 buf_offset = 0;

	sfs_inode_t inode;
	remaining = length;

	int numBids = (length / BLOCK_SIZE) + 1; 
	blkid bids[numBids];

	sfs_read_block(inode_ptr, fdtable[fd].inode_bid);
	inode = *(sfs_inode_t*)inode_ptr;

	blkid frame_bid = inode.first_frame;

	/* TODO: check if we need to resize */
	
	//Find frame holding the cursor, and the end frame
	frame_start = cur / (SFS_FRAME_COUNT * BLOCK_SIZE);
	frame_end = (cur + length) / (SFS_FRAME_COUNT * BLOCK_SIZE);
	content_num = cur / BLOCK_SIZE;
	offset = cur % BLOCK_SIZE;

	//Traverse the frames and check if frame_num exists
	sfs_inode_frame_t frame;
	int count = 0;

	while(count < frame_end){

		sfs_read_block(tmp, frame_bid);
		frame = *(sfs_inode_frame_t*)tmp;

		//If the next frame points to nothing, create another frame
		if(frame.next == 0){
			printf("sfs_write: Extending file!\n");
			blkid new_bid = sfs_alloc_block();
			frame.next = new_bid;
			sfs_write_block(&frame, frame_bid);
			frame_bid = new_bid;
		} else{
			printf("sfs_write: Checking next frame\n");
			frame_bid = frame.next;
		}

		count++;
	}

	/* TODO: get the block ids of all contents (using sfs_get_file_content() */
	int num_blocks = sfs_get_file_content(bids, fd, cur, length);
	printf("Number of content blocks: %d\n", num_blocks);

	/* TODO: main loop, go through every block, copy the necessary parts
	   to the buffer, consult the hint in the document. Do not forget to 
	   flush to the disk.
	*/

	char block_ptr[BLOCK_SIZE];
	int bid_count = 0;
	//The block numbers should already be known by this part
	while(remaining > 0){
		to_copy = 0;
		printf("\n\nBytes remaining: %d\n", remaining);
		printf("Offset: %d\n", offset);

		sfs_read_block(block_ptr, bids[bid_count]);

		//Case 1: Write to block with offset
		if(offset != 0){
			
			//Partially fill block
			if(remaining < (BLOCK_SIZE - offset)){
				to_copy = remaining;
				printf("SMALL COPY\n");
			} else{
				//Fill entire block
				to_copy = BLOCK_SIZE - offset;
				printf("BIG COPY\n");
			}

			strncpy(block_ptr+offset, buf, to_copy);

			offset = 0;
		} else if(remaining > BLOCK_SIZE){
			printf("FULL WRITE\n");
			//Case 2: Write to a full block
			to_copy = BLOCK_SIZE;
			strncpy(block_ptr, buf, to_copy);

		} else{
			//Case 3: Write a partial block
			printf("PARTIAL WRITE\n");
			to_copy = remaining;
			strncpy(block_ptr, buf, to_copy);
		}

		sfs_write_block(&block_ptr, bids[bid_count]);

		remaining = remaining - to_copy;
		cur = cur + to_copy;
		
		bid_count++;
	}

	/* TODO: update the cursor and free the temp buffer
	   for sfs_get_file_content()
	*/
	fdtable[fd].cur = cur;

	/*
	*
	*
	* Free temp buffer for sfs_get_file_content()?
	  What does this mean?
	*
	*/

	//Update size of file in inode
	inode.size = length - remaining;
	sfs_write_block(&inode, fdtable[fd].inode_bid);

	//RETURN BYTES WRITTEN
	return length - remaining;
}

/*
 * Read from an opened file. 
 * Read can not enlarge file. So you should not read outside the size of 
 * the file. If the read exceeds the file size, its result will be truncated.
 *
 * This function returns the number of bytes read.
 */
int sfs_read(int fd, void *buf, int length)
{
	printf("\n\nEntering sfs_read\n");
	int remaining, to_copy, offset;
	int frame_start, frame_end, content_blk, num_blocks;
	int i, n;
	char *p = (char *)buf;
	char inode_ptr[BLOCK_SIZE];
	char tmp[BLOCK_SIZE];
	u32 cur = fdtable[fd].cur;

	sfs_inode_t inode;
	sfs_read_block(inode_ptr, fdtable[fd].inode_bid);
	inode = *(sfs_inode_t*)inode_ptr;

	remaining = length;

	num_blocks = (length / BLOCK_SIZE) + 1; 
	blkid bids[num_blocks];

	//Find frame holding the cursor
	frame_start = cur / (SFS_FRAME_COUNT * BLOCK_SIZE);
	//Find the frame that would be holding the last byte we want to read
	frame_end = (cur + length) / (SFS_FRAME_COUNT * BLOCK_SIZE);

	int num_frames = frame_start - frame_end + 1;

	content_blk = cur / BLOCK_SIZE;
	offset = cur % BLOCK_SIZE;
printf("offset: %d\n", offset);
	/* TODO: check if we need to truncate */

	//Traverse the frames and check if the length exceeds the 
	//number of frames
	blkid frame_bid = inode.first_frame;
	sfs_inode_frame_t frame;

	int count = 0;
	int nonzero_block = 0;
	//Truncate file to the smallest BLOCK
	//The first and last blocks will have fewer bytes stored
	printf("Content block:%d\n", content_blk);
	//Check frames to determine if all content blocks exist
	while(count < num_blocks){
		printf("\nEntering loop\n");
		//Read the next frame
		sfs_read_block(tmp, frame_bid);
		frame = *(sfs_inode_frame_t*)tmp;
		frame_bid = frame.next;
		
		//Count the number of nonzero indices in the content array
		while(content_blk < SFS_FRAME_COUNT && count < num_blocks){
			printf("Looping\n");
			if(frame.content[content_blk] != 0){
				nonzero_block++;
				count++;
				content_blk++;
				printf("content[%d]: %d\n", content_blk, frame.content[content_blk]);
			}
		}

		//Reset index to zero
		content_blk = 0;

		if(frame_bid == 0 && count < num_frames){
			printf("The specified length exceeds the file size.\n");
			break;
		}
	}

	//The length exceeds the number of valid blocks, truncate read
	if(count < num_blocks){
		//Add fewer bytes from first block
		remaining = BLOCK_SIZE - offset;
		num_blocks--;

		//Add the remaining blocks
		remaining = remaining + num_blocks * BLOCK_SIZE;

	} else{
		remaining = length;
	}

	/* TODO: similar to the sfs_write() */
	int num_content_blocks = sfs_get_file_content(bids, fd, cur, length);

	int bid_count = 0;
	char block_ptr[BLOCK_SIZE];
	
	while(remaining > 0){
		to_copy = 0;
		printf("\n\nBytes remaining: %d\n", remaining);
		printf("Offset: %d\n", offset);

		sfs_read_block(block_ptr, bids[bid_count]);

		//Case 1: Read block with offset
		if(offset != 0){
			
			//Partially fill block
			if(remaining < (BLOCK_SIZE - offset)){
				to_copy = remaining;
				printf("SMALL READ\n");
			} else{
				//Fill entire block
				to_copy = BLOCK_SIZE - offset;
				printf("BIG READ\n");
			}

			strncpy(buf, block_ptr+offset, to_copy);
			offset = 0;

		} else if(remaining > BLOCK_SIZE){
			printf("FULL READ\n");
			//Case 2: Write to a full block
			to_copy = BLOCK_SIZE;
			strncpy(buf, block_ptr, to_copy);

		} else{
			//Case 3: Write a partial block
			printf("PARTIAL READ\n");
			to_copy = remaining;
			strncpy(buf, block_ptr, to_copy);
		}

		remaining = remaining - to_copy;
		cur = cur + to_copy;
		
		bid_count++;
	}

	return length - remaining;
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

	if(loc == SFS_SEEK_SET){
		fdtable[fd].cur = relative;
	} else if(loc == SFS_SEEK_CUR){
		fdtable[fd].cur = fdtable[fd].cur + relative;
	} else if(loc == SFS_SEEK_END){
		
	}
	
	return 0;
}

/*
 * Check if we reach the EOF(end-of-file).
 * 
 * This function returns 1 if it is EOF, otherwise 0.
 */
int sfs_eof(int fd)
{
	char inode_ptr[BLOCK_SIZE];
	sfs_read_block(inode_ptr, fdtable[fd].inode_bid);
	sfs_inode_t inode = *(sfs_inode_t*)inode_ptr;


	if(fdtable[fd].cur > inode.size){
		return 1;
	}

	/* TODO: check if the cursor has gone out of bound */
	return 0;
}