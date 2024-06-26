#include "vcb.h"

#include <pthread.h>
#include <string.h>

#include "simple-fs.h"

static int bm_get_idx(size_t block_num, size_t *idx);

/* Initializes the VCB struct with the defined block size and block count.
 * Also initializes the free block bitmap to all 1s, indicating all blocks are
 * free.
 * @param vcb: The VCB struct to initialize. Should be on the first block
 * of the "disk."
 * @param alloc_bytes: The number of bytes allocated for the VCB. This will
 * allow the function to check if it has enough room for the bitmap.
 * @return: void
 */
void vcb_init(struct vcb *vcb, size_t alloc_bytes)
{
	vcb->block_size = BLOCK_SIZE;
	vcb->block_count = BLOCK_COUNT;
	vcb->free_block_count = BLOCK_COUNT;

	size_t num_bytes = BLOCK_COUNT / 8;
	if (BLOCK_COUNT % 8 != 0)
		++num_bytes;
	// Prevents VCB from overflowing other buff
	if (alloc_bytes < num_bytes + sizeof(struct vcb))
		return;
	memset(vcb->free_block_bm, 0xFFFF, num_bytes);
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
	size_t idx;
	if (bm_get_idx(block_num, &idx)) {
		return;
	}

	int add_to_free = free ? 1 : -1;
	vcb->free_block_count += add_to_free;

	char *byte = &vcb->free_block_bm[idx];
	int bitnum = block_num % 8;
	if (free)
		*byte |= (1 << bitnum);
	else
		*byte &= ~(1 << bitnum);
}

/* Returns whether the block at block_num is free or not.
 * @param vcb: The VCB struct to check.
 * @param block_num: The block number to check.
 * @return: -1 if the VCB is not initialized, 0 if the block is not free,
 * non-zero if the block is free.
 */
int vcb_get_block_free(struct vcb *vcb, size_t block_num)
{
	int free;
	size_t idx;
	if (bm_get_idx(block_num, &idx)) {
		return -1;
	}
	free = vcb->free_block_bm[idx] & (1 << ((block_num) % 8));
	return free;
}

/* Returns the number of free blocks in the VCB.
 * @param vcb: The VCB struct to check.
 * @return: The number of free blocks.
 */
size_t vcb_free_block_count(struct vcb *vcb)
{
	size_t cnt;
	cnt = vcb->free_block_count;
	return cnt;
}

/* Grabs a word (8 bytes) from the free block bitmap. This function is used
 * for efficient finding when finding a set of free blocks for allocation.
 * @param vcb: The VCB struct to get the word from.
 * @param idx: The byte index to start from. 0 grabs first 8 bytes, 1 grabs
 * the next 8 bytes, and so on.
 * @param word: The word to set. The function sets this value.
 * @return: The number of bytes set in the word.
 */
size_t vcb_get_bm_word(struct vcb *vcb, size_t idx, unsigned long *word)
{
	size_t max_idx = BLOCK_COUNT / 8;
	if (BLOCK_COUNT % 8 != 0)
		++max_idx;
	*word = 0;
	int i;
	for (i = 0; i < 8 && i < max_idx; ++i) {
		unsigned long byte = vcb->free_block_bm[idx + i];
		byte &= 0xFF;
		*word |= byte << (i * 8);
	}
	return i;
}

/* Gets the index for accessing the VCB's free block bitmap.
 * Checks if the block number is within the range of the bitmap.
 * @param block_num: The block number to get the index for.
 * @param idx: The index to set.
 * @return: a non-zero value if the block number is out of range, 0 otherwise.
 */
static int bm_get_idx(size_t block_num, size_t *idx)
{
	size_t max_idx = BLOCK_COUNT / 8;
	if (BLOCK_COUNT % 8 != 0)
		++max_idx;
	*idx = block_num / 8;
	if (*idx > max_idx)
		return -1;
	return 0;
}
