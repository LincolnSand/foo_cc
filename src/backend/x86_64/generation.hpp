#pragma once

#include <variant>
#include <string>
#include <stdexcept>

#include <frontend/parsing/ast.hpp>


using assembly_output_t = std::string;

void store_constant(assembly_output_t& assembly_output, const ast::constant_t& constant);
void store_register(assembly_output_t& assembly_output, const std::string& register_name);
void pop_constant(assembly_output_t& assembly_output, const std::string& register_name);
void generate_negation(assembly_output_t& assembly_output);
void generate_bitwise_not(assembly_output_t& assembly_output);
void generate_logic_not(assembly_output_t& assembly_output);
void generate_unary_op(assembly_output_t& assembly_output, const ast::unop_t& expression);
void generate_grouping(assembly_output_t& assembly_output, const ast::grouping_t& grouping);
void generate_factor(assembly_output_t& assembly_output, const ast::factor_t& factor);
void generate_times_divide_expr(assembly_output_t& assembly_output, const ast::times_divide_expression_t& expression);
void generate_plus_minus_expr(assembly_output_t& assembly_output, const ast::plus_minus_expression_t& expression);
void generate_term(assembly_output_t& assembly_output, const ast::term_t& term);
void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression);
void generate_return_statement(assembly_output_t& assembly_output, const ast::return_statement_t& statement);
void generate_function_decl(assembly_output_t& assembly_output, const ast::function_declaration_t& function_declaration);
assembly_output_t generate_asm(const ast::program_t& program);
