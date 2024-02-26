#ifndef SIMPLE_FS_OPEN_FT_H
#define SIMPLE_FS_OPEN_FT_H

#include <stddef.h>
#include <stdio.h>
#include <stdatomic.h>
#include <unistd.h>

#include "dir.h"

// Max 32 files open system-wide
#define SYS_OFT_LEN 32
// Max 8 processes
#define PROC_OFTS_LEN 8
// Max 16 files open per process
#define PROC_OFT_LEN 16

struct sys_oft {
  struct sys_oft_entry *entries;
  size_t len;
  size_t cap;
};

struct sys_oft_entry {
  struct dentry *dentry;
  struct fcb *fcb;
  atomic_ulong ref_count;
};

struct proc_ofts {
  struct proc_oft *ofts;
  size_t len;
  size_t cap;
};

struct proc_oft {
  struct proc_oft_entry *entries;
  size_t len;
  size_t cap;
  pid_t pid;
};

struct proc_oft_entry {
  struct sys_oft_entry *sys_entry;
  off_t file_pos;
};

void sys_oft_init();

int oft_open(struct dentry *dentry, struct fcb *fcb, int oflag);

int oft_close(int fd);

void oft_free();

#endif // SIMPLE_FS_OPEN_FT_H
