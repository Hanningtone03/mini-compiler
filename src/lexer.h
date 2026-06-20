#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_NUMBER,
    TOKEN_IDENT,
    TOKEN_STRING,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_ASSIGN,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMI,
    TOKEN_COMMA,
    TOKEN_VAR,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FUNC,
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char *text;
    double number;
    int line;
} Token;

typedef struct {
    const char *source;
    int pos;
    int line;
} Lexer;

void lexer_init(Lexer *lexer, const char *source);
Token lexer_next(Lexer *lexer);
const char *token_type_name(TokenType type);

#endif