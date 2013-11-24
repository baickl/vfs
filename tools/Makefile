
CC = gcc
AR =ar

WARN=-o2 -Wall -fPIC -W -ansi -pedantic
INCS=-I./../include -I./../src
LIBS=-lpak -lm
DEFINES=-D_FILE_OFFSET_BITS=64
CFLAGS = $(WARN) $(INCS) $(DEFINES)
LIB_OPTION=-shared

OBJS_TOOLS=pack_dir.o 

all:pack_dir

pack_dir:$(OBJS_TOOLS)
	$(CC) $(CFLAGS) -o ./../bin/pack_dir $(OBJS_TOOLS) $(LIBS) -L./../lib

objs/pack_dir.o:pack_dir.c
	$(CC) $(CFLAGS) -c pack_dir.c -o pack_dir.o 

clean:
	rm -f  ./../bin/pak_dir $(OBJS_TOOLS)
