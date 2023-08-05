#pragma once

#include <variant>
#include <string>
#include <stdexcept>

#include <frontend/parsing/ast.hpp>
#include "traverse_ast_helpers.hpp"


void generate_grouping(assembly_output_t& assembly_output, const ast::grouping_t& grouping);
void generate_unary_expression(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp);
void generate_binary_expression(assembly_output_t& assembly_output, const ast::binary_expression_t& binary_exp);
void generate_ternary_expression(assembly_output_t& assembly_output, const ast::ternary_expression_t& ternary_exp);

void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression);

void generate_return_statement(assembly_output_t& assembly_output, const ast::return_statement_t& return_stmt);
void generate_if_statement(assembly_output_t& assembly_output, const ast::if_statement_t& if_stmt);
void generate_statement(assembly_output_t& assembly_output, const ast::statement_t& stmt);

void generate_declaration(assembly_output_t& assembly_output, const ast::declaration_t& decl);
void generate_compound_statement(assembly_output_t& assembly_output, const ast::compound_statement_t& compound_stmt);
void generate_function_decl(assembly_output_t& assembly_output, const ast::function_declaration_t& function);

void generate_program(assembly_output_t& assembly_output, const ast::program_t& program);
std::string generate_asm(const ast::program_t& program);
