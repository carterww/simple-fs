#ifndef SIMPLE_FS_H
#define SIMPLE_FS_H

#include "stddef.h"
#include "stdio.h"

#define MAX_FILE_NAME_LEN 7

// Seek settings for lseek() passed into whence
// SFS_SEEK_SET: Set the file pointer to offset
// SFS_SEEK_CUR: Set the file pointer to the current position plus offset
// SFS_SEEK_END: Set the file pointer to the size of the file plus offset
#define SFS_SEEK_SET 0
#define SFS_SEEK_CUR 1
#define SFS_SEEK_END 2

// Blocks are 2KiB in size the FS has 512 blocks
#define BLOCK_SIZE 2048
#define BLOCK_COUNT 512

/* Raw blocks for storage. These blocks mimic a disk.
 * Block 0 will always be the VCB.
 * Block 1-2 will always be the dentry table.
 */
extern char raw_blocks[BLOCK_COUNT][BLOCK_SIZE];

void create(const char *name, size_t blocks);

int open(const char *name, int oflag);

int close(int fd);

ssize_t read(int fd, void *buf, size_t nbytes);

ssize_t write(int fd, const void *buf, size_t nbytes);

off_t lseek(int fd, off_t offset, int whence);

void init_fs();

void close_fs();

#endif // SIMPLE_FS_H
