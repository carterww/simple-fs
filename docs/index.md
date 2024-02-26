# Notes for Simple FS
Notes for implementing the file system laid out in [this document](fs.pdf)

## Data Structures
The Simple FS will require many data structures, on disk and in memory, for managing each process's requests.
### Volume Control Block (Superblock)
The volume control block will sit on the first block of the file system and contain metadata about the file system. This includes the size of each block, the total number of blocks, the free number of blocks, and a bitmap for keeping track of free blocks. The first three attributes have obvious use cases, but the bitmap needs more explaining:
- The bitmap keeps track of which blocks are used and which are free. The bitmap is a variable sized array of bytes that takes the rest of the first block's size. Each bit corresponds to a block number. 1 represents a free block, and 0 represents an occupied block.
#### Operations
1. vcb_init(struct vcb *vcb)
    - Initializes the volume control block at the pointer. In normal use cases, this pointer should point to the the first block of disk. The block size, block count, and free block count are set to BLOCK_SIZE, BLOCK_COUNT, and BLOCK_COUNT respectively. The bitmap is then set to 0xFF at each byte to reflect that each block is free.
    - This function also initializes the spinlock that will prevent race conditions between reads/writes on the VCB. Multiple processes will need to read/write values to the VCB to reflect changes to the underlying block allocations.
2. vcb_set_block_free(struct vcb *vcb, size_t block_num, int free)
    - Updates the bitmap in the VCB by setting the associated block at "block_num" to "free." If free is a non-zero value, the bit at block_num is set to 1. If free is zero, the bit is set to 0. These operations are wrapped in a "lock" and "unlock" call to prevent multiple blocks from writing at the same time.
    - Higher level functions that allocate blocks will need their own lock in order to prevent multiple threads from thinking they own the same block. For example, two processes are creating a file of 1 block each. Both see that block 3 is free, so they both allocate block 3 for their file. Both threads write to bit 3 to indicate that block 3 is not free. If these instructions are intertwined in a certain way, two files will point to the same block.
        - The higher level lock may render the VCB spin lock useless.

## The Public API
The API provided to outside processes will consist of the following functions (inspiration from POSIX definitions):
1. int create(const char *path, size_t blocks, [mode_t mode]?);
    - Creates a file with the name "path" of size "blocks."
    - Returns an error code or ok?
    - POSIX docs for creat [here](https://linux.die.net/man/3/creat). Ours will need to act differently than theirs.
2. int open(const char *path, int oflag, ...);
    - oflag is a bitwise OR of flags.
        - Contains one of O_RDONLY, O_WRONLY, or O_RDWR (read/write).
        - Any combination of
            - O_APPEND: file offset shall always be set to end prior to writes
            - O_CREAT: creates the file
            - O_EXCL: with O_CREAT, fails if file exists
            - O_TRUNC: truncate file len to 0
    - Third arg is "mode," a bitwise OR, that sets permissions (probably N/A here).
    - Returns a file descriptor (index to the per-process open file table)
    - POSIX docs for open [here](https://linux.die.net/man/3/open)
3. int close(int filedes);
    - Closes and released resources associated with the passed in file descriptor.
    - Return error code or ok
    - POSIX docs for close [here](https://linux.die.net/man/3/close)
4. ssize_t read(int fd, char *buf, size_t buff_size);
    - Reads up to "buff_size" bytes into "buf" from file "fd."
    - Returns number of bytes read (0=EOF) or -1 on error.
        - File position is advanced by this number. Not sure if we need to implement per-process file pointer.
    - POSIX docs for read [here](https://linux.die.net/man/3/read)
5. ssize_t write(int fd, const char *buf, size_t nbytes);
    - Writes "nbytes" bytes from "buf" to the file "fd."
    - Need per-process file pointer if we don't want to write at start everytime.
    - nbytes may be larger than create() asked for, fail on this
    - Need to handle concurrent writes from processes (Reader-Writer locks?)
    - Returns the number of bytes written or -1 on error
### Our Implementation
1. void create(const char *name, size_t blocks);

2. int open(const char *name, int oflag);

3. int close(int fd);

4. ssize_t read(int fd, void *buf, size_t nbytes);

5. ssize_t write(int fd, const void *buf, size_t nbytes);

6. off_t lseek(int fd, off_t offset, int whence);
