#pragma once

#include "compile_operators.hpp"
#include <frontend/parsing/parser_utils.hpp>


// Put this here despite it being defined in `traverse_ast.cpp` and already having its prototype in `traverse_ast.hpp` since most of the functions in this file have to call it.
void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression);

void generate_binary_operation(assembly_output_t& assembly_output, const ast::binary_expression_t& binary_exp, void(*func)(assembly_output_t&));
void generate_logical_and(assembly_output_t& assembly_output, const ast::binary_expression_t& binary_exp);
void generate_logical_or(assembly_output_t& assembly_output, const ast::binary_expression_t& binary_exp);
void generate_assignment_expression(assembly_output_t& assembly_output, const ast::binary_expression_t& assignment);
void generate_unary_operation(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp, void(*func)(assembly_output_t&));
