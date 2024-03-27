CC=gcc
CFLAGS=-Wall -g

simulation: dir.o simple-fs.o open-ft.o vcb.o main.c
	$(CC) $(CFLAGS) -o simulation dir.o simple-fs.o open-ft.o vcb.o main.c

test: dir.o simple-fs.o open-ft.o vcb.o test-primitives.c
	$(CC) $(CFLAGS) -o test dir.o simple-fs.o open-ft.o vcb.o test-primitives.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o simulation
