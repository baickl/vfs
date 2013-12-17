
CC = gcc
AR =ar

WARN=-g -Wall -fPIC -W -ansi -pedantic
INCS=-I./../include -I./../src
LIBS=-lpak -lm
BIGFILES=-D_FILE_OFFSET_BITS=64
CFLAGS = $(WARN) $(INCS) $(BIGFILES) 

OBJS=pack_dir.o

all:pack_dir

pack_dir:$(OBJS)
	$(CC) $(CFLAGS) -o ./../bin/pack_dir $(OBJS) $(LIBS) -L./../lib

pack_dir.o:pack_dir.c
	$(CC) $(CFLAGS) -c pack_dir.c -o pack_dir.o 

clean:
	rm -f  ./../bin/pack_dir $(OBJS)
