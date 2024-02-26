#include "open-ft.h"

#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct sys_oft sys_oft;
static struct proc_ofts proc_ofts;

// TODO: Need to update the add functions to reuse previously used
// entries if they are free. Right now, it just adds to the end of the array
// then fails if the array is full.

static struct sys_oft_entry *sys_oft_find(struct dentry *dentry);
static struct sys_oft_entry *sys_oft_add(struct dentry *dentry, struct fcb *fcb,
                                         int oflag);

static struct proc_oft *proc_oft_find(pid_t pid);
static struct proc_oft *proc_oft_add(pid_t pid);
static struct proc_oft_entry *
proc_oft_entry_add(struct proc_oft *oft, struct sys_oft_entry *sys_entry);

void sys_oft_init() {
  // Make the space static for now,
  // we can change this to a dynamic array scheme later
  sys_oft.entries = malloc(sizeof(struct sys_oft_entry) * SYS_OFT_LEN);
  if (sys_oft.entries == NULL) {
    perror("malloc");
    exit(1);
  }
  memset(sys_oft.entries, 0, sizeof(struct sys_oft_entry) * SYS_OFT_LEN);
  sys_oft.cap = SYS_OFT_LEN;
  sys_oft.len = 0;

  proc_ofts.ofts = malloc(sizeof(struct proc_oft) * PROC_OFTS_LEN);
  if (proc_ofts.ofts == NULL) {
    perror("malloc");
    exit(1);
  }
  memset(proc_ofts.ofts, 0, sizeof(struct proc_oft) * PROC_OFTS_LEN);
  proc_ofts.cap = PROC_OFT_LEN;
  proc_ofts.len = 0;
}

int oft_open(struct dentry *dentry, struct fcb *fcb, int oflag) {
  // Check if file is open in the system OFT
  struct sys_oft_entry *entry = sys_oft_find(dentry);
  if (entry == NULL) {
    // File is not open in the system OFT
    entry = sys_oft_add(dentry, fcb, oflag);
    // Out of space
    if (entry == NULL) {
      return -1;
    }
  }
  atomic_fetch_add(&entry->ref_count, 1);

  // Add to the process OFT
  pid_t caller = getpid();
  struct proc_oft *oft = proc_oft_find(caller);
  // If the process OFT does not exist, create it
  if (oft == NULL) {
    oft = proc_oft_add(caller);
    if (oft == NULL) {
      // Out of space
      return -1;
    }
  }
  struct proc_oft_entry *proc_entry = proc_oft_entry_add(oft, entry);
  if (proc_entry == NULL) {
    // Out of space
    return -1;
  }

  // return the index of the file in the process OFT
  return (proc_entry - oft->entries);
}

int oft_close(int fd) { return 0; }

void oft_free() {
  // Sys OFT
  free(sys_oft.entries);

  // Proc OFTs
  for (size_t i = 0; i < proc_ofts.len; i++) {
    free(proc_ofts.ofts[i].entries);
  }
  free(proc_ofts.ofts);
}

static struct sys_oft_entry *sys_oft_find(struct dentry *dentry) {
  for (size_t i = 0; i < sys_oft.len; i++) {
    if (strcmp(sys_oft.entries[i].dentry->file_name, dentry->file_name) == 0) {
      return &sys_oft.entries[i];
    }
  }
  return NULL;
}

static struct sys_oft_entry *sys_oft_add(struct dentry *dentry, struct fcb *fcb,
                                         int oflag) {
  if (sys_oft.len == sys_oft.cap) {
    return NULL;
  }

  struct sys_oft_entry *entry = &sys_oft.entries[sys_oft.len];
  entry->dentry = dentry;
  entry->fcb = fcb;
  ++sys_oft.len;
  atomic_init(&entry->ref_count, 0);
  return entry;
}

static struct proc_oft *proc_oft_find(pid_t pid) {
  for (size_t i = 0; i < proc_ofts.len; i++) {
    if (proc_ofts.ofts[i].pid == pid) {
      return &proc_ofts.ofts[i];
    }
  }
  return NULL;
}

static struct proc_oft *proc_oft_add(pid_t pid) {
  if (proc_ofts.len == proc_ofts.cap) {
    return NULL;
  }
  struct proc_oft *oft = &proc_ofts.ofts[proc_ofts.len];
  struct proc_oft_entry *entries =
      malloc(sizeof(struct proc_oft_entry) * PROC_OFT_LEN);
  if (entries == NULL) {
    perror("malloc");
    exit(1);
  }
  oft->pid = pid;
  oft->entries = entries;
  oft->cap = PROC_OFT_LEN;

  ++proc_ofts.len;
  return oft;
}

static struct proc_oft_entry *
proc_oft_entry_add(struct proc_oft *oft, struct sys_oft_entry *sys_entry)
{
  if (oft->len == oft->cap) {
    return NULL;
  }
  struct proc_oft_entry *entry = &oft->entries[oft->len];
  entry->sys_entry = sys_entry;
  entry->file_pos = 0;
  ++oft->len;
  return entry;
}
