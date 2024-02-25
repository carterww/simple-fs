#ifndef SIMPLE_FS_VCB_H
#define SIMPLE_FS_VCB_H

#include <stdint.h>
#include <stddef.h>

#define VCB_INIT_FLAG 1

struct vcb {
  size_t block_size;
  size_t block_count;
  size_t free_block_count;
  uint64_t flags;

  // Block bitmap just takes rest of the block
  char free_block_bm[];
};

void vcb_init(struct vcb *vcb);

void vcb_set_block_free(struct vcb *vcb, size_t block_num, int free);

int vcb_get_block_free(struct vcb *vcb, size_t block_num);

size_t vcb_free_block_count(struct vcb *vcb);

#endif // SIMPLE_FS_VCB_H