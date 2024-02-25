#ifndef SIMPLE_FS_DIR_H
#define SIMPLE_FS_DIR_H

#include <stddef.h>

struct dentry {
  size_t start_block_num;
  size_t file_size;
  char file_name[];
};

struct dentry_table {
  size_t num_entries;
  size_t max_entries;
  struct dentry entries[];
};

void dentry_add(struct dentry_table *table, struct dentry *entry);

#endif // SIMPLE_FS_DIR_H
