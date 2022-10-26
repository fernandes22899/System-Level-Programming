#
# This is a simple Makefile for building the lab5 test programs.
#
all: lab5 test1.o test2.o test1 test2

test1: test1.o symtab.o
	gcc -g test1.o symtab.o -o test1

test2: test2.o symtab.o
	gcc -g test2.o symtab.o -o test2

lab5: main.o symtab.o
	gcc -g main.o symtab.o -o lab5

test1.o: test1.c symtab.h
	gcc -Wall -std=c99 -c -g test1.c

test2.o: test2.c symtab.h
	gcc -Wall -std=c99 -c -g test2.c

main.o: main.c symtab.h
	gcc -Wall -std=c99 -c -g main.c

symtab.o: symtab.c symtab.h
	gcc -Wall -std=c99 -c -g symtab.c

clean:
	rm -f main.o symtab.o lab5 test1.o test2.o test1 test2
