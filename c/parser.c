#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lexer.h"
#include "parser.h"
#include "util.h"


/*
 * Grammar:
 *
 *     main        := doc;
 *     doc          = command*;
 *     command      = ( (directive args (';' | block))
 *                    | map
 *                    | include
 *                    );
 *     directive    = ID;
 *     args         = arg*;
 *     arg          = ID|sring;
 *     block        = '{' doc '}';
 *     map          = 'map' ID|string ID '{' map_content '}';
 *     map_content  = (key value ';')*;
 *     key          = ID|string;
 *     value        = ID|string;
 *     include      = 'include' ID ';';
 *
 */

#define MAX_PATH_SIZE  256
#define expect(EXP, t)                                                  \
do {                                                                    \
    printf("Expected " EXP " but got %s at line %d\n",                  \
            token_name[t->type], __LINE__);                             \
} while (0)


void my_indent(parser_t *p) {
    int i;
    for (i = 0; i < p->indent; i++) {
        printf("  ");
    }
}

void print_info(parser_t *p, char *name, token_t *t)
{
    my_indent(p);
    printf("%s:%.*s\n", name, (int) (t->end - t->start), t->start);
}

int parse_args(parser_t *p, lexer_t *l, token_t *t)
{
    if (peek_token(l, t) == NULL) {
        return -1;
    }

    if (t->type == TK_EOF) {
        expect("TK_ID or TK_SEMI_COLON or TK_OPEN_BLOCK", t);
        //printf("EOF\n");
        return -1;
    }

    while (t->type != TK_SEMI_COLON && t->type != TK_OPEN_BLOCK) {
        (void) get_token(l, t);

        if (t->type != TK_ID && t->type != TK_STRING) {
            expect("TK_ID", t);
            return -1;
        }

        print_info(p, "arg", t);

        if (peek_token(l, t) == NULL) {
            return -1;
        }
    }

    /* t->type == TK_OPEN_BLOCK || t->type == TK_SEMI_COLON */

    return 0;
}

int parse_directive(parser_t *p, lexer_t *l, token_t *t)
{
    if (get_token(l, t) == NULL) {
        return -1;
    }

    if (t->type != TK_ID) {
        expect("TK_ID", t);
        return -1;
    }

    print_info(p, "directive", t);

    return 0;
}

int parse_block(parser_t *p, lexer_t *l, token_t *t)
{
    if (get_token(l, t) == NULL) {
        return -1;
    }

    if (t->type != TK_OPEN_BLOCK) {
        expect("TK_OPEN_BLOCK", t);
        return -1;
    }

    p->indent++;

    parse_doc(p, l, t);

    if (get_token(l, t) == NULL) {
        return -1;
    }

    if (t->type != TK_CLOSE_BLOCK) {
        expect("TK_CLOSE_BLOCK", t);
        return -1;
    }

    p->indent--;
    return 0;
}

int parse_map_content(parser_t *p, lexer_t *l, token_t *t)
{
    while (1) {
        if (get_token(l, t) == NULL) {
            return -1;
        }

        if (t->type != TK_ID && t->type != TK_STRING) {
            expect("TK_ID or TK_STRING", t);
            return -1;
        }
        print_info(p, "key", t);

        if (get_token(l, t) == NULL) {
            return -1;
        }

        if (t->type != TK_ID && t->type != TK_STRING) {
            expect("TK_ID or TK_STRING", t);
            return -1;
        }
        print_info(p, "value", t);

        if (get_token(l, t) == NULL) {
            return -1;
        }

        if (t->type != TK_SEMI_COLON) {
            expect("TK_SEMI_COLON", t);
            return -1;
        }

        if (peek_token(l, t) == NULL) {
            return -1;
        }

        if (t->type == TK_CLOSE_BLOCK) {
            break;
        }
    }

    return 0;
}

int parse_map(parser_t *p, lexer_t *l, token_t *t)
{
    /* read "map" */
    (void) get_token(l, t);
    print_info(p, "directive", t);

    if (get_token(l, t) == NULL) {
        return -1;
    }

    if (t->type != TK_ID && t->type != TK_STRING) {
        expect("TK_ID or TK_STRING", t);
        return -1;
    }
    print_info(p, "arg", t);

    if (get_token(l, t) == NULL) {
        return -1;
    }

    if (t->type != TK_ID) {
        expect("TK_ID", t);
        return -1;
    }
    print_info(p, "arg", t);

    if (get_token(l, t) == NULL) {
        return -1;
    }

    if (t->type != TK_OPEN_BLOCK) {
        expect("TK_OPEN_BLOCK", t);
        return -1;
    }
    p->indent++;

    if (parse_map_content(p, l, t) != 0) {
        return -1;
    }

    if (get_token(l, t) == NULL) {
        return -1;
    }

    if (t->type != TK_CLOSE_BLOCK) {
        expect("TK_OPEN_BLOCK", t);
        return -1;
    }
    p->indent--;

    return 0;
}

int parse_include(parser_t *p, lexer_t *l, token_t *t)
{
    parser_t old_parser;
    lexer_t lexer;
    FILE *f;
    char *filename;
    size_t s, len;

    /* include */
    (void) get_token(l, t);

    if (get_token(l, t) == NULL) {
        return -1;
    }

    if (t->type != TK_ID && t->type != TK_STRING) {
        expect("TK_ID or TK_STRING", t);
        return -1;
    }

    filename = malloc(MAX_PATH_SIZE);
    if (filename == NULL) {
        perror("Allocate memory or filename");
        return -1;
    }

    s = t->end - t->start;

    getcwd(filename, MAX_PATH_SIZE);
    len = strnlen(filename, MAX_PATH_SIZE);

    if (len + s >= MAX_PATH_SIZE) {
        printf("filename too long");
        return -1;
    }

    filename[len] = '/';

    if (*t->start == '"' || *t->start == '\'') {
        /* strip quotes */
        memcpy(filename + len + 1, t->start + 1, s - 1);
        filename[len + s - 1] = '\0';
    } else {
        memcpy(filename + len + 1, t->start, s);
        filename[len + s + 1] = '\0';
    }

    f = fopen(filename, "r");
    if (f == NULL) {
        printf("Can't open file %s\n", filename);
        return -1;
    }

    lexer_init(&lexer, f);
    lexer_skip(&lexer, 1, 1);

    memcpy(&old_parser, p, sizeof(parser_t));

    p->state = parse_start;

    if (parse_doc(p, &lexer, t) != 0) {
        return -1;
    }

    memcpy(p, &old_parser, sizeof(parser_t));

    if (get_token(l, t) == NULL) {
        return -1;
    }

    if (t->type != TK_SEMI_COLON) {
        expect("TK_SEMI_COLON", t);
        return -1;
    }

    return 0;
}

int parse_cmd(parser_t *p, lexer_t *l, token_t *t)
{
    int rc;

    if (peek_token(l, t) == NULL) {
        return -1;
    }

    if (t->type == TK_ID && t->end - t->start == 3
        && strncmp("map", t->start, 3) == 0)
    {
        return parse_map(p, l, t);
    }

    if (t->type == TK_ID && t->end - t->start == 7
        && strncmp("include", t->start, 7) == 0)
    {
        return parse_include(p, l, t);
    }


    rc = parse_directive(p, l, t);
    if (rc != 0) {
        return rc;
    }

    rc = parse_args(p, l, t);
    if (rc != 0) {
        return rc;
    }

    if (peek_token(l, t) == NULL) {
        return -1;
    }

    if (t->type == TK_OPEN_BLOCK) {
        return parse_block(p, l, t);
    }

    if (t->type == TK_SEMI_COLON) {
        (void) get_token(l, t);

        return 0;
    }

    expect("TK_OPEN_BLOCK or TK_SEMI_COLON", t);
    return -1;
}

int parse_doc(parser_t *p, lexer_t *l, token_t *t)
{
    int rc;

    while (1) {
        if (peek_token(l, t) == NULL) {
            return -1;
        }

        if (t->type == TK_EOF) {
            (void) get_token(l, t);
            //printf("EOF\n");
            break;
        }

        if (t->type == TK_CLOSE_BLOCK) {
            return 0;
        }

        rc = parse_cmd(p, l, t);
        if (rc != 0) {
            return rc;
        }
    }

    return 0;
}

int parse(lexer_t *l, FILE *f)
{
    parser_t  parser;
    parser_t  *p;
    token_t    token;
    token_t    *t;

    t = &token;
    p = &parser;
    p->state = parse_start;
    p->indent = 0;

    return parse_doc(p, l, t);
}

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

    printf("Parse done\n");
    return 0;
}
