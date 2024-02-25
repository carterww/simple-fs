#include "vcb.h"

#include <pthread.h>
#include <string.h>

#include "simple-fs.h"

static pthread_spinlock_t vcb_lock;

void vcb_init(struct vcb *vcb)
{
  // Already initialized
  if (VCB_INIT_FLAG & vcb->flags)
    return;
  vcb->block_size = BLOCK_SIZE;
  vcb->block_count = BLOCK_COUNT;
  vcb->free_block_count = BLOCK_COUNT;

  size_t num_bytes = BLOCK_COUNT / 8;
  if (BLOCK_COUNT % 8 != 0)
    ++num_bytes;
  memset(vcb->free_block_bm, 0, num_bytes);

  pthread_spin_init(&vcb_lock, 0);
  vcb->flags |= VCB_INIT_FLAG;
}

void vcb_set_block_free(struct vcb *vcb, size_t block_num, int free)
{
  if (VCB_INIT_FLAG & vcb->flags)
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

int vcb_get_block_free(struct vcb *vcb, size_t block_num)
{
  if (VCB_INIT_FLAG & vcb->flags)
    return -1;
  int free;
  pthread_spin_lock(&vcb_lock);
  free = vcb->free_block_bm[block_num / 8]
    & (1 << (block_num) % 8);
  pthread_spin_unlock(&vcb_lock);
  return free;
}

size_t vcb_free_block_count(struct vcb *vcb)
{
  if (VCB_INIT_FLAG & vcb->flags)
    return 0;
  size_t cnt;
  // Probably don't need this, on 64 bit systems
  pthread_spin_lock(&vcb_lock);
  cnt = vcb->free_block_count;
  pthread_spin_unlock(&vcb_lock);
  return cnt;
}
