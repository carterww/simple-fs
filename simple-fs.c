#include "simple-fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dir.h"
#include "open-ft.h"
#include "vcb.h"

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
static pthread_mutex_t vcb_lock;
static pthread_mutex_t dentry_table_lock;
static pthread_mutex_t open_file_table_lock;

/* Raw blocks for storage. These blocks mimic a disk.
 * Block 0 will always be the VCB.
 * Block 1-2 will always be the dentry table.
 */
char raw_blocks[BLOCK_COUNT][BLOCK_SIZE];

/* Create a file in the file system with the given name and number of blocks.
 * @param name: The name of the file to create. The name should be less than 64
 * characters.
 * @param blocks: The number of blocks to allocate for the file.
 * @return: void
 */
void create(const char *name, size_t blocks) {}

/* Open a file for reading and/or writing.
 * @param name: The name of the file to open.
 * @param oflag: The open flags for the file. (Unused for now)
 * @return: The file descriptor for the file, or -1 if the file could not be
 * opened.
 */
int open(const char *name, int oflag) { return 0; }

/* Closes a previously opened file.
 * @param fd: The file descriptor of the file to close.
 * @return: 0 on success, or -1 if the file could not be closed.
 */
int close(int fd) { return 0; }

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
ssize_t read(int fd, void *buf, size_t nbytes) { return 0; }

/* Write to a file at the current file offset. If the number of bytes
 * to be written is greater than the number of bytes to the end of the file,
 * an error will occur. Call lseek to set the file offset prior to writing.
 * @param fd: The file descriptor of the file to write to.
 * @param buf: The buffer to write from.
 * @param nbytes: The number of bytes to write.
 * @return: The number of bytes written or -1 if the file could not be written
 * to.
 */
ssize_t write(int fd, const void *buf, size_t nbytes) { return 0; }

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
off_t lseek(int fd, off_t offset, int whence) { return 0; }

/* Initialize the file system. This function should be called before any other
 * file system functions are called. This is not a public function like the
 * others, so processes should NOT call this function. Only the main thread 
 * (fake kernel that mounts our fs) should call this function.
 * @return: void
 */
void init_fs() {
  memset(raw_blocks, 0, sizeof(raw_blocks));

  struct vcb *vcb = (struct vcb *)raw_blocks[0];
  vcb_init(vcb, BLOCK_SIZE);
  vcb_set_block_free(vcb, 0, 0);

  struct dentry_table *table = (struct dentry_table *)raw_blocks[1];
  // Give dentry_table 2 blocks for 4KiB total
  // If we limit the len of file names to 64, we can fit minimum
  // 50 dentries in 4KB
  dentry_table_init(table, 2);
  vcb_set_block_free(vcb, 1, 0);
  vcb_set_block_free(vcb, 2, 0);

  // Open file tables are in memory (not on disk) structures so
  // don't alloc them to raw blocks
  oft_init();
}

void close_fs() {

}
