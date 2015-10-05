#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"


#define BLK_SIZE 80


void print_tk(token_t *t)
{
    if (t->type == TK_SPACE || t->type == TK_MISC || t->type == TK_COMMENT) {
        return;
    }

    printf("[%d]%.*s\n", t->type, (int) (t->end - t->start), t->start);
}

int main(int argc, char **argv )
{
    char        *buf;
    size_t       s;
    size_t       read_size;
    parser_ctx   ctx;
    int          eof;
    int          rc;
    token_t      token;

    if (argc < 2) {
        printf("Usage: %s file\n", argv[0]);
        return 0;
    }

    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        perror("File error");
        return -1;
    }

    buf = malloc(BLK_SIZE);
    if (buf == NULL) {
        perror("Malloc failed");
        return -1;
    }

    cp_init(&ctx);

    eof = 0;
    read_size = BLK_SIZE;

    s = fread(buf, 1, read_size, f);

    if (s < read_size) {
        eof = feof(f);
        if (!eof) {
            perror("Read file error");
            return -1;
        }
    }

    ctx.p = buf;
    ctx.pe = buf + s;
    ctx.state = start;

    while (1) {
        while (ctx.p != ctx.pe) {
            rc  = cp_lex(&ctx, &token);
            if (rc != 0) {
                printf("parsing error: %.*s", 10, ctx.p);
                return -1;
            }

            if (token.type != TK_NONE) {
                print_tk(&token);
            }
        }

        if (eof) {
            printf("END of file\n");
            break;
        }

        buf = malloc(BLK_SIZE);
        if (buf == NULL) {
            perror("Malloc failed");
            return -1;
        }

        if (token.type == TK_NONE) {
            /* half token */
            s = ctx.pe - token.start;
            memcpy(buf, token.start, s);
            read_size = BLK_SIZE - s;
            ctx.p = buf + s;
        } else {
            s = 0;
            read_size = BLK_SIZE;
            ctx.p = buf;
        }

        if (read_size <= 0) {
            printf("Token larger than BLK_SIZE: %d\n", BLK_SIZE);
            return -1;
        }

        s = fread(buf + s, 1, read_size, f);
        if (s < read_size) {
            eof = feof(f);
            if (!eof) {
                perror("Read file error");
                return -1;
            }
        }

        ctx.pe = ctx.p + s;
    }

    printf("Parse done\n");
    return 0;
}
