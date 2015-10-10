#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "lexer.h"


static long get_fsize(FILE *f)
{
    long pos;
    long rc;

    pos = ftell(f);
    fseek(f, 0, SEEK_END);

    rc = ftell(f);
    fseek(f, pos, SEEK_SET);

    return rc;
}

void lexer_skip(lexer_t *l, int comment, int spaces)
{
    if (comment) {
        l->skip_comment = 1;
    }

    if (spaces) {
        l->skip_spaces = 1;
    }
}

int lexer_init(lexer_t *l, FILE *f)
{
    long rc;
    size_t size;
    size_t s;

    rc = get_fsize(f);
    if (rc < 0) {
        perror("get size error");
        return -1;
    }

    size = (size_t) rc;

    l->buf = malloc(size);
    if (l->buf == NULL) {
        perror("Malloc failed");
        return -1;
    }

    s = fread(l->buf, 1, size, f);

    if (s < size) {
        perror("Read file error");
        return -1;
    }

    l->p = l->buf;
    l->pe = l->buf + s;
    l->state = start;
    l->peeked = 0;
    l->skip_comment = 0;
    l->skip_spaces = 0;

    return 0;
}

void lexer_free(lexer_t *l)
{
    free(l->buf);
}

static int cp_lex(lexer_t *ctx, token_t *t)
{
    char ch, pch;
    int skip_char = 0;
    int parsing_var = 0;

    t->type = TK_NONE;
    t->start = ctx->p;

    while (ctx->p != ctx->pe) {
        ch = *ctx->p++;

        switch (ctx->state) {
        case start:
            if (ch == '#') {
                /* comment line */
                t->type = TK_COMMENT;
                ctx->state = parsing_comment;
                t->start = ctx->p - 1;
            } else if (ch == ' ' || ch == '\n' || ch == '\t') {
                /* space */
                t->type = TK_SPACE;
                ctx->state = parsing_space;
                t->start = ctx->p - 1;
            } else if (ch == '(') {
                t->start = ctx->p - 1;
                t->end = ctx->p;
                t->type = TK_LEFT_PAREN;
                return 0;
            } else if (ch == ')') {
                t->start = ctx->p - 1;
                t->end = ctx->p;
                t->type = TK_RIGHT_PAREN;
                return 0;
            } else if (ch == '{') {
                /* open block */
                t->start = ctx->p - 1;
                t->end = ctx->p;
                t->type = TK_OPEN_BLOCK;
                return 0;
            } else if (ch == '}') {
                /* close block */
                t->start = ctx->p - 1;
                t->end = ctx->p;
                t->type = TK_CLOSE_BLOCK;
                return 0;
            } else if (ch == ';') {
                /* line end */
                t->start = ctx->p - 1;
                t->end = ctx->p;
                t->type = TK_SEMI_COLON;
                return 0;
            } else {
                /* parsing string/token */
                t->start = ctx->p - 1;
                if (ch == '\'') {
                    ctx->state = parsing_sg_string;
                } else if (ch == '"') {
                    ctx->state = parsing_db_string;
                } else {
                    ctx->state = parsing_token;
                }
            }
            break;
        case parsing_token:
            if (ch == '\n' || ch == ' ' || ch == '\n' || ch == '\t'
                || ch == ';' || ch == '{' || ch == '}')
            {
                if (ch == '{' && pch == '$') {
                    /* variable */
                    parsing_var = 1;
                    break;
                }
                if (ch == '}' && parsing_var) {
                    parsing_var = 0;
                    break;
                }
                ctx->p--;
                t->end = ctx->p;
                t->type = TK_ID;
                ctx->state = start;
                return 0;
            }
            break;
        case parsing_sg_string:
            if (skip_char) {
                skip_char = 0;
                break;
            }
            if (ch == '\\') {
                skip_char = 1;
            } else if (ch == '\'') {
                t->end = ctx->p;
                t->type = TK_STRING;
                ctx->state = start;
                return 0;
            }
            break;
        case parsing_db_string:
            if (skip_char) {
                skip_char = 0;
                break;
            }
            if (ch == '\\') {
                skip_char = 1;
            } else if (ch == '"') {
                t->end = ctx->p;
                t->type = TK_STRING;
                ctx->state = start;
                return 0;
            }
            break;
        case parsing_comment:
            if (ch == '\n') {
                t->end = ctx->p;
                t->type = TK_COMMENT;
                ctx->state = start;
                return 0;
            }
            break;
        case parsing_space:
            if (ch != ' ' && ch != '\n' && ch != '\t') {
                ctx->p--;
                t->end = ctx->p;
                t->type = TK_SPACE;
                ctx->state = start;
                return 0;
            }
            break;
        default:
            printf("unknown state: %d\n", ctx->state);
            return -1;
            break;
        }

        pch = ch;
    }

    return 0;
}

token_t *get_token(lexer_t *l, token_t *t)
{
    int  rc;

    if (l->peeked) {
        memcpy(t, &l->top_token, sizeof(token_t));
        l->peeked = 0;
        return t;
    }

    while (1) {
        if (l->p >= l->pe) {
            t->type = TK_EOF;
            t->start = NULL;
            t->end = NULL;
            return t;
        }

        rc  = cp_lex(l, t);
        if (rc != 0) {
            printf("parsing error: %.*s", 10, l->p);
            return NULL;
        }

        if (l->skip_comment && t->type == TK_COMMENT) {
            continue;
        }

        if (l->skip_spaces && t->type == TK_SPACE) {
            continue;
        }

        break;
    }

    return t;
}

token_t *peek_token(lexer_t *l, token_t *t)
{
    if (l->peeked) {
        return &l->top_token;
    }

    (void) get_token(l, &l->top_token);
    l->peeked = 1;

    memcpy(t, &l->top_token, sizeof(token_t));
    return &l->top_token;
}

