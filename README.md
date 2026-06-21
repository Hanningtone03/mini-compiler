![CI](https://github.com/Hanningtone03/mini-compiler/actions/workflows/ci.yml/badge.svg)

# Mini Compiler

A compiler and virtual machine for a small custom language, written in C; lexer, parser, AST, bytecode compiler, and stack-based VM with proper call frames for recursion.

## How it works

Source code goes through four stages. The lexer turns raw text into tokens. The parser builds an abstract syntax tree using recursive descent with correct operator precedence. The compiler walks the tree and emits bytecode instructions. The virtual machine executes that bytecode on a stack, with each function call getting its own isolated set of local variables so recursion works correctly.

Pairs well with [raft-consensus](https://github.com/Hanningtone03/raft-consensus) and [tiny-blockchain](https://github.com/Hanningtone03/tiny-blockchain) — different domains, same idea: build the thing everyone else treats as a black box.

## Language features

Variables, arithmetic, comparisons, string concatenation, if/else, while loops, functions with parameters, recursion, and print statements.

## Project structure

```
src/
├── lexer.c / lexer.h
├── parser.c / parser.h
├── ast.c / ast.h
├── compiler.c / compiler.h
├── vm.c / vm.h
└── main.c
```

## Building

```bash
gcc -Wall -o compiler src/main.c src/lexer.c src/parser.c src/ast.c src/compiler.c src/vm.c
```

## Running

```bash
./compiler examples/basics.lang
./compiler examples/fibonacci.lang
```

Add `--ast` to print the parsed syntax tree, or `--tokens` to print the raw token stream.

## Example

```
func fib(n) {
    if (n < 2) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

print fib(10);
```

Recursive calls each get their own isolated local scope, so `fib(10)` correctly resolves to `55`:

![Fibonacci recursion resolving to 55](screenshots/result.png)

## Tech

- C
- No external dependencies
- No parser generator — hand-written recursive descent parser
