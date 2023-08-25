#!/bin/bash

# /home/link/foo_cc/src/backend/x86_64/compile_operators.cpp /home/link/foo_cc/src/backend/x86_64/traverse_ast.cpp /home/link/foo_cc/src/backend/x86_64/traverse_ast_helpers.cpp

# -fsanitize=address -static-libasan -g
g++ -std=c++17 /home/link/foo_cc/src/main.cpp /home/link/foo_cc/src/io/file_io.cpp /home/link/foo_cc/src/frontend/lexing/lexer.cpp /home/link/foo_cc/src/frontend/parsing/parser_utils.cpp /home/link/foo_cc/src/frontend/parsing/parser.cpp /home/link/foo_cc/src/frontend/ast/ast_printer.cpp /home/link/foo_cc/src/middle_end/validation/validate_ast.cpp /home/link/foo_cc/src/middle_end/typing/generate_typing.cpp /home/link/foo_cc/src/middle_end/typing/type_checker.cpp /home/link/foo_cc/src/backend/interpreter/compile_time_evaluator.cpp -I/home/link/foo_cc/src -fsanitize=address -static-libasan -g -o /home/link/foo_cc/src/main_out
