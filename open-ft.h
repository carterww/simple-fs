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
// Max 32 files open per process
#define PROC_OFT_LEN 32

// System-wide open file table. Tracks all open files across the FS.
struct sys_oft {
  struct sys_oft_entry *entries;
  size_t len;
  size_t cap;
};

// Entry into the system-wide open file table. Tracks the file's dentry and FCB.
// A process's open file table will have a reference to this entry.
// The ref_count is used to track how many processes have the file open.
struct sys_oft_entry {
  struct dentry *dentry;
  struct fcb *fcb;
  atomic_ulong ref_count;
};

// Process open file tables. Holds all the open file talbes for all processes.
struct proc_ofts {
  struct proc_oft *ofts;
  size_t len;
  size_t cap;
};

// A process's open file table. Tracks all the files a process has open.
// Processes are identified by their PID.
struct proc_oft {
  struct proc_oft_entry *entries;
  size_t len;
  size_t cap;
  pid_t pid;
};

// Entry into the process's open file table.
// Tracks the system-wide open file table entry and the file's position.
struct proc_oft_entry {
  struct sys_oft_entry *sys_entry;
  off_t file_pos;
};

void oft_init();

int oft_open(struct dentry *dentry, struct fcb *fcb, int oflag);

struct proc_oft_entry *oft_get(int fd);

int oft_close(int fd);

void oft_free();

#endif // SIMPLE_FS_OPEN_FT_H
