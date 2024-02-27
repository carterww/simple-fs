#ifndef SIMPLE_FS_VCB_H
#define SIMPLE_FS_VCB_H

#include <stdint.h>
#include <stddef.h>


// Volume control block. Details the state of the file system.
// Should be on block 0 of the file system.
struct vcb {
  size_t block_size;
  size_t block_count;
  size_t free_block_count;

  // Block bitmap just takes rest of the block
  char free_block_bm[];
};

void vcb_init(struct vcb *vcb, size_t alloc_bytes);

void vcb_set_block_free(struct vcb *vcb, size_t block_num, int free);

int vcb_get_block_free(struct vcb *vcb, size_t block_num);

size_t vcb_free_block_count(struct vcb *vcb);

#endif // SIMPLE_FS_VCB_H
