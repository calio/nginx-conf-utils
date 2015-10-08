
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lexer.h"
#include "parser.h"
#include "util.h"

int main(int argc, char **argv)
{
    lexer_t    lexer;

    if (argc < 2) {
        printf("Usage: %s file\n", argv[0]);
        return 0;
    }

    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        printf("Can't open file %s\n", argv[1]);
        return -1;
    }

    lexer_init(&lexer, f);
    lexer_skip(&lexer, 1, 1);

    parse(&lexer, f);

    lexer_free(&lexer);
    fclose(f);

    //printf("Parse done\n");
    return 0;
}
