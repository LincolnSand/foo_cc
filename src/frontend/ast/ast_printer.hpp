#pragma once


#include <frontend/ast/ast.hpp>


void print_expression(const ast::expression_t& expr);

void print_return_statement(const ast::return_statement_t& return_stmt);
void print_if_statement(const ast::if_statement_t& if_statement);
void print_statement(const ast::statement_t& stmt, bool is_last_statement = false);

void print_declaration(const ast::declaration_t& declaration, bool is_last_statement = false);
void print_compound_statement(const ast::compound_statement_t& declaration_statement);
void print_function_decl(const ast::function_declaration_t& func_decl);
void print_function_definition(const ast::function_definition_t& function_definition);

void print_ast(const ast::program_t& program);