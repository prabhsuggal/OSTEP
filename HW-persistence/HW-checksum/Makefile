# An admittedly primitive Makefile
# To compile, type "make" or make "all"
# To remove files, type "make clean"

CC = gcc
CFLAGS = -g -Wall -Werror
OBJS = check-xor check-fletcher check-crc

.SUFFIXES: .c .o

all: ${OBJS}

%:%.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	-rm -f $(OBJS)