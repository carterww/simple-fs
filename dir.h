#ifndef SIMPLE_FS_DIR_H
#define SIMPLE_FS_DIR_H

#include <stddef.h>
#include <stdint.h>

// Note: I think the FCB can just be placed
// at the start of the file's first data block.
// It will always be the first 8 bytes of that block.
struct fcb {
  size_t file_size;
};

#define MAX_FILE_NAME_LEN 63

struct dentry {
  size_t start_block_num;
  size_t file_size;
  uint8_t file_name_len;
  char file_name[];
};

struct dentry_table {
  size_t num_entries;
  size_t max_entries;
  struct dentry entries[];
};

void dentry_table_init(struct dentry_table *table);

void dentry_add(struct dentry_table *table, struct dentry *entry);

struct dentry *dentry_get(struct dentry_table *table, const char *file_name);

#endif // SIMPLE_FS_DIR_H
