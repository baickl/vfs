
CC = gcc
INCLUDE = include

WARN=-O2 -Wall -fPIC -W 
INCS=-I$(INCLUDE)
CFLAGS = $(WARN) $(INCS) 
LIB_OPTION=-shared



SRCS=src/pak.c src/crc32/crc32.c
OBJS=src/pak.o src/crc32/crc32.o

lib:lib/pak.so

lib/pak.so:$(OBJS)
	$(CC) $(CFLAGS) $(LIB_OPTION) -o lib/pak.so $(OBJS) 

PAK.o:PAK.c PAK.h crc32.h
crc32.o:crc32.c crc32.h

clean:
	rm -f lib/pak.so $(OBJS)
