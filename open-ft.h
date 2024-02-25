#ifndef SIMPLE_FS_OPEN_FT_H
#define SIMPLE_FS_OPEN_FT_H

#include <stddef.h>
#include <stdio.h>
#include <stdatomic.h>

struct sys_oft_entry {
  struct dentry *dentry;
  struct fcb *fcb;
  atomic_long ref_count;
};

struct proc_oft_entry {
  struct sys_oft *sys_oft;
  off_t file_pos;
};

#endif // SIMPLE_FS_OPEN_FT_H
