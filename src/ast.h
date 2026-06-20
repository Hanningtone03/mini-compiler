#ifndef AST_H
#define AST_H

typedef enum {
    NODE_NUMBER,
    NODE_STRING,
    NODE_BOOL,
    NODE_IDENT,
    NODE_BINARY,
    NODE_ASSIGN,
    NODE_VAR_DECL,
    NODE_PRINT,
    NODE_IF,
    NODE_WHILE,
    NODE_BLOCK,
    NODE_FUNC_DECL,
    NODE_CALL,
    NODE_RETURN,
    NODE_PROGRAM
} NodeType;

typedef struct Node Node;

typedef struct {
    Node **items;
    int count;
    int capacity;
} NodeList;

struct Node {
    NodeType type;
    int line;

    double number;
    char *string;
    int boolean;
    char *name;

    char op;
    Node *left;
    Node *right;

    Node *condition;
    Node *then_branch;
    Node *else_branch;

    NodeList *body;
    NodeList *params;
    NodeList *args;

    Node *value;
};

NodeList *nodelist_new(void);
void nodelist_push(NodeList *list, Node *node);

Node *node_number(double value, int line);
Node *node_string(const char *value, int line);
Node *node_bool(int value, int line);
Node *node_ident(const char *name, int line);
Node *node_binary(char op, Node *left, Node *right, int line);
Node *node_assign(const char *name, Node *value, int line);
Node *node_var_decl(const char *name, Node *value, int line);
Node *node_print(Node *value, int line);
Node *node_if(Node *condition, Node *then_branch, Node *else_branch, int line);
Node *node_while(Node *condition, NodeList *body, int line);
Node *node_block(NodeList *body, int line);
Node *node_func_decl(const char *name, NodeList *params, NodeList *body, int line);
Node *node_call(const char *name, NodeList *args, int line);
Node *node_return(Node *value, int line);
Node *node_program(NodeList *body);

void node_print_tree(Node *node, int depth);

#endif