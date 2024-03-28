#include "open-ft.h"

#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// System Open File Table
static struct sys_oft sys_oft;
// Process Open File Tables
static struct proc_ofts proc_ofts;

// TODO: These data structures should be more dynamic and robust.
// Eventually create a dynamic array scheme for these tables.

static struct sys_oft_entry *sys_oft_find(struct dentry *dentry);
static struct sys_oft_entry *sys_oft_add(struct dentry *dentry, struct fcb *fcb,
                                         int oflag);

static struct proc_oft *proc_oft_find(pid_t pid);
static struct proc_oft *proc_oft_add(pid_t pid);
static struct proc_oft_entry *
proc_oft_entry_add(struct proc_oft *oft, struct sys_oft_entry *sys_entry);
static struct proc_oft_entry * proc_oft_entry_get(struct proc_oft *oft, int fd);

/* Initializes the system open file table by alocating space for SYS_OFT_LEN
 * entries. Also initializes the process open file tables by allocating space
 * for PROC_OFTS_LEN tables.
 * @return: void
 */
void oft_init() {
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

/* Opens a file for a process. Reuses or adds an entry into the system open file
 * table. If the calling process does not have a process open file table, it is
 * created. The file is then added to the process open file table. If any tables
 * are full, -1 is returned.
 * @param dentry: The dentry of the file to open.
 * @param fcb: The file control block of the file to open.
 * @param oflag: The open flags for the file. (Unused for now)
 * @return: The index of the file in the process open file table, or -1 if the
 * file could not be opened.
 */
int oft_open(struct dentry *dentry, struct fcb *fcb, int oflag) {
  struct sys_oft_entry *entry = sys_oft_find(dentry);
  if (entry == NULL) {
    entry = sys_oft_add(dentry, fcb, oflag);
    // Out of space
    if (entry == NULL) {
      return -1;
    }
  }
  atomic_fetch_add(&entry->ref_count, 1);

  // Add to the process OFT
  pid_t caller = getpid();
  // Debug message. This may be incorrect so want to check when in testable state
  //printf("Caller %d trying to open %s", caller, dentry->file_name);
  struct proc_oft *oft = proc_oft_find(caller);
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

  return (proc_entry - oft->entries);
}

struct proc_oft_entry *oft_get(int fd) {
  pid_t caller = getpid();
  struct proc_oft *oft = proc_oft_find(caller);
  if (oft == NULL) {
    return NULL;
  }

  return proc_oft_entry_get(oft, fd);
}

/* Closes a file for a process. Decrements the reference count of the file in
 * the system open file table. If the reference count reaches 0, the file is
 * removed from the system open file table. The file is also removed from the
 * process open file table.
 * @param fd: The index of the file in the process open file table.
 * @return: 0 if the file was closed, -1 if the file could not be closed.
 */
int oft_close(int fd) { return 0; }

/* Free the system and process open file tables.
 * @return: void
 */
void oft_free() {
  // Sys OFT
  free(sys_oft.entries);

  // Proc OFTs
  for (size_t i = 0; i < proc_ofts.len; i++) {
    free(proc_ofts.ofts[i].entries);
  }
  free(proc_ofts.ofts);
}

/* Find an entry in the system open file table. Uses the file name to find the
 * entry.
 * @param dentry: The dentry of the file to find.
 * @return: The entry in the system open file table, or NULL if the file is not
 * found.
 */
static struct sys_oft_entry *sys_oft_find(struct dentry *dentry) {
  for (size_t i = 0; i < sys_oft.len; i++) {
    if (strcmp(sys_oft.entries[i].dentry->file_name, dentry->file_name) == 0) {
      return &sys_oft.entries[i];
    }
  }
  return NULL;
}

/* Add an entry to the system open file table.
 * @param dentry: The dentry of the file to add.
 * @param fcb: The file control block of the file to add.
 * @param oflag: The open flags for the file. (Unused for now)
 * @return: The entry in the system open file table, or NULL if the table is
 * full.
 */
static struct sys_oft_entry *sys_oft_add(struct dentry *dentry, struct fcb *fcb,
                                         int oflag) {
  if (sys_oft.len == sys_oft.cap) {
    return NULL;
  }

  struct sys_oft_entry *entry = &sys_oft.entries[sys_oft.len++];
  entry->dentry = dentry;
  entry->fcb = fcb;
  atomic_init(&entry->ref_count, 0);
  return entry;
}

/* Find a process open file table. A process's open file table
 * is identified by its process id.
 * @param pid: The process id of the calling process.
 * @return: The process open file table, or NULL if the table is not found.
 */
static struct proc_oft *proc_oft_find(pid_t pid) {
  for (size_t i = 0; i < proc_ofts.len; ++i) {
    if (proc_ofts.ofts[i].pid == pid) {
      return &proc_ofts.ofts[i];
    }
  }
  return NULL;
}

/* Add a process open file table to the list of process tables.
 * @param pid: The process id of the calling process.
 * @return: The process open file table, or NULL if the list of tables is full.
 */
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
  oft->entries = entries;
  oft->len = 0;
  oft->cap = PROC_OFT_LEN;
  oft->pid = pid;

  ++proc_ofts.len;
  return oft;
}

/* Add an entry to a processes's open file table.
 * @param oft: The process's open file table.
 * @param sys_entry: The entry in the system open file table.
 * @return: The entry in the process open file table, or NULL if the table is
 * full.
 */
static struct proc_oft_entry *
proc_oft_entry_add(struct proc_oft *oft, struct sys_oft_entry *sys_entry) {
  if (oft->len == oft->cap) {
    return NULL;
  }
  struct proc_oft_entry *entry = &oft->entries[oft->len++];
  entry->sys_entry = sys_entry;
  entry->file_pos = 0;
  return entry;
}

static struct proc_oft_entry * proc_oft_entry_get(struct proc_oft *oft, int fd) {
  if (fd >= oft->len)
    return NULL;
  return &oft->entries[fd];
}
