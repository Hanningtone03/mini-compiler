#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"

void chunk_init(Chunk *chunk) {
    chunk->code_count = 0;
    chunk->function_count = 0;
    chunk->global_count = 0;
}

int chunk_add_global(Chunk *chunk, const char *name) {
    for (int i = 0; i < chunk->global_count; i++) {
        if (strcmp(chunk->global_names[i], name) == 0) return i;
    }
    chunk->global_names[chunk->global_count] = strdup(name);
    chunk->globals[chunk->global_count].type = VAL_NUMBER;
    chunk->globals[chunk->global_count].number = 0;
    return chunk->global_count++;
}

void vm_init(VM *vm, Chunk *chunk) {
    vm->chunk = chunk;
    vm->ip = 0;
    vm->sp = 0;
    vm->frame_count = 0;
}

static void push(VM *vm, Value value) {
    if (vm->sp >= STACK_MAX) {
        fprintf(stderr, "Stack overflow\n");
        exit(1);
    }
    vm->stack[vm->sp++] = value;
}

static Value pop(VM *vm) {
    if (vm->sp <= 0) {
        fprintf(stderr, "Stack underflow\n");
        exit(1);
    }
    return vm->stack[--vm->sp];
}

static Value make_number(double n) {
    Value v;
    v.type = VAL_NUMBER;
    v.number = n;
    return v;
}

static Value make_bool(int b) {
    Value v;
    v.type = VAL_BOOL;
    v.boolean = b;
    return v;
}

static Value make_string(const char *s) {
    Value v;
    v.type = VAL_STRING;
    v.string = strdup(s);
    return v;
}

static int is_truthy(Value v) {
    if (v.type == VAL_BOOL) return v.boolean;
    if (v.type == VAL_NUMBER) return v.number != 0;
    return 1;
}

static void print_value(Value v) {
    switch (v.type) {
        case VAL_NUMBER:
            if (v.number == (long)v.number) {
                printf("%ld\n", (long)v.number);
            } else {
                printf("%g\n", v.number);
            }
            break;
        case VAL_STRING:
            printf("%s\n", v.string);
            break;
        case VAL_BOOL:
            printf("%s\n", v.boolean ? "true" : "false");
            break;
    }
}

static CallFrame *current_frame(VM *vm) {
    if (vm->frame_count == 0) return NULL;
    return &vm->frames[vm->frame_count - 1];
}

static int find_local(CallFrame *frame, const char *name) {
    if (!frame) return -1;
    for (int i = 0; i < frame->local_count; i++) {
        if (strcmp(frame->local_names[i], name) == 0) return i;
    }
    return -1;
}

void vm_run(VM *vm) {
    Chunk *chunk = vm->chunk;

    while (vm->ip < chunk->code_count) {
        Instruction inst = chunk->code[vm->ip];
        CallFrame *frame = current_frame(vm);

        switch (inst.op) {
            case OP_PUSH_NUMBER:
                push(vm, make_number(inst.operand));
                vm->ip++;
                break;

            case OP_PUSH_STRING:
                push(vm, make_string(inst.str_operand));
                vm->ip++;
                break;

            case OP_PUSH_BOOL:
                push(vm, make_bool((int)inst.operand));
                vm->ip++;
                break;

            case OP_LOAD_VAR: {
                const char *name = chunk->global_names[(int)inst.operand];
                int local_index = find_local(frame, name);
                if (local_index >= 0) {
                    push(vm, frame->locals[local_index]);
                } else {
                    push(vm, chunk->globals[(int)inst.operand]);
                }
                vm->ip++;
                break;
            }

            case OP_STORE_VAR: {
                const char *name = chunk->global_names[(int)inst.operand];
                Value value = pop(vm);
                int local_index = find_local(frame, name);
                if (local_index >= 0) {
                    frame->locals[local_index] = value;
                } else if (frame && frame->local_count < FRAME_VARS_MAX) {
                    frame->local_names[frame->local_count] = strdup(name);
                    frame->locals[frame->local_count] = value;
                    frame->local_count++;
                } else {
                    chunk->globals[(int)inst.operand] = value;
                }
                vm->ip++;
                break;
            }

            case OP_ADD: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_STRING || b.type == VAL_STRING) {
                    char buf[512];
                    char abuf[256], bbuf[256];
                    if (a.type == VAL_STRING) snprintf(abuf, sizeof(abuf), "%s", a.string);
                    else snprintf(abuf, sizeof(abuf), "%g", a.number);
                    if (b.type == VAL_STRING) snprintf(bbuf, sizeof(bbuf), "%s", b.string);
                    else snprintf(bbuf, sizeof(bbuf), "%g", b.number);
                    snprintf(buf, sizeof(buf), "%s%s", abuf, bbuf);
                    push(vm, make_string(buf));
                } else {
                    push(vm, make_number(a.number + b.number));
                }
                vm->ip++;
                break;
            }

            case OP_SUB: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, make_number(a.number - b.number));
                vm->ip++;
                break;
            }

            case OP_MUL: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, make_number(a.number * b.number));
                vm->ip++;
                break;
            }

            case OP_DIV: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (b.number == 0) {
                    fprintf(stderr, "Division by zero\n");
                    exit(1);
                }
                push(vm, make_number(a.number / b.number));
                vm->ip++;
                break;
            }

            case OP_NEG: {
                Value a = pop(vm);
                push(vm, make_number(-a.number));
                vm->ip++;
                break;
            }

            case OP_LT: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, make_bool(a.number < b.number));
                vm->ip++;
                break;
            }

            case OP_GT: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, make_bool(a.number > b.number));
                vm->ip++;
                break;
            }

            case OP_LE: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, make_bool(a.number <= b.number));
                vm->ip++;
                break;
            }

            case OP_GE: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, make_bool(a.number >= b.number));
                vm->ip++;
                break;
            }

            case OP_EQ: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, make_bool(a.number == b.number));
                vm->ip++;
                break;
            }

            case OP_NEQ: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, make_bool(a.number != b.number));
                vm->ip++;
                break;
            }

            case OP_PRINT: {
                Value v = pop(vm);
                print_value(v);
                vm->ip++;
                break;
            }

            case OP_JUMP:
                vm->ip = inst.jump_target;
                break;

            case OP_JUMP_IF_FALSE: {
                Value cond = pop(vm);
                if (!is_truthy(cond)) {
                    vm->ip = inst.jump_target;
                } else {
                    vm->ip++;
                }
                break;
            }

            case OP_CALL: {
                if (vm->frame_count >= CALL_STACK_MAX) {
                    fprintf(stderr, "Call stack overflow\n");
                    exit(1);
                }
                CallFrame *new_frame = &vm->frames[vm->frame_count++];
                new_frame->return_address = vm->ip + 1;
                new_frame->local_count = 0;
                vm->ip = inst.jump_target;
                break;
            }

            case OP_RETURN: {
                if (vm->frame_count > 0) {
                    CallFrame *finished = &vm->frames[--vm->frame_count];
                    for (int i = 0; i < finished->local_count; i++) {
                        free(finished->local_names[i]);
                    }
                    vm->ip = finished->return_address;
                } else {
                    return;
                }
                break;
            }

            case OP_POP:
                pop(vm);
                vm->ip++;
                break;

            case OP_HALT:
                return;
        }
    }
}