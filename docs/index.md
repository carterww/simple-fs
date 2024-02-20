# Notes for Simple FS
Notes for implementing the file system laid out in [this document](fs.pdf)

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
