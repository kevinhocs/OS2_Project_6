CC=gcc
CFLAGS=-Wall -Wextra -Werror -DCTEST_ENABLE

all: libvvsfs.a testfs

libvvsfs.a: image.o block.o free.o inode.o pack.o
	ar rcs libvvsfs.a image.o block.o free.o inode.o pack.o

testfs: testfs.o libvvsfs.a
	$(CC) $(CFLAGS) -o testfs testfs.o -L. -lvvsfs

image.o: image.c image.h
	$(CC) $(CFLAGS) -c image.c

block.o: block.c block.h image.h
	$(CC) $(CFLAGS) -c block.c

testfs.o: testfs.c image.h block.h free.h inode.h
	$(CC) $(CFLAGS) -c testfs.c

free.o: free.c free.h
	$(CC) $(CFLAGS) -c free.c

inode.o: inode.c inode.h block.h free.h
	$(CC) $(CFLAGS) -c inode.c

pack.o: pack.c pack.h
	$(CC) $(CFLAGS) -c pack.c

test: testfs
	./testfs

.PHONY: test clean pristine

clean:
	rm -f *.o

pristine:
	rm -f *.o *.a testfs disk.img