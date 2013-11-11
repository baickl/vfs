
CC = gcc
AR =ar
INCLUDE = ./../include

WARN= -Wall -fPIC -W 
INCS=-I$(INCLUDE)
BIGFILES=-D_FILE_OFFSET_BITS=64
CFLAGS = $(WARN) $(INCS) $(BIGFILES) 
LIB_OPTION=-shared

OBJS_TOOLS=test.o 

all:test

test:$(OBJS_TOOLS)
	$(CC) $(CFLAGS) -o ./../bin/test $(OBJS_TOOLS) -lpak -L./../lib

objs/test.o:test.c
	$(CC) $(CFLAGS) -c test.c -o test.o 

clean:
	rm -f  ./../bin/test $(OBJS_TOOLS)
