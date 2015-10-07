#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"


void print_tk(token_t *t)
{
    if (t->type == TK_SPACE || t->type == TK_MISC || t->type == TK_COMMENT) {
        //return;
    }

    printf("[%d]%.*s\n", t->type, (int) (t->end - t->start), t->start);
}

int main(int argc, char **argv )
{
    lexer_t    lexer;
    token_t    token;

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
        if (get_token(&lexer, &token) != NULL) {
            if (token.type == TK_EOF) {
                break;
            }
            print_tk(&token);
        }
    }

    lexer_free(&lexer);
    fclose(f);

    printf("Parse done\n");
    return 0;
}
