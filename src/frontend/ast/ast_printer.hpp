#pragma once


#include <frontend/ast/ast.hpp>


void print_expression(bool has_types, const ast::expression_t& expr);

void print_return_statement(bool has_types, const ast::return_statement_t& return_stmt);
void print_if_statement(bool has_types, const ast::if_statement_t& if_statement);
void print_statement(bool has_types, const ast::statement_t& stmt, bool is_nested, bool is_last_statement = false);

void print_declaration(bool has_types, const ast::declaration_t& declaration, bool is_last_statement = false);
void print_compound_statement(bool has_types, const ast::compound_statement_t& declaration_statement, bool is_nested);
void print_function_decl(const ast::function_declaration_t& func_decl);
void print_function_definition(bool has_types, const ast::function_definition_t& function_definition);

void print_global_variable_definition(const ast::global_variable_declaration_t& global_var_def);
void print_validated_ast(const ast::validated_program_t& validated_program);
