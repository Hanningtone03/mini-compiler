#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

NodeList *nodelist_new(void) {
    NodeList *list = malloc(sizeof(NodeList));
    list->capacity = 8;
    list->count = 0;
    list->items = malloc(sizeof(Node *) * list->capacity);
    return list;
}

void nodelist_push(NodeList *list, Node *node) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = realloc(list->items, sizeof(Node *) * list->capacity);
    }
    list->items[list->count++] = node;
}

static Node *new_node(NodeType type, int line) {
    Node *node = malloc(sizeof(Node));
    memset(node, 0, sizeof(Node));
    node->type = type;
    node->line = line;
    return node;
}

Node *node_number(double value, int line) {
    Node *node = new_node(NODE_NUMBER, line);
    node->number = value;
    return node;
}

Node *node_string(const char *value, int line) {
    Node *node = new_node(NODE_STRING, line);
    node->string = strdup(value);
    return node;
}

Node *node_bool(int value, int line) {
    Node *node = new_node(NODE_BOOL, line);
    node->boolean = value;
    return node;
}

Node *node_ident(const char *name, int line) {
    Node *node = new_node(NODE_IDENT, line);
    node->name = strdup(name);
    return node;
}

Node *node_binary(char op, Node *left, Node *right, int line) {
    Node *node = new_node(NODE_BINARY, line);
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

Node *node_assign(const char *name, Node *value, int line) {
    Node *node = new_node(NODE_ASSIGN, line);
    node->name = strdup(name);
    node->value = value;
    return node;
}

Node *node_var_decl(const char *name, Node *value, int line) {
    Node *node = new_node(NODE_VAR_DECL, line);
    node->name = strdup(name);
    node->value = value;
    return node;
}

Node *node_print(Node *value, int line) {
    Node *node = new_node(NODE_PRINT, line);
    node->value = value;
    return node;
}

Node *node_if(Node *condition, Node *then_branch, Node *else_branch, int line) {
    Node *node = new_node(NODE_IF, line);
    node->condition = condition;
    node->then_branch = then_branch;
    node->else_branch = else_branch;
    return node;
}

Node *node_while(Node *condition, NodeList *body, int line) {
    Node *node = new_node(NODE_WHILE, line);
    node->condition = condition;
    node->body = body;
    return node;
}

Node *node_block(NodeList *body, int line) {
    Node *node = new_node(NODE_BLOCK, line);
    node->body = body;
    return node;
}

Node *node_func_decl(const char *name, NodeList *params, NodeList *body, int line) {
    Node *node = new_node(NODE_FUNC_DECL, line);
    node->name = strdup(name);
    node->params = params;
    node->body = body;
    return node;
}

Node *node_call(const char *name, NodeList *args, int line) {
    Node *node = new_node(NODE_CALL, line);
    node->name = strdup(name);
    node->args = args;
    return node;
}

Node *node_return(Node *value, int line) {
    Node *node = new_node(NODE_RETURN, line);
    node->value = value;
    return node;
}

Node *node_program(NodeList *body) {
    Node *node = new_node(NODE_PROGRAM, 0);
    node->body = body;
    return node;
}

static void print_indent(int depth) {
    for (int i = 0; i < depth; i++) printf("  ");
}

void node_print_tree(Node *node, int depth) {
    if (!node) return;

    print_indent(depth);

    switch (node->type) {
        case NODE_NUMBER:
            printf("Number(%.2f)\n", node->number);
            break;
        case NODE_STRING:
            printf("String(\"%s\")\n", node->string);
            break;
        case NODE_BOOL:
            printf("Bool(%s)\n", node->boolean ? "true" : "false");
            break;
        case NODE_IDENT:
            printf("Ident(%s)\n", node->name);
            break;
        case NODE_BINARY:
            printf("Binary(%c)\n", node->op);
            node_print_tree(node->left, depth + 1);
            node_print_tree(node->right, depth + 1);
            break;
        case NODE_ASSIGN:
            printf("Assign(%s)\n", node->name);
            node_print_tree(node->value, depth + 1);
            break;
        case NODE_VAR_DECL:
            printf("VarDecl(%s)\n", node->name);
            node_print_tree(node->value, depth + 1);
            break;
        case NODE_PRINT:
            printf("Print\n");
            node_print_tree(node->value, depth + 1);
            break;
        case NODE_IF:
            printf("If\n");
            node_print_tree(node->condition, depth + 1);
            node_print_tree(node->then_branch, depth + 1);
            if (node->else_branch) node_print_tree(node->else_branch, depth + 1);
            break;
        case NODE_WHILE:
            printf("While\n");
            node_print_tree(node->condition, depth + 1);
            for (int i = 0; i < node->body->count; i++) {
                node_print_tree(node->body->items[i], depth + 1);
            }
            break;
        case NODE_BLOCK:
            printf("Block\n");
            for (int i = 0; i < node->body->count; i++) {
                node_print_tree(node->body->items[i], depth + 1);
            }
            break;
        case NODE_FUNC_DECL:
            printf("FuncDecl(%s)\n", node->name);
            for (int i = 0; i < node->body->count; i++) {
                node_print_tree(node->body->items[i], depth + 1);
            }
            break;
        case NODE_CALL:
            printf("Call(%s)\n", node->name);
            for (int i = 0; i < node->args->count; i++) {
                node_print_tree(node->args->items[i], depth + 1);
            }
            break;
        case NODE_RETURN:
            printf("Return\n");
            if (node->value) node_print_tree(node->value, depth + 1);
            break;
        case NODE_PROGRAM:
            printf("Program\n");
            for (int i = 0; i < node->body->count; i++) {
                node_print_tree(node->body->items[i], depth + 1);
            }
            break;
    }
}