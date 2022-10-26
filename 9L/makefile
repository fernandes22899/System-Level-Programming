CC = gcc
CFLAGS = -O -g -Wall -pthread

lab9-thread: lab9-simple.o lab9-mutex.o lab9-cv.o lab9-thinlock.o
	$(CC) $(CFLAGS) lab9-simple.o lab9-mutex.c lab9-cv.c lab9-thinlock.c -o lab9-simple

clean:
	rm -f lab9-simple lab9-mutex lab9-cv lab9-thinlock
