#ifndef SIMPLE_FS_DIR_H
#define SIMPLE_FS_DIR_H

#include "simple-fs.h"
#include <stddef.h>
#include <stdint.h>

// Note: I think the FCB can just be placed
// at the start of the file's first data block.
// It will always be the first 16 bytes of that block.

// File control block which details the state of the file.
struct fcb {
  size_t start_block_num;
  size_t file_size;
};

// Directory entry. Details the file's name and starting block number.
struct dentry {
  size_t start_block_num;
  size_t file_size;
  char file_name[MAX_FILE_NAME_LEN];
};

// Table of directory entries. Used for looking up files in the file system.
// The table is stored on blocks 1-2 of the file system.
struct dentry_table {
  size_t num_entries;
  size_t curr_size;
  struct dentry entries[];
};

void dentry_table_init(struct dentry_table *table, size_t nblocks);

void dentry_add(struct dentry_table *table, struct dentry *entry);

struct dentry *dentry_get(struct dentry_table *table, const char *file_name);

#endif // SIMPLE_FS_DIR_H
