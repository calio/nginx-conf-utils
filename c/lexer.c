
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "parser.h"

void cp_init(parser_ctx *ctx) {
    return;
}

int cp_lex(parser_ctx *ctx, token_t *t) {
    char ch;
    int skip_char = 0;

    assert(ctx->p < ctx->pe);

    t->type = TK_NONE;
    t->start = ctx->p;

    while (ctx->p != ctx->pe) {
        ch = *ctx->p++;

        switch (ctx->state) {
        case start:
            if (ch == '#') {
                /* comment line */
                ctx->state = parsing_comment;
                t->start = ctx->p - 1;
            } else if (ch == ' ' || ch == '\n' || ch == '\t') {
                /* space */
                ctx->state = parsing_space;
                t->start = ctx->p - 1;
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
                t->type = TK_MISC;
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
    }

    return 0;
}
