#pragma once


#include <unordered_map>

#include <utils/common.hpp>
#include <utils/data_structures/random_access_stack.hpp>
#include <frontend/ast/ast.hpp>


struct validation_t {
    // TODO: we currently only support integer types. But this is to create some of the infrastructure for later when we add more types.
    utils::data_structures::validation_variable_lookup_t variable_lookup;
    std::unordered_map<ast::func_name_t, ast::function_declaration_t> declarations_lookup;
    std::unordered_map<ast::func_name_t, ast::function_definition_t> definitions_lookup;
};


void validate_type(const ast::type_name_t& expected, const ast::type_name_t& actual, const std::string& error_message);
void validate_variable(validation_t& validation, const ast::var_name_t& var_name);
void validate_variable_declaration(validation_t& validation, const ast::declaration_t& declaration);
void validate_function_declaration(validation_t& validation, const ast::function_declaration_t& function_declaration);
void validate_expression(validation_t& validation, const ast::expression_t& expression);
void validate_statement(validation_t& validation, const ast::statement_t& statement);
void validate_compound_statement(validation_t& validation, const ast::compound_statement_t& compound_statement, bool is_function_block = false);
void validate_function_definition(validation_t& validation, const ast::function_definition_t& function_definition);
void validate_function_call(validation_t& validation, const ast::function_call_t& function_call);
void validate_ast(const ast::program_t& program);
