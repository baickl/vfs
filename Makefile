
CC = gcc
AR =ar
INCLUDE = include

WARN=-O2 -Wall -fPIC -W 
INCS=-I$(INCLUDE)
BIGFILES=-D_FILE_OFFSET_BITS=64
CFLAGS = $(WARN) $(INCS) $(BIGFILES) 
LIB_OPTION=-shared

OBJS=objs/pak.o 				\
	 objs/util.o				\
	 objs/vfs.o					\
	 objs/crc32/crc32.o			\
	 objs/bzip2/blocksort.o		\
	 objs/bzip2/huffman.o		\
	 objs/bzip2/crctable.o		\
	 objs/bzip2/randtable.o		\
	 objs/bzip2/compress.o		\
	 objs/bzip2/decompress.o	\
	 objs/bzip2/bzlib.o


all:lib tool test

lib:$(OBJS)
	rm -f lib/libpak.a
	$(AR) cq lib/libpak.a $(OBJS) 

objs/pak.o:src/pak.c
	$(CC) $(CFLAGS) -c src/pak.c -o objs/pak.o
objs/util.o:src/util.c
	$(CC) $(CFLAGS) -c src/util.c -o objs/util.o
objs/vfs.o:src/vfs.c
	$(CC) $(CFLAGS) -c src/vfs.c -o objs/vfs.o
objs/crc32/crc32.o:src/crc32/crc32.c
	$(CC) $(CFLAGS) -c src/crc32/crc32.c -o objs/crc32/crc32.o
objs/bzip2/blocksort.o:src/bzip2/blocksort.c
	$(CC) $(CFLAGS) -c src/bzip2/blocksort.c -o objs/bzip2/blocksort.o
objs/bzip2/huffman.o:src/bzip2/huffman.c	
	$(CC) $(CFLAGS) -c src/bzip2/huffman.c -o objs/bzip2/huffman.o
objs/bzip2/crctable.o:src/bzip2/crctable.c
	$(CC) $(CFLAGS) -c src/bzip2/crctable.c -o objs/bzip2/crctable.o
objs/bzip2/randtable.o:src/bzip2/randtable.c
	$(CC) $(CFLAGS) -c src/bzip2/randtable.c -o objs/bzip2/randtable.o
objs/bzip2/compress.o:src/bzip2/compress.c	
	$(CC) $(CFLAGS) -c src/bzip2/compress.c -o objs/bzip2/compress.o
objs/bzip2/decompress.o:src/bzip2/decompress.c
	$(CC) $(CFLAGS) -c src/bzip2/decompress.c -o objs/bzip2/decompress.o
objs/bzip2/bzlib.o:src/bzip2/bzlib.c
	$(CC) $(CFLAGS) -c src/bzip2/bzlib.c -o objs/bzip2/bzlib.o

tool:
	$(MAKE) -C tools

test:
	$(MAKE) -C examples

clean:
	rm -f lib/libpak.a $(OBJS)
	cd tools;$(MAKE) clean
	cd examples;$(MAKE) clean
