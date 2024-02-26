#include "vcb.h"

#include <pthread.h>
#include <string.h>

#include "simple-fs.h"

// VCB flags
// VCB_INIT_FLAG: VCB has been initialized
#define VCB_INIT_FLAG 1
static uint64_t vcb_flags = 0;

// Lock for accessing the VCB
static pthread_spinlock_t vcb_lock;

/* Initializes the VCB struct with the defined block size and block count.
 * Also initializes the free block bitmap to all 1s, indicating all blocks are
 * free.
 * @param vcb: The VCB struct to initialize. Should be on the first block
 * of the "disk."
 * @return: void
 */
void vcb_init(struct vcb *vcb)
{
  if (VCB_INIT_FLAG & vcb_flags)
    return;
  vcb->block_size = BLOCK_SIZE;
  vcb->block_count = BLOCK_COUNT;
  vcb->free_block_count = BLOCK_COUNT;

  size_t num_bytes = BLOCK_COUNT / 8;
  if (BLOCK_COUNT % 8 != 0)
    ++num_bytes;
  memset(vcb->free_block_bm, 0xFFFF, num_bytes);

  pthread_spin_init(&vcb_lock, 0);
  vcb_flags |= VCB_INIT_FLAG;
}

/* Sets the block at block_num to free or not free in the VCB's free block
 * bitmap.
 * @param vcb: The VCB struct to modify.
 * @param block_num: The block number to set free or not free.
 * @param free: 0 to set the block to not free, otherwise set to free.
 * @return: void
 */
void vcb_set_block_free(struct vcb *vcb, size_t block_num, int free)
{
  if (VCB_INIT_FLAG & vcb_flags)
    return;
  pthread_spin_lock(&vcb_lock);

  int add_to_free = free ? 1 : -1;
  vcb->free_block_count += add_to_free;

  char *byte = &vcb->free_block_bm[block_num / 8];
  int bitnum = block_num % 8;
  if (free)
    *byte |= (1 << bitnum);
  else
    *byte &= ~(1 << bitnum);

  pthread_spin_unlock(&vcb_lock);
}

/* Returns whether the block at block_num is free or not.
 * @param vcb: The VCB struct to check.
 * @param block_num: The block number to check.
 * @return: -1 if the VCB is not initialized, 0 if the block is not free,
 * 1 if the block is free.
 */
int vcb_get_block_free(struct vcb *vcb, size_t block_num)
{
  if (VCB_INIT_FLAG & vcb_flags)
    return -1;
  int free;
  pthread_spin_lock(&vcb_lock);
  free = vcb->free_block_bm[block_num / 8]
    & (1 << (block_num) % 8);
  pthread_spin_unlock(&vcb_lock);
  return free;
}

/* Returns the number of free blocks in the VCB.
 * @param vcb: The VCB struct to check.
 * @return: The number of free blocks.
 */
size_t vcb_free_block_count(struct vcb *vcb)
{
  if (VCB_INIT_FLAG & vcb_flags)
    return 0;
  size_t cnt;
  // Probably don't need this, on 64 bit systems
  pthread_spin_lock(&vcb_lock);
  cnt = vcb->free_block_count;
  pthread_spin_unlock(&vcb_lock);
  return cnt;
}
