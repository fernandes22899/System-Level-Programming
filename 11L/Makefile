#
# Makefile for simple test of concurrent buffer
#

CC = gcc
CFLAGS = -g -Wall -std=c99 -pthread
EXES = sum

all: $(EXES)

sum: sum.c simVM.o
	$(CC) $(CFLAGS) sum.c simVM.o -o sum

simVM.o: simVM.c simVM.h
	$(CC) $(CFLAGS) -c -pthread simVM.c simVM.h

#histogram.o: histogram.c
	#	$(CC) $(CFLAGS) -c -pthread histogram.c

clean:
	-rm -f *.o $(EXES)
