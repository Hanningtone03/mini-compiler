#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer *lexer;
    Token current;
    Token previous;
    int had_error;
} Parser;

void parser_init(Parser *parser, Lexer *lexer);
Node *parser_parse_program(Parser *parser);

#endif