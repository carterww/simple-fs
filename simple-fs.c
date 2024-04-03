#include "simple-fs.h"

#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dir.h"
#include "open-ft.h"
#include "vcb.h"

#define FIRST_DATA_BLOCK_IDX 3

// NOT PERMANENT: These locks are just an idea for now
// Will have to see how it plays out after implementing these
// To prevent implementing a complex lock system, we should just have
// one lock for each DS. Each FS call (open, create, read, etc.) can
// lock the DS it needs in a specific order. This will prevent deadlocks
// and will be easier to implement. The downside is that it will be much slower
// if these DS are only being used for short periods of time in the functions.
// If it gets too unwieldy, a simple solution would be to give each process OFT
// their own lock. This will allow multiple processes to access their OFT and
// the system OFT (read only) at the same time.

/* Locking Scheme:
 * 1. vcb_lock
 * 2. dentry_table_lock
 * 3. open_file_table_lock
 * Unlock in reverse order
 */
static pthread_mutex_t vcb_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t dentry_table_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t open_file_table_lock = PTHREAD_MUTEX_INITIALIZER;

inline void lock_all();

void lock_all() {
  pthread_mutex_lock(&vcb_lock);
  pthread_mutex_lock(&dentry_table_lock);
  pthread_mutex_lock(&open_file_table_lock);
}

inline void unlock_all();

void unlock_all() {
  pthread_mutex_unlock(&open_file_table_lock);
  pthread_mutex_unlock(&dentry_table_lock);
  pthread_mutex_unlock(&vcb_lock);
}

static int find_free_blocks(size_t *start, size_t blocks);

// TODO: Maybe extern these in impl files so
// vcb and dentry don't have to be passed around
struct vcb *vcb = NULL;
struct dentry_table *dentry_table = NULL;

/* Raw blocks for storage. These blocks mimic a disk.
 * Block 0 will always be the VCB.
 * Block 1-2 will always be the dentry table.
 */
char raw_blocks[BLOCK_COUNT][BLOCK_SIZE];

/* Create a file in the file system with the given name and number of blocks.
 * @param name: The name of the file to create. The name should be less than 7
 * characters.
 * @param blocks: The number of blocks to allocate for the file.
 * @return: void
 */
void create(const char *name, size_t blocks) {
  lock_all();

  size_t start;
  if (find_free_blocks(&start, blocks)) {
    // No space for file
    return;
  }

  // Mark blocks as used
  for (int j = start; j < start + blocks; ++j) {
    // This alters free block count in VCB
    vcb_set_block_free(vcb, j, 0);
  }

  // Add entry in dentry table
  struct dentry entry = {
    .start_block_num = start,
    .file_size = blocks,
  };
  strncpy(entry.file_name, name, MAX_FILE_NAME_LEN);
  // Ensure null-terminated string
  entry.file_name[MAX_FILE_NAME_LEN - 1] = '\0';
  dentry_add(dentry_table, &entry);

  // Initialize FCB
  char *block = raw_blocks[start];
  struct fcb *fcb = (struct fcb *)block;
  fcb->file_size = blocks;
  fcb->start_block_num = start;

  unlock_all();

  return;
}

/* Open a file for reading and/or writing.
 * @param name: The name of the file to open.
 * @param oflag: The open flags for the file. (Unused for now)
 * @return: The file descriptor for the file, or -1 if the file could not be
 * opened.
 */
int open(const char *name, int oflag) {
  lock_all();
  struct dentry *entry = dentry_get(dentry_table, name);
  unlock_all();
  if (entry == NULL) {
    return -1;
  }
  struct fcb *file_fcb = (struct fcb*)raw_blocks[entry->start_block_num];
  return oft_open(entry, file_fcb, 0);
}

/* Closes a previously opened file.
 * @param fd: The file descriptor of the file to close.
 * @return: 0 on success, or -1 if the file could not be closed.
 */
int close(int fd) { 
  lock_all();
  int res = oft_close(fd);
  unlock_all();
  return res;
}

/* Read from a file at the current file offset. If the file offset is at the end
 * of the file, no bytes will be read. Call lseek to set the file offset prior
 * to reading.
 * @param fd: The file descriptor of the file to read from.
 * @param buf: The buffer to read into.
 * @param nbytes: The number of bytes to read. The buffer should be at least
 * this size.
 * @return: The number of bytes read, -1 if the file could not be read, or EOF
 * if the file offset is at the end of the file after the read.
 */
ssize_t read(int fd, void *buf, size_t nbytes) { 
  lock_all();

	struct proc_oft_entry *entry = oft_get(fd);
	if (entry == NULL || buf == NULL) {
		return -1;
  }
  struct fcb *fcb = entry->sys_entry->fcb;
  // Make sure we don't read past the end of the file
  size_t max_file_size = fcb->file_size * BLOCK_SIZE;
  if (nbytes > max_file_size - entry->file_pos) {
    nbytes = max_file_size - entry->file_pos;
  }
	
  // Get current file position
  off_t current_pos = entry->file_pos;
	//Calculate the starting block and offset within that block
	size_t block_idx = fcb->start_block_num;
	size_t offset = current_pos % BLOCK_SIZE;
	ssize_t bytes_read = 0;
	
	while (bytes_read < nbytes){
		size_t current_block = block_idx;
		size_t remaining_bytes = nbytes - bytes_read;
		size_t read_size = remaining_bytes;

		memcpy(buf + bytes_read, &raw_blocks[current_block][offset], read_size);

		bytes_read += read_size;

		current_pos += read_size;
		block_idx = (current_pos / BLOCK_SIZE) + fcb->start_block_num;
		offset = current_pos % BLOCK_SIZE;

		if (read_size == 0)
		  break;
	}

  // Update file position
  entry->file_pos = current_pos;

  unlock_all();
	return bytes_read;
}

/* Write to a file at the current file offset. If the number of bytes
 * to be written is greater than the number of bytes to the end of the file,
 * an error will occur. Call lseek to set the file offset prior to writing.
 * @param fd: The file descriptor of the file to write to.
 * @param buf: The buffer to write from.
 * @param nbytes: The number of bytes to write.
 * @return: The number of bytes written or -1 if the file could not be written
 * to.
 */
ssize_t write(int fd, const void *buf, size_t nbytes) {
  lock_all();

	struct proc_oft_entry *entry = oft_get(fd);
	if(entry == NULL || buf == NULL) {
		return -1;
  }

  struct fcb *fcb = entry->sys_entry->fcb;
  size_t max_file_size = fcb->file_size * BLOCK_SIZE;
  // Check if we have enough room to write
  if (max_file_size - entry->file_pos < nbytes) {
    return -1;
  }

  off_t current_pos = entry->file_pos;

	size_t block_idx = fcb->start_block_num;
	size_t offset = current_pos % BLOCK_SIZE;
	ssize_t bytes_written = 0;

	while (bytes_written < nbytes){
		size_t current_block = block_idx;
		size_t remaining_space = BLOCK_SIZE - offset;
		size_t remaining_bytes = nbytes - bytes_written;
		size_t write_size = remaining_space < remaining_bytes ? remaining_space : remaining_bytes;

		//Write data to current block
		memcpy(&raw_blocks[current_block][offset], buf + bytes_written, write_size);

		bytes_written += write_size;
		current_pos += write_size; //updating file position
		block_idx = (current_pos / BLOCK_SIZE) + fcb->start_block_num; //updating block index
		offset = current_pos % BLOCK_SIZE; //updating offset within the block
	}

  // Update file position
  entry->file_pos = current_pos;

  unlock_all();
	return bytes_written;
}

/* Set the file offset for a file in number of bytes from the beginning, the
 * current file offset, or the end of the file.
 * @param fd: The file descriptor of the file to set the offset for.
 * @param offset: The offset to set.
 * @param whence: The base for the offset. SFS_SEEK_SET for the beginning of the
 * file, SFS_SEEK_CUR for the current file offset, or SFS_SEEK_END for the end
 * of the file.
 * @return: The new file offset from the beginning of the file, or -1 if the
 * file offset could not be set.
 */
off_t lseek(int fd, off_t offset, int whence) {
  lock_all();

  struct proc_oft_entry *entry = oft_get(fd);
  if (entry == NULL)
    return -1;
  switch (whence) {
  case SFS_SEEK_CUR:
    entry->file_pos += offset;
  case SFS_SEEK_SET:
    entry->file_pos = offset;
    break;
  case SFS_SEEK_END:
    size_t bytes = entry->sys_entry->fcb->file_size * BLOCK_SIZE;
    --bytes; // offset of 0 should put at last byte
    entry->file_pos = bytes - offset;
  default:
    return -1;
  }
  // Ensure file position is within bounds
  if (entry->file_pos < sizeof(struct fcb)) {
    entry->file_pos = sizeof(struct fcb);
  } else if (entry->file_pos > entry->sys_entry->fcb->file_size * BLOCK_SIZE) {
    entry->file_pos = entry->sys_entry->fcb->file_size * BLOCK_SIZE;
  }

  unlock_all();
  return entry->file_pos;
}

/* Initialize the file system. This function should be called before any other
 * file system functions are called. This is not a public function like the
 * others, so processes should NOT call this function. Only the main thread 
 * (fake kernel that mounts our fs) should call this function.
 * @return: void
 */
void init_fs() {
  memset(raw_blocks, 0, sizeof(raw_blocks));

  vcb = (struct vcb *)raw_blocks[0];
  vcb_init(vcb, BLOCK_SIZE);
  vcb_set_block_free(vcb, 0, 0);

  dentry_table = (struct dentry_table *)raw_blocks[1];
  // Give dentry_table 2 blocks for 4KiB total
  dentry_table_init(dentry_table, 2);
  vcb_set_block_free(vcb, 1, 0);
  vcb_set_block_free(vcb, 2, 0);

  // Open file tables are in memory (not on disk) structures so
  // don't alloc them to raw blocks
  oft_init();
}

/* Finds a free set of contiguous blocks for a file.
 * @param start: The starting block number of the free blocks.
 * This value will be set by the function to the file's starting block.
 * @param blocks: The number of blocks to allocate.
 * @return: 0 if a free set of blocks was found, -1 if no free blocks were found.
 */
static int find_free_blocks(size_t *start, size_t blocks) {
  *start = FIRST_DATA_BLOCK_IDX;
  unsigned long word;
  // Traverse blocks to find first fit
  size_t i = 0;
  while (i < BLOCK_COUNT) {
    size_t num_bytes = vcb_get_bm_word(vcb, i / 64, &word);
    for (int j = 0; j < sizeof(unsigned long) * num_bytes; ++j, ++i) {
      // If word is 0 at spot, then block is not free, continue
      if (!(word & (1 << j))) {
        *start = i + 1;
        continue;
      }
      if (i - *start + 1 == blocks) {
        goto out_while; // Found a fit
      }
    }
  }
out_while:
  if (i >= BLOCK_COUNT) {
    // No space for file
    return -1;
  }
  return 0;
}
