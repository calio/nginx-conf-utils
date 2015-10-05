all: parser
.PHONY: all

parser: lexer.c parser.c
	gcc -g3 -Wall -o $@ $+

clean:
	-rm parser
