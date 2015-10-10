/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

int main(int argc, char **argv)
{
    int      rc;
    lexer_t  lexer;

    if (argc < 2) {
        printf("Usage: %s file\n", argv[0]);
        return 0;
    }

    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        perror("File error");
        return -1;
    }

    lexer_init(&lexer, f);
    lexer_skip(&lexer, 1, 1);

    rc = parse(&lexer, f);

    lexer_free(&lexer);
    fclose(f);

    return rc;
}
