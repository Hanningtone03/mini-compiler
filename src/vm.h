#ifndef VM_H
#define VM_H

#define STACK_MAX 256
#define VARS_MAX 256
#define CODE_MAX 4096
#define CALL_STACK_MAX 64
#define FRAME_VARS_MAX 32

typedef enum {
    OP_PUSH_NUMBER,
    OP_PUSH_STRING,
    OP_PUSH_BOOL,
    OP_LOAD_VAR,
    OP_STORE_VAR,
    OP_LOAD_LOCAL,
    OP_STORE_LOCAL,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NEG,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_EQ,
    OP_NEQ,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_CALL,
    OP_RETURN,
    OP_POP,
    OP_HALT
} OpCode;

typedef enum {
    VAL_NUMBER,
    VAL_STRING,
    VAL_BOOL
} ValueType;

typedef struct {
    ValueType type;
    double number;
    char *string;
    int boolean;
} Value;

typedef struct {
    OpCode op;
    double operand;
    char *str_operand;
    int jump_target;
} Instruction;

typedef struct {
    char *name;
    int address;
    int param_count;
    char *param_names[FRAME_VARS_MAX];
} Function;

typedef struct {
    Instruction code[CODE_MAX];
    int code_count;

    Function functions[64];
    int function_count;

    Value globals[VARS_MAX];
    char *global_names[VARS_MAX];
    int global_count;
} Chunk;

typedef struct {
    int return_address;
    Value locals[FRAME_VARS_MAX];
    char *local_names[FRAME_VARS_MAX];
    int local_count;
} CallFrame;

typedef struct {
    Chunk *chunk;
    int ip;

    Value stack[STACK_MAX];
    int sp;

    CallFrame frames[CALL_STACK_MAX];
    int frame_count;
} VM;

void chunk_init(Chunk *chunk);
int chunk_add_global(Chunk *chunk, const char *name);

void vm_init(VM *vm, Chunk *chunk);
void vm_run(VM *vm);

#endif