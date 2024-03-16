#include "dir.h"


/* Initialize the dentry table.
 * @param table: Table of directory entries.
 * @param nblocks: Number of blocks.
 * @return: void
 */
void dentry_table_init(struct dentry_table* table, size_t nblocks) {}

/* Add a new entry to the table
 * @param table: Table of directory entries.
 * @param entry: Directory entry.
 * @return: void
 */
	void dentry_add(struct dentry_table* table, struct dentry* entry) {}

/* Get a data entry from the table
 * @param table: Table of directory entries.
 * @param file_name: Name of the file to get from the table.
 * @return: Directory entry
 */
	struct dentry* dentry_get(struct dentry_table* table, const char* file_name) {}