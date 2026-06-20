#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

void lexer_init(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->pos = 0;
    lexer->line = 1;
}

static char peek(Lexer *lexer) {
    return lexer->source[lexer->pos];
}

static char peek_next(Lexer *lexer) {
    if (lexer->source[lexer->pos] == '\0') return '\0';
    return lexer->source[lexer->pos + 1];
}

static char advance(Lexer *lexer) {
    char c = lexer->source[lexer->pos++];
    if (c == '\n') lexer->line++;
    return c;
}

static void skip_whitespace(Lexer *lexer) {
    while (1) {
        char c = peek(lexer);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance(lexer);
        } else if (c == '/' && peek_next(lexer) == '/') {
            while (peek(lexer) != '\n' && peek(lexer) != '\0') advance(lexer);
        } else {
            break;
        }
    }
}

static Token make_token(TokenType type, const char *text, int line) {
    Token token;
    token.type = type;
    token.text = text ? strdup(text) : NULL;
    token.number = 0;
    token.line = line;
    return token;
}

static Token make_number(Lexer *lexer) {
    int start = lexer->pos;
    while (isdigit(peek(lexer))) advance(lexer);
    if (peek(lexer) == '.' && isdigit(peek_next(lexer))) {
        advance(lexer);
        while (isdigit(peek(lexer))) advance(lexer);
    }
    int length = lexer->pos - start;
    char *buf = malloc(length + 1);
    strncpy(buf, lexer->source + start, length);
    buf[length] = '\0';

    Token token = make_token(TOKEN_NUMBER, buf, lexer->line);
    token.number = atof(buf);
    free(buf);
    return token;
}

static Token make_ident(Lexer *lexer) {
    int start = lexer->pos;
    while (isalnum(peek(lexer)) || peek(lexer) == '_') advance(lexer);
    int length = lexer->pos - start;
    char *buf = malloc(length + 1);
    strncpy(buf, lexer->source + start, length);
    buf[length] = '\0';

    TokenType type = TOKEN_IDENT;
    if (strcmp(buf, "var") == 0) type = TOKEN_VAR;
    else if (strcmp(buf, "if") == 0) type = TOKEN_IF;
    else if (strcmp(buf, "else") == 0) type = TOKEN_ELSE;
    else if (strcmp(buf, "while") == 0) type = TOKEN_WHILE;
    else if (strcmp(buf, "func") == 0) type = TOKEN_FUNC;
    else if (strcmp(buf, "return") == 0) type = TOKEN_RETURN;
    else if (strcmp(buf, "print") == 0) type = TOKEN_PRINT;
    else if (strcmp(buf, "true") == 0) type = TOKEN_TRUE;
    else if (strcmp(buf, "false") == 0) type = TOKEN_FALSE;

    Token token = make_token(type, buf, lexer->line);
    free(buf);
    return token;
}

static Token make_string(Lexer *lexer) {
    advance(lexer);
    int start = lexer->pos;
    while (peek(lexer) != '"' && peek(lexer) != '\0') advance(lexer);
    int length = lexer->pos - start;
    char *buf = malloc(length + 1);
    strncpy(buf, lexer->source + start, length);
    buf[length] = '\0';
    advance(lexer);

    Token token = make_token(TOKEN_STRING, buf, lexer->line);
    free(buf);
    return token;
}

Token lexer_next(Lexer *lexer) {
    skip_whitespace(lexer);

    if (peek(lexer) == '\0') return make_token(TOKEN_EOF, NULL, lexer->line);

    char c = peek(lexer);

    if (isdigit(c)) return make_number(lexer);
    if (isalpha(c) || c == '_') return make_ident(lexer);
    if (c == '"') return make_string(lexer);

    advance(lexer);

    switch (c) {
        case '+': return make_token(TOKEN_PLUS, "+", lexer->line);
        case '-': return make_token(TOKEN_MINUS, "-", lexer->line);
        case '*': return make_token(TOKEN_STAR, "*", lexer->line);
        case '/': return make_token(TOKEN_SLASH, "/", lexer->line);
        case '(': return make_token(TOKEN_LPAREN, "(", lexer->line);
        case ')': return make_token(TOKEN_RPAREN, ")", lexer->line);
        case '{': return make_token(TOKEN_LBRACE, "{", lexer->line);
        case '}': return make_token(TOKEN_RBRACE, "}", lexer->line);
        case ';': return make_token(TOKEN_SEMI, ";", lexer->line);
        case ',': return make_token(TOKEN_COMMA, ",", lexer->line);
        case '=':
            if (peek(lexer) == '=') { advance(lexer); return make_token(TOKEN_EQ, "==", lexer->line); }
            return make_token(TOKEN_ASSIGN, "=", lexer->line);
        case '!':
            if (peek(lexer) == '=') { advance(lexer); return make_token(TOKEN_NEQ, "!=", lexer->line); }
            break;
        case '<':
            if (peek(lexer) == '=') { advance(lexer); return make_token(TOKEN_LE, "<=", lexer->line); }
            return make_token(TOKEN_LT, "<", lexer->line);
        case '>':
            if (peek(lexer) == '=') { advance(lexer); return make_token(TOKEN_GE, ">=", lexer->line); }
            return make_token(TOKEN_GT, ">", lexer->line);
    }

    return make_token(TOKEN_ERROR, NULL, lexer->line);
}

const char *token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_IDENT: return "IDENT";
        case TOKEN_STRING: return "STRING";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_STAR: return "STAR";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_EQ: return "EQ";
        case TOKEN_NEQ: return "NEQ";
        case TOKEN_LT: return "LT";
        case TOKEN_GT: return "GT";
        case TOKEN_LE: return "LE";
        case TOKEN_GE: return "GE";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_SEMI: return "SEMI";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_VAR: return "VAR";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_FUNC: return "FUNC";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_PRINT: return "PRINT";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";
        case TOKEN_EOF: return "EOF";
        default: return "ERROR";
    }
}