#pragma once


#include <frontend/ast/ast.hpp>


bool is_convertible(const ast::type_t& lhs, const ast::type_t& rhs);

void type_check_expression(ast::expression_t& expression);
void type_check_statement(ast::statement_t& statement, const ast::type_t& function_return_type);
void type_check_declaration(ast::declaration_t& declaration);
void type_check_compound_statement(ast::compound_statement_t& compound_statement, const ast::type_t function_return_type);
void type_check_function_definition(ast::function_definition_t& function_definition);
// TODO: take in `std::unordered_map<ast::type_t, ast::type>` once we add custom/user-defined types
void type_check(ast::validated_program_t& validated_program);
