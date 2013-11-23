
CC = gcc
AR =ar
INCLUDE = include

WARN=-O2 -Wall -fPIC -W -ansi -pedantic 
INCS=-I$(INCLUDE)
DEFINES=-D_FILE_OFFSET_BITS=64 -DBZ_NO_STDIO
CFLAGS = $(WARN) $(INCS) $(DEFINES) 

OBJS=objs/pak.o 		\
	 objs/util.o		\
	 objs/vfs.o			\
	 objs/file.o		\
	 objs/hashtable.o	\
	 objs/hashtable_itr.o		\
	 objs/hashtable_utility.o	\
	 objs/crc32.o		\
	 objs/blocksort.o	\
	 objs/huffman.o		\
	 objs/crctable.o	\
	 objs/randtable.o	\
	 objs/compress.o	\
	 objs/decompress.o	\
	 objs/bzlib.o


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
	
objs/file.o:src/file.c
	$(CC) $(CFLAGS) -c src/file.c -o objs/file.o

objs/hashtable.o:src/hashtable/hashtable.c
	$(CC) $(CFLAGS) -c src/hashtable/hashtable.c -o objs/hashtable.o

objs/hashtable_itr.o:src/hashtable/hashtable_itr.c
	$(CC) $(CFLAGS) -c src/hashtable/hashtable_itr.c -o objs/hashtable_itr.o

objs/hashtable_utility.o:src/hashtable/hashtable_utility.c
	$(CC) $(CFLAGS) -c src/hashtable/hashtable_utility.c -o objs/hashtable_utility.o

objs/crc32.o:src/crc32/crc32.c
	$(CC) $(CFLAGS) -c src/crc32/crc32.c -o objs/crc32.o

objs/blocksort.o:src/bzip2/blocksort.c
	$(CC) $(CFLAGS) -c src/bzip2/blocksort.c -o objs/blocksort.o

objs/huffman.o:src/bzip2/huffman.c	
	$(CC) $(CFLAGS) -c src/bzip2/huffman.c -o objs/huffman.o

objs/crctable.o:src/bzip2/crctable.c
	$(CC) $(CFLAGS) -c src/bzip2/crctable.c -o objs/crctable.o

objs/randtable.o:src/bzip2/randtable.c
	$(CC) $(CFLAGS) -c src/bzip2/randtable.c -o objs/randtable.o

objs/compress.o:src/bzip2/compress.c	
	$(CC) $(CFLAGS) -c src/bzip2/compress.c -o objs/compress.o

objs/decompress.o:src/bzip2/decompress.c
	$(CC) $(CFLAGS) -c src/bzip2/decompress.c -o objs/decompress.o

objs/bzlib.o:src/bzip2/bzlib.c
	$(CC) $(CFLAGS) -c src/bzip2/bzlib.c -o objs/bzlib.o

tool:
	$(MAKE) -C tools

test:
	$(MAKE) -C examples

clean:
	rm -f lib/libpak.a $(OBJS)
	rm -f bin/pack_dir
	cd tools;$(MAKE) clean
	cd examples;$(MAKE) clean
