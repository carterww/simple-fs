CC=gcc
CFLAGS=-Wall -g

simulation: dir.o simple-fs.o open-ft.o vcb.o main.c
	$(CC) $(CFLAGS) -o simulation dir.o simple-fs.o open-ft.o vcb.o main.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o simple-fs
