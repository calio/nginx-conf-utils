all: parser
.PHONY: all

%.o: %.c
	$(CC) -g3 -Wall -c -o $@ $+

parser: lexer.o parser.o
	$(CC) -o $@ $+

clean:
	-rm parser *.o
