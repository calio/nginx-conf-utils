/*
 *
 */

#include <stdio.h>
#include "lexer.h"
#include "util.h"

int main(int argc, char **argv)
{

    lexer_t   lexer;
    token_t   token;
    token_t  *t;

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

    while (1) {
        t = get_token(&lexer, &token);
        print_tk(t);

        if (t->type == TK_EOF) {
            break;
        }
    }

	return 0;
}
