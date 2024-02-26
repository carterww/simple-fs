#include "simple-fs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "open-ft.h"
#include "vcb.h"
#include "dir.h"

/* Raw blocks for storage.
 * Block 0 will always be the VCB.
 */
char raw_blocks[BLOCK_COUNT][BLOCK_SIZE];

void create(const char *name, size_t blocks)
{

}

int open(const char *name, int oflag)
{
  return 0;
}

int close(int fd)
{
  return 0;
}

ssize_t read(int fd, void *buf, size_t nbytes)
{
  return 0;
}

ssize_t write(int fd, const void *buf, size_t nbytes)
{
  return 0;
}

off_t lseek(int fd, off_t offset, int whence)
{
  return 0;
}

void init_fs()
{
  // Make sure the raw blocks are zeroed out
  memset(raw_blocks, 0, sizeof(raw_blocks));
  // Initialize the VCB
  struct vcb *vcb = (struct vcb *)raw_blocks[0];
  vcb_init(vcb);
  vcb_set_block_free(vcb, 0, 0);

  // Initialize dentry_table
  struct dentry_table *table = (struct dentry_table *)raw_blocks[1];
  // Give dentry_table 2 blocks for 4KiB total
  // If we limit the len of file names to 64, we can fit min
  // 50 dentries in 4KB
  dentry_table_init(table, 2);
  vcb_set_block_free(vcb, 1, 0);
  vcb_set_block_free(vcb, 2, 0);

  // Open file tables are in memory (not on disk) structures so
  // don't alloc them to raw blocks
  sys_oft_init();

}
