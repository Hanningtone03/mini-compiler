#ifndef COMPILER_H
#define COMPILER_H

#include "ast.h"
#include "vm.h"

void compiler_compile(Node *program, Chunk *chunk);

#endif