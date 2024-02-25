#include "simple-fs.h"

#include <stdio.h>
#include <stdlib.h>

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
