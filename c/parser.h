#ifndef PARSER_H
#  define PARSER_H

enum parser_state {
    parse_start,
    expect_command,
    expect_args,
};

typedef struct {
    int indent;
    enum parser_state state;
} parser_t;

int parse(lexer_t *l, FILE *f);
int parse_doc(parser_t *p, lexer_t *l, token_t *t);

#endif /* ifndef PARSER_H */
