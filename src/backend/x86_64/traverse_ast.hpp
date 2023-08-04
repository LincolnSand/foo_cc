#pragma once

#include <variant>
#include <string>
#include <stdexcept>

#include <frontend/parsing/ast.hpp>
#include "traverse_ast_helpers.hpp"


void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression);
void generate_decl(assembly_output_t& assembly_output, const ast::declaration_t& decl);
void generate_return_stmt(assembly_output_t& assembly_output, const ast::return_statement_t& return_stmt);
void generate_stmt(assembly_output_t& assembly_output, const ast::statement_t& stmt);
void generate_func_decl(assembly_output_t& assembly_output, const ast::function_declaration_t& function);
void generate_program(assembly_output_t& assembly_output, const ast::program_t& program);
std::string generate_asm(const ast::program_t& program);
