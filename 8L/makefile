CC = gcc
CFLAGS = -g -Wall -std=c99

all: 4P

#main: getFP.s

4P: asm.s eh.c prog4.c
	$(CC) $(CFLAGS) asm.s eh.c prog4.c -o 4P

clean:
	rm -f 4P
