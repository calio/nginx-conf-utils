/*
 * $Id$
 */
#ifndef LEXER_H
#  define LEXER_H

#include <stdint.h>

#define TOKEN(name) name

enum token_type {
#include "token.h"
};
#undef TOKEN

typedef struct {
    enum token_type type;
    char *start;
    char *end;
} token_t;

enum lexer_state {
    start,
    parsing_comment,
    parsing_space,
    parsing_token,
    parsing_sg_string,
    parsing_db_string,
    parsing_var,
    parsing_num,
};

typedef struct {
    char *p;
    char *pe;
    FILE *f;
    char *buf;
    enum  lexer_state state;
    token_t top_token;
    uint8_t peeked;
    uint8_t skip_comment;
    uint8_t skip_spaces;
} lexer_t;


int lexer_init(lexer_t *l, FILE *f);
token_t *get_token(lexer_t *l, token_t *t);
token_t *peek_token(lexer_t *l, token_t *t);
void lexer_free(lexer_t *l);
void lexer_skip(lexer_t *l, int comment, int spaces);

#endif /* ifndef LEXER_H */

