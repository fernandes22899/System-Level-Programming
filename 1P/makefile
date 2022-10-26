CXX=gcc

all: utf8to16 utf16to8 utf8gen

utf8to16: utf8to16.c
	$(CXX)  -Wall utf8to16.c -o utf8to16

utf16to8: utf16to8.c
	$(CXX)  -Wall utf16to8.c -o utf16to8

utf8gen: utf8gen.c
	$(CXX)  -Wall utf8gen.c -o utf8gen

clean:
	rm -f utf8to16 utf16to8 utf8gen
