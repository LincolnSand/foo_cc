#pragma once

#include <variant>
#include <string>
#include <stdexcept>

#include <frontend/parsing/ast.hpp>
#include "compile_operators.hpp"


void generate_unary_op(assembly_output_t& assembly_output, const ast::unary_op_expression_t& op);
void generate_grouping(assembly_output_t& assembly_output, const ast::grouping_t& grouping);
void generate_factor(assembly_output_t& assembly_output, const ast::factor_t& factor);
void generate_times_divide_binary_expression(assembly_output_t& assembly_output, const ast::times_divide_binary_expression_t& expr);
void generate_times_divide_expression(assembly_output_t& assembly_output, const ast::times_divide_expression_t& expr);
void generate_plus_minus_binary_expression(assembly_output_t& assembly_output, const ast::plus_minus_binary_expression_t& expr);
void generate_plus_minus_expression(assembly_output_t& assembly_output, const ast::plus_minus_expression_t& expr);
void generate_relational_binary_expression(assembly_output_t& assembly_output, const ast::relational_binary_expression_t& expr);
void generate_relational_expression(assembly_output_t& assembly_output, const ast::relational_expression_t& expr);
void generate_equality_binary_expression(assembly_output_t& assembly_output, const ast::equality_binary_expression_t& expr);
void generate_equality_expression(assembly_output_t& assembly_output, const ast::equality_expression_t& expr);
void generate_logical_and_binary_expression(assembly_output_t& assembly_output, const ast::logical_and_binary_expression_t& expr);
void generate_logical_and_expression(assembly_output_t& assembly_output, const ast::logical_and_expression_t& expr);
void generate_logical_or_binary_expression(assembly_output_t& assembly_output, const ast::logical_or_binary_expression_t& expr);
void generate_logical_or_expression(assembly_output_t& assembly_output, const ast::logical_or_expression_t& expr);
void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression);
void generate_return_stmt(assembly_output_t& assembly_output, const ast::return_statement_t& return_stmt);
void generate_func_decl(assembly_output_t& assembly_output, const ast::function_declaration_t& function);
void generate_program(assembly_output_t& assembly_output, const ast::program_t& program);
std::string generate_asm(const ast::program_t& program);
