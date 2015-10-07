#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"


#define TOKEN(name) #name
const char *token_name [] = {
#include "token.h"
NULL
};
#undef TOKEN

void print_tk(token_t *t)
{
    if (t->type == TK_SPACE || t->type == TK_MISC || t->type == TK_COMMENT) {
        //return;
    }

    printf("[%s]%.*s\n", token_name[t->type],
           (int) (t->end - t->start), t->start);
}

