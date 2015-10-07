/*
 * $Id$
 */
#ifndef LEXER_H
#  define LEXER_H

enum lexer_state {
    start,
    parsing_comment,
    parsing_space,
    parsing_token,
    parsing_sg_string,
    parsing_db_string,
};

typedef struct {
    char *p;
    char *pe;
    FILE *f;
    char *buf;
    enum  lexer_state state;
} lexer_t;

enum token_type {
    TK_NONE,
    TK_NUM,
    TK_STRING,
    TK_COMMENT,
    TK_VAR,
    TK_ID,
    TK_SPACE,
    TK_SUPER_ID,
    TK_MISC,
    TK_OPEN_BLOCK,
    TK_CLOSE_BLOCK,
    TK_UNKNOWN,
    TK_EOF,
};

typedef struct {
    enum token_type type;
    char *start;
    char *end;
} token_t;


int lexer_init(lexer_t *l, FILE *f);
token_t *get_token(lexer_t *l, token_t *t);
void lexer_free(lexer_t *l);

#endif /* ifndef LEXER_H */

