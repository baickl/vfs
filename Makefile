all:libpak


MYLIB=libpak.a
CC = gcc
INCLUDE = .
CFLAGS = -g -Wall -ansi


libpak:PAK.o crc32.o
	$(CC) -o libpak PAK.o crc32.o

PAK.o:PAK.c PAK.h crc32.h
crc32.o:crc32.c crc32.h

clean:
	-rm PAK.o crc32.o $(MYLIB)

