#ifndef SIMPLE_FS_H
#define SIMPLE_FS_H

#include "stddef.h"
#include "stdio.h"

#define BLOCK_SIZE 2048
#define BLOCK_COUNT 512

extern char raw_blocks[BLOCK_COUNT][BLOCK_SIZE];

void create(const char *name, size_t blocks);

int open(const char *name, int oflag);

int close(int fd);

ssize_t read(int fd, void *buf, size_t nbytes);

ssize_t write(int fd, const void *buf, size_t nbytes);

off_t lseek(int fd, off_t offset, int whence);

void init_fs();

#endif // SIMPLE_FS_H
