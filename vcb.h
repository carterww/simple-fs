#ifndef SIMPLE_FS_VCB_H
#define SIMPLE_FS_VCB_H

#include <stdint.h>
#include <stddef.h>

#define BLOCK_SIZE 2048
#define BLOCK_COUNT 512

// Use rest of the VCB block for storing the bitmap.
#define REST_OF_BLOCK (BLOCK_SIZE - \
    ((3 * sizeof(size_t)) + sizeof(uint64_t)))

#define VCB_INIT_FLAG 1

struct vcb {
  size_t block_size;
  size_t block_count;
  size_t free_block_count;
  uint64_t flags;

  char free_block_bm[REST_OF_BLOCK];
};

void vcb_init(struct vcb *vcb);

void vcb_set_block_free(struct vcb *vcb, size_t block_num, int free);

int vcb_get_block_free(struct vcb *vcb, size_t block_num);

size_t vcb_free_block_count(struct vcb *vcb);

#endif // SIMPLE_FS_VCB_H
