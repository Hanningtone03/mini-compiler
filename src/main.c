#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "compiler.h"
#include "vm.h"

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Could not open file: %s\n", path);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = '\0';

    fclose(file);
    return buffer;
}

static void run_source(const char *source, int show_ast, int show_tokens) {
    if (show_tokens) {
        printf("\n--- Tokens ---\n");
        Lexer token_lexer;
        lexer_init(&token_lexer, source);
        Token token;
        do {
            token = lexer_next(&token_lexer);
            printf("  %-10s line %d\n", token_type_name(token.type), token.line);
        } while (token.type != TOKEN_EOF);
        printf("\n");
    }

    Lexer lexer;
    lexer_init(&lexer, source);

    Parser parser;
    parser_init(&parser, &lexer);

    Node *program = parser_parse_program(&parser);

    if (parser.had_error) {
        fprintf(stderr, "Compilation failed due to parse errors.\n");
        exit(1);
    }

    if (show_ast) {
        printf("--- AST ---\n");
        node_print_tree(program, 0);
        printf("\n");
    }

    Chunk chunk;
    compiler_compile(program, &chunk);

    printf("--- Output ---\n");
    VM vm;
    vm_init(&vm, &chunk);
    vm_run(&vm);
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file.lang> [--ast] [--tokens]\n", argv[0]);
        printf("\nExample:\n  %s examples/fibonacci.lang\n\n", argv[0]);
        return 1;
    }

    int show_ast = 0;
    int show_tokens = 0;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--ast") == 0) show_ast = 1;
        if (strcmp(argv[i], "--tokens") == 0) show_tokens = 1;
    }

    char *source = read_file(argv[1]);
    run_source(source, show_ast, show_tokens);
    free(source);

    return 0;
}