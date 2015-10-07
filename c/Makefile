all: parser

%.o: %.c
	$(CC) -g3 -Wall -c -o $@ $+

parser: lexer.o parser.o util.o
	$(CC) -o $@ $+

lexer_test: lexer_test.o lexer.o util.o
	$(CC) -o $@ $+

clean:
	rm -f parser lexer_test *.o

test: lexer_test
	@./test.sh

.PHONY: all clean
