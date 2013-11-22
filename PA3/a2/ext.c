#include "ext.h"
#include <stdio.h>

static FILE *fh;

int sfs_read_block(void *buf, blkid bid)
{
	fseek(fh, bid * BLOCK_SIZE, SEEK_SET);
	fread(buf, 1, BLOCK_SIZE, fh);
	return 0;
}

int sfs_write_block(void *buf, blkid bid)
{
	fseek(fh, bid * BLOCK_SIZE, SEEK_SET);
	fwrite(buf, 1, BLOCK_SIZE, fh);
	return 0;
}

void sfs_init_storage(char *local_image_path)
{
	fh = fopen(local_image_path, "w+b");
}

void sfs_close_storage()
{
	fclose(fh);
}

void sfs_remount_storage(const char* fs_name)
{
	fh = fopen(fs_name, "r+b");
}

