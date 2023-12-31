#pragma once

#include <variant>
#include <string>
#include <stdexcept>

#include <frontend/ast/ast.hpp>
#include "traverse_ast_helpers.hpp"
#include <utils/common.hpp>


void generate_grouping(assembly_output_t& assembly_output, const ast::grouping_t& grouping);
void generate_convert(assembly_output_t& assembly_output, const ast::convert_t& convert);
void generate_unary_expression(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp);
void generate_binary_expression(assembly_output_t& assembly_output, const ast::binary_expression_t& binary_exp);
void generate_ternary_expression(assembly_output_t& assembly_output, const ast::ternary_expression_t& ternary_exp);
void generate_function_call(assembly_output_t& assembly_output, const ast::function_call_t& function_call_exp);
void generate_variable_access(assembly_output_t& assembly_output, const ast::variable_access_t& var_name);
void generate_constant(assembly_output_t& assembly_output, const ast::constant_t& constant);

void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression);

void generate_return_statement(assembly_output_t& assembly_output, const ast::return_statement_t& return_stmt);
void generate_if_statement(assembly_output_t& assembly_output, const ast::if_statement_t& if_stmt);
void generate_statement(assembly_output_t& assembly_output, const ast::statement_t& stmt);

void generate_declaration(assembly_output_t& assembly_output, const ast::declaration_t& decl);
// caller of `generate_compound_statement()` must create the new block scope for functions and populate it with the parameter list
void generate_compound_statement(assembly_output_t& assembly_output, const ast::compound_statement_t& compound_stmt, bool is_function = false);
void generate_function_definition(assembly_output_t& assembly_output, const ast::function_definition_t& function_definition);

void generate_program(assembly_output_t& assembly_output, const ast::validated_program_t& program);
std::string generate_asm(const ast::validated_program_t& program);
