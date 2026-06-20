#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

static void advance(Parser *parser);
static Node *parse_statement(Parser *parser);
static Node *parse_expression(Parser *parser);
static NodeList *parse_block(Parser *parser);

static void error(Parser *parser, const char *message) {
    fprintf(stderr, "Parse error at line %d: %s\n", parser->current.line, message);
    parser->had_error = 1;
}

void parser_init(Parser *parser, Lexer *lexer) {
    parser->lexer = lexer;
    parser->had_error = 0;
    advance(parser);
}

static void advance(Parser *parser) {
    parser->previous = parser->current;
    parser->current = lexer_next(parser->lexer);
}

static int check(Parser *parser, TokenType type) {
    return parser->current.type == type;
}

static int match(Parser *parser, TokenType type) {
    if (check(parser, type)) {
        advance(parser);
        return 1;
    }
    return 0;
}

static void expect(Parser *parser, TokenType type, const char *message) {
    if (check(parser, type)) {
        advance(parser);
        return;
    }
    error(parser, message);
}

static Node *parse_primary(Parser *parser) {
    int line = parser->current.line;

    if (match(parser, TOKEN_NUMBER)) {
        return node_number(parser->previous.number, line);
    }
    if (match(parser, TOKEN_STRING)) {
        return node_string(parser->previous.text, line);
    }
    if (match(parser, TOKEN_TRUE)) {
        return node_bool(1, line);
    }
    if (match(parser, TOKEN_FALSE)) {
        return node_bool(0, line);
    }
    if (match(parser, TOKEN_LPAREN)) {
        Node *expr = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "expected ')'");
        return expr;
    }
    if (check(parser, TOKEN_IDENT)) {
        char *name = strdup(parser->current.text);
        advance(parser);
        if (match(parser, TOKEN_LPAREN)) {
            NodeList *args = nodelist_new();
            if (!check(parser, TOKEN_RPAREN)) {
                nodelist_push(args, parse_expression(parser));
                while (match(parser, TOKEN_COMMA)) {
                    nodelist_push(args, parse_expression(parser));
                }
            }
            expect(parser, TOKEN_RPAREN, "expected ')' after arguments");
            Node *call = node_call(name, args, line);
            free(name);
            return call;
        }
        Node *ident = node_ident(name, line);
        free(name);
        return ident;
    }

    error(parser, "expected expression");
    advance(parser);
    return node_number(0, line);
}

static Node *parse_unary(Parser *parser) {
    int line = parser->current.line;
    if (match(parser, TOKEN_MINUS)) {
        Node *right = parse_unary(parser);
        return node_binary('u', node_number(0, line), right, line);
    }
    return parse_primary(parser);
}

static Node *parse_term(Parser *parser) {
    Node *left = parse_unary(parser);
    while (check(parser, TOKEN_STAR) || check(parser, TOKEN_SLASH)) {
        char op = check(parser, TOKEN_STAR) ? '*' : '/';
        int line = parser->current.line;
        advance(parser);
        Node *right = parse_unary(parser);
        left = node_binary(op, left, right, line);
    }
    return left;
}

static Node *parse_additive(Parser *parser) {
    Node *left = parse_term(parser);
    while (check(parser, TOKEN_PLUS) || check(parser, TOKEN_MINUS)) {
        char op = check(parser, TOKEN_PLUS) ? '+' : '-';
        int line = parser->current.line;
        advance(parser);
        Node *right = parse_term(parser);
        left = node_binary(op, left, right, line);
    }
    return left;
}

static Node *parse_comparison(Parser *parser) {
    Node *left = parse_additive(parser);
    while (check(parser, TOKEN_LT) || check(parser, TOKEN_GT) ||
           check(parser, TOKEN_LE) || check(parser, TOKEN_GE) ||
           check(parser, TOKEN_EQ) || check(parser, TOKEN_NEQ)) {
        char op;
        switch (parser->current.type) {
            case TOKEN_LT: op = '<'; break;
            case TOKEN_GT: op = '>'; break;
            case TOKEN_LE: op = 'l'; break;
            case TOKEN_GE: op = 'g'; break;
            case TOKEN_EQ: op = 'e'; break;
            default: op = 'n'; break;
        }
        int line = parser->current.line;
        advance(parser);
        Node *right = parse_additive(parser);
        left = node_binary(op, left, right, line);
    }
    return left;
}

static Node *parse_expression(Parser *parser) {
    return parse_comparison(parser);
}

static Node *parse_var_decl(Parser *parser) {
    int line = parser->current.line;
    expect(parser, TOKEN_VAR, "expected 'var'");
    char *name = strdup(parser->current.text);
    expect(parser, TOKEN_IDENT, "expected identifier");
    expect(parser, TOKEN_ASSIGN, "expected '='");
    Node *value = parse_expression(parser);
    expect(parser, TOKEN_SEMI, "expected ';'");
    Node *node = node_var_decl(name, value, line);
    free(name);
    return node;
}

static Node *parse_print(Parser *parser) {
    int line = parser->current.line;
    expect(parser, TOKEN_PRINT, "expected 'print'");
    Node *value = parse_expression(parser);
    expect(parser, TOKEN_SEMI, "expected ';'");
    return node_print(value, line);
}

static Node *parse_if(Parser *parser) {
    int line = parser->current.line;
    expect(parser, TOKEN_IF, "expected 'if'");
    expect(parser, TOKEN_LPAREN, "expected '('");
    Node *condition = parse_expression(parser);
    expect(parser, TOKEN_RPAREN, "expected ')'");
    NodeList *then_body = parse_block(parser);
    Node *then_branch = node_block(then_body, line);
    Node *else_branch = NULL;
    if (match(parser, TOKEN_ELSE)) {
        NodeList *else_body = parse_block(parser);
        else_branch = node_block(else_body, line);
    }
    return node_if(condition, then_branch, else_branch, line);
}

static Node *parse_while(Parser *parser) {
    int line = parser->current.line;
    expect(parser, TOKEN_WHILE, "expected 'while'");
    expect(parser, TOKEN_LPAREN, "expected '('");
    Node *condition = parse_expression(parser);
    expect(parser, TOKEN_RPAREN, "expected ')'");
    NodeList *body = parse_block(parser);
    return node_while(condition, body, line);
}

static Node *parse_func_decl(Parser *parser) {
    int line = parser->current.line;
    expect(parser, TOKEN_FUNC, "expected 'func'");
    char *name = strdup(parser->current.text);
    expect(parser, TOKEN_IDENT, "expected function name");
    expect(parser, TOKEN_LPAREN, "expected '('");
    NodeList *params = nodelist_new();
    if (!check(parser, TOKEN_RPAREN)) {
        nodelist_push(params, node_ident(parser->current.text, line));
        expect(parser, TOKEN_IDENT, "expected parameter");
        while (match(parser, TOKEN_COMMA)) {
            nodelist_push(params, node_ident(parser->current.text, line));
            expect(parser, TOKEN_IDENT, "expected parameter");
        }
    }
    expect(parser, TOKEN_RPAREN, "expected ')'");
    NodeList *body = parse_block(parser);
    Node *node = node_func_decl(name, params, body, line);
    free(name);
    return node;
}

static Node *parse_return(Parser *parser) {
    int line = parser->current.line;
    expect(parser, TOKEN_RETURN, "expected 'return'");
    Node *value = NULL;
    if (!check(parser, TOKEN_SEMI)) {
        value = parse_expression(parser);
    }
    expect(parser, TOKEN_SEMI, "expected ';'");
    return node_return(value, line);
}

static Node *parse_assign_or_expr(Parser *parser) {
    int line = parser->current.line;
    if (check(parser, TOKEN_IDENT)) {
        char *name = strdup(parser->current.text);
        Lexer saved_lexer = *parser->lexer;
        Token saved_current = parser->current;
        Token saved_previous = parser->previous;

        advance(parser);
        if (match(parser, TOKEN_ASSIGN)) {
            Node *value = parse_expression(parser);
            expect(parser, TOKEN_SEMI, "expected ';'");
            Node *node = node_assign(name, value, line);
            free(name);
            return node;
        }

        *parser->lexer = saved_lexer;
        parser->current = saved_current;
        parser->previous = saved_previous;
        free(name);
    }

    Node *expr = parse_expression(parser);
    expect(parser, TOKEN_SEMI, "expected ';'");
    return expr;
}

static Node *parse_statement(Parser *parser) {
    if (check(parser, TOKEN_VAR)) return parse_var_decl(parser);
    if (check(parser, TOKEN_PRINT)) return parse_print(parser);
    if (check(parser, TOKEN_IF)) return parse_if(parser);
    if (check(parser, TOKEN_WHILE)) return parse_while(parser);
    if (check(parser, TOKEN_FUNC)) return parse_func_decl(parser);
    if (check(parser, TOKEN_RETURN)) return parse_return(parser);
    return parse_assign_or_expr(parser);
}

static NodeList *parse_block(Parser *parser) {
    expect(parser, TOKEN_LBRACE, "expected '{'");
    NodeList *body = nodelist_new();
    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        nodelist_push(body, parse_statement(parser));
    }
    expect(parser, TOKEN_RBRACE, "expected '}'");
    return body;
}

Node *parser_parse_program(Parser *parser) {
    NodeList *body = nodelist_new();
    while (!check(parser, TOKEN_EOF)) {
        nodelist_push(body, parse_statement(parser));
    }
    return node_program(body);
}