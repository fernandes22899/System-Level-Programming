CC = gcc
CFLAGS = -g -Wall -std=c99

all: lab7

#main: getFP.s

lab7: getFP.s lab7.c framecount.c
	$(CC) $(CFLAGS) getFP.s lab7.c framecount.c -o lab7

clean:
	rm -f lab7
