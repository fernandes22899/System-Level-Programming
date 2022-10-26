#
# Makefile for simple test of concurrent buffer
#

CC = gcc
CFLAGS = -g -Wall -std=c99 -pthread
EXES = countChars

all: $(EXES)

countChars: countChars.c concurrentBuffer.o histogram.o
	$(CC) $(CFLAGS) countChars.c concurrentBuffer.o histogram.o -o countChars

concurrentBuffer.o: concurrentBuffer.c
	$(CC) $(CFLAGS) -c -pthread concurrentBuffer.c

histogram.o: histogram.c
		$(CC) $(CFLAGS) -c -pthread histogram.c

clean:
	-rm -f *.o $(EXES)
