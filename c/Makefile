all: ncfp

%.o: %.c token.h parser.h lexer.h util.h
	$(CC) -g3 -Wall -c -o $@ $<

ncfp: lexer.o parser.o util.o main.o
	$(CC) -o $@ $+

lexer_test: lexer_test.o lexer.o util.o
	$(CC) -o $@ $+

parser_test: parser_test.o lexer.o parser.o util.o
	$(CC) -o $@ $+

clean:
	rm -f parser lexer_test *.o

test: lexer_test parser_test
	@./test.sh

.PHONY: all clean test
