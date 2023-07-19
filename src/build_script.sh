#!/bin/bash

# -fsanitize=address -static-libasan -g
g++ -std=c++17 /home/link/foo_cc/src/main.cpp /home/link/foo_cc/src/io/file_io.cpp /home/link/foo_cc/src/frontend/parsing/parser.cpp /home/link/foo_cc/src/frontend/parsing/ast.cpp /home/link/foo_cc/src/frontend/lexing/lexer.cpp /home/link/foo_cc/src/backend/x86_64/generation.cpp -I/home/link/foo_cc/src -o /home/link/foo_cc/src/main_out
