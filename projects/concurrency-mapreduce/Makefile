
# An admittedly primitive Makefile
# To compile, type "make" or make "all"
# To remove files, type "make clean"

CC = gcc
CFLAGS = -g -Wall -Werror -pthread
OBJS = main.o mapreduce.o

.SUFFIXES: .c .o 

all: main

main: main.o mapreduce.o
	$(CC) $(CFLAGS) -o main main.o mapreduce.o 

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) main