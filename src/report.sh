#!/bin/bash

gcov /home/link/foo_cc/src/main.cpp --object-directory /home/link/foo_cc/src/main
gcov /home/link/foo_cc/src/io/file_io.cpp --object-directory /home/link/foo_cc/src/file_io
gcov /home/link/foo_cc/src/frontend/lexing/lexer.cpp --object-directory /home/link/foo_cc/src/lexer
gcov /home/link/foo_cc/src/frontend/parsing/parser_utils.cpp --object-directory /home/link/foo_cc/src/parser_utils
gcov /home/link/foo_cc/src/frontend/parsing/parser.cpp --object-directory /home/link/foo_cc/src/parser
gcov /home/link/foo_cc/src/middle_end/typing/generate_typing.cpp --object-directory /home/link/foo_cc/src/generate_typing
gcov /home/link/foo_cc/src/backend/interpreter/compile_time_evaluator.cpp --object-directory /home/link/foo_cc/src/compile_time_evaluator
gcov /home/link/foo_cc/src/frontend/ast/ast_printer.cpp --object-directory /home/link/foo_cc/src/ast_printer
