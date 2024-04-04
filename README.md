# CS 445 In-Memory File System
## Michael Wittkopf and Carter Williams
This project is a simple in-memory file system that mimics the POSIX file system calls creat(), open(), close(), read(), write(), and lseek().
The file system, however, is much simpler than a typical file system, so the POSIX calls are not completely implemented and altered slightly. 
These calls are declared in "simple-fs.h" and defined in "simple-fs.c." Comments regarding each function's purpose and implementation (for all functions)
are included above their respective definitions. There is also a webpage for this project that comments on the POSIX calls and what their parameters,
return values, and purposes are. This webpage can be found [here](https://carterww.github.io/simple-fs/).


To run the program, you must be on a Linux system. After unzipping the containing folder to the location of your choosing, using a terminal, CD to the
folder containing the makefile. From here, you can type "make" in the command line. This will compile the file and create a file called simulation. From
here, you can type "./simulation". This will run the main.c program and call the functions that will perform all required operations.

First, the file system will be initialized, followed by the creation of the first pthread, which will call the p1_thread (P1). P1 will create file 1, write 
"hello", then close file 1. From there, P1 will create file 2, write "world", then close it. The first pthread will be joined, and the second pthread will 
be created (P2). This will open file 1, read and print it to the screen, along with the words "file1", then close file 1. Pthread 3 will then be created 
(P3), which will open file 2, read and print its contents to the screen, along with the words "file2", then close file 2. At this point, pthreads 2 and 3 
will both be joined and the program will exit.


All processes were completed as tasked, with none left out. These included create(), open(), close(), read() and write(). These are all contained in the
simple-fs.c file.