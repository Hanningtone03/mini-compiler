#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

static int emit(Chunk *chunk, OpCode op) {
    chunk->code[chunk->code_count].op = op;
    chunk->code[chunk->code_count].operand = 0;
    chunk->code[chunk->code_count].str_operand = NULL;
    chunk->code[chunk->code_count].jump_target = 0;
    return chunk->code_count++;
}

static int emit_number(Chunk *chunk, OpCode op, double value) {
    int index = emit(chunk, op);
    chunk->code[index].operand = value;
    return index;
}

static int emit_string(Chunk *chunk, OpCode op, const char *value) {
    int index = emit(chunk, op);
    chunk->code[index].str_operand = strdup(value);
    return index;
}

static void patch_jump(Chunk *chunk, int index, int target) {
    chunk->code[index].jump_target = target;
}

static void compile_node(Node *node, Chunk *chunk);

static void compile_binary(Node *node, Chunk *chunk) {
    if (node->op == 'u') {
        compile_node(node->right, chunk);
        emit(chunk, OP_NEG);
        return;
    }

    compile_node(node->left, chunk);
    compile_node(node->right, chunk);

    switch (node->op) {
        case '+': emit(chunk, OP_ADD); break;
        case '-': emit(chunk, OP_SUB); break;
        case '*': emit(chunk, OP_MUL); break;
        case '/': emit(chunk, OP_DIV); break;
        case '<': emit(chunk, OP_LT); break;
        case '>': emit(chunk, OP_GT); break;
        case 'l': emit(chunk, OP_LE); break;
        case 'g': emit(chunk, OP_GE); break;
        case 'e': emit(chunk, OP_EQ); break;
        case 'n': emit(chunk, OP_NEQ); break;
    }
}

static void compile_node(Node *node, Chunk *chunk) {
    if (!node) return;

    switch (node->type) {
        case NODE_NUMBER:
            emit_number(chunk, OP_PUSH_NUMBER, node->number);
            break;

        case NODE_STRING:
            emit_string(chunk, OP_PUSH_STRING, node->string);
            break;

        case NODE_BOOL:
            emit_number(chunk, OP_PUSH_BOOL, node->boolean);
            break;

        case NODE_IDENT: {
            int index = chunk_add_global(chunk, node->name);
            emit_number(chunk, OP_LOAD_VAR, index);
            break;
        }

        case NODE_BINARY:
            compile_binary(node, chunk);
            break;

        case NODE_ASSIGN: {
            compile_node(node->value, chunk);
            int index = chunk_add_global(chunk, node->name);
            emit_number(chunk, OP_STORE_VAR, index);
            break;
        }

        case NODE_VAR_DECL: {
            compile_node(node->value, chunk);
            int index = chunk_add_global(chunk, node->name);
            emit_number(chunk, OP_STORE_VAR, index);
            break;
        }

        case NODE_PRINT:
            compile_node(node->value, chunk);
            emit(chunk, OP_PRINT);
            break;

        case NODE_IF: {
            compile_node(node->condition, chunk);
            int jump_else = emit(chunk, OP_JUMP_IF_FALSE);

            for (int i = 0; i < node->then_branch->body->count; i++) {
                compile_node(node->then_branch->body->items[i], chunk);
            }

            if (node->else_branch) {
                int jump_end = emit(chunk, OP_JUMP);
                patch_jump(chunk, jump_else, chunk->code_count);
                for (int i = 0; i < node->else_branch->body->count; i++) {
                    compile_node(node->else_branch->body->items[i], chunk);
                }
                patch_jump(chunk, jump_end, chunk->code_count);
            } else {
                patch_jump(chunk, jump_else, chunk->code_count);
            }
            break;
        }

        case NODE_WHILE: {
            int loop_start = chunk->code_count;
            compile_node(node->condition, chunk);
            int jump_exit = emit(chunk, OP_JUMP_IF_FALSE);

            for (int i = 0; i < node->body->count; i++) {
                compile_node(node->body->items[i], chunk);
            }

            int jump_back = emit(chunk, OP_JUMP);
            patch_jump(chunk, jump_back, loop_start);
            patch_jump(chunk, jump_exit, chunk->code_count);
            break;
        }

        case NODE_BLOCK:
            for (int i = 0; i < node->body->count; i++) {
                compile_node(node->body->items[i], chunk);
            }
            break;

        case NODE_FUNC_DECL: {
            int jump_over = emit(chunk, OP_JUMP);
            int func_address = chunk->code_count;

            Function *func = &chunk->functions[chunk->function_count++];
            func->name = strdup(node->name);
            func->address = func_address;
            func->param_count = node->params->count;

            for (int i = node->params->count - 1; i >= 0; i--) {
                int index = chunk_add_global(chunk, node->params->items[i]->name);
                emit_number(chunk, OP_STORE_VAR, index);
            }

            for (int i = 0; i < node->body->count; i++) {
                compile_node(node->body->items[i], chunk);
            }

            emit_number(chunk, OP_PUSH_NUMBER, 0);
            emit(chunk, OP_RETURN);

            patch_jump(chunk, jump_over, chunk->code_count);
            break;
        }

        case NODE_CALL: {
            Function *func = NULL;
            for (int i = 0; i < chunk->function_count; i++) {
                if (strcmp(chunk->functions[i].name, node->name) == 0) {
                    func = &chunk->functions[i];
                    break;
                }
            }
            if (!func) {
                fprintf(stderr, "Unknown function: %s\n", node->name);
                exit(1);
            }

            for (int i = node->args->count - 1; i >= 0; i--) {
                compile_node(node->args->items[i], chunk);
            }

            int call_index = emit(chunk, OP_CALL);
            patch_jump(chunk, call_index, func->address);
            break;
        }

        case NODE_RETURN:
            if (node->value) {
                compile_node(node->value, chunk);
            } else {
                emit_number(chunk, OP_PUSH_NUMBER, 0);
            }
            emit(chunk, OP_RETURN);
            break;

        case NODE_PROGRAM:
            for (int i = 0; i < node->body->count; i++) {
                compile_node(node->body->items[i], chunk);
            }
            emit(chunk, OP_HALT);
            break;
    }
}

void compiler_compile(Node *program, Chunk *chunk) {
    chunk_init(chunk);
    compile_node(program, chunk);
}