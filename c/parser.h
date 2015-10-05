/*
 * $Id$
 */
#ifndef PARSER_H
#  define PARSER_H

enum cp_state {
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
    char *eof;
    int cs;
    enum cp_state state;
} parser_ctx;

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
};

typedef struct {
    enum token_type type;
    char *start;
    char *end;
} token_t;

void cp_init(parser_ctx *ctx);
//int cp_parse(parser_ctx *ctx);
int cp_lex(parser_ctx *ctx, token_t *token);

#endif /* ifndef PARSER_H */
