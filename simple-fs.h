#ifndef SIMPLE_FS_H
#define SIMPLE_FS_H

#define BLOCK_SIZE 2048
#define BLOCK_COUNT 512

extern char raw_blocks[BLOCK_COUNT][BLOCK_SIZE];

#endif // SIMPLE_FS_H
