#include "dir.h"
#include <string.h>

/* Initialize the dentry table.
 * @param table: Table of directory entries.
 * @param nblocks: Number of blocks.
 * @return: void
 */
void dentry_table_init(struct dentry_table *table, size_t nblocks)
{
	table->num_entries = 0;
	table->curr_size = sizeof(struct dentry_table);
	table->max_size = nblocks * BLOCK_SIZE;
}

/* Add a new entry to the table
 * @param table: Table of directory entries.
 * @param entry: Directory entry.
 * @return: void
 */
int dentry_add(struct dentry_table *table, struct dentry *entry)
{
	// No space for dentry
	if (table->curr_size + sizeof(struct dentry) > table->max_size) {
		return -1;
	}
	table->curr_size += sizeof(struct dentry);
	table->entries[table->num_entries++] = *entry;
	return 0;
}

/* Get a data entry from the table
 * @param table: Table of directory entries.
 * @param file_name: Name of the file to get from the table.
 * @return: Directory entry
 */
struct dentry *dentry_get(struct dentry_table *table, const char *file_name)
{
	for (int i = 0; i < table->num_entries; ++i) {
		if (strncmp(table->entries[i].file_name, file_name,
			    MAX_FILE_NAME_LEN) == 0) {
			return &table->entries[i];
		}
	}
	return NULL;
}
