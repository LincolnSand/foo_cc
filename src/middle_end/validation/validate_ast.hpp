#pragma once


#include <unordered_map>
#include <stdexcept>

#include <utils/common.hpp>
#include <utils/data_structures/random_access_stack.hpp>
#include <frontend/ast/ast.hpp>
#include <backend/interpreter/compile_time_evaluator.hpp>


struct validation_t {
    // TODO: we currently only support integer types. But this is to create some of the infrastructure for later when we add more types.
    utils::data_structures::validation_variable_lookup_t variable_lookup;
    std::unordered_map<ast::func_name_t, ast::function_declaration_t> function_declarations_lookup;
    std::unordered_map<ast::func_name_t, ast::function_definition_t> function_definitions_lookup;
    std::unordered_map<ast::var_name_t, ast::global_variable_declaration_t> global_variable_declarations;
    std::unordered_map<ast::var_name_t, ast::global_variable_declaration_t> global_variable_definitions;
};


void validate_type_name(const ast::type_name_t& expected, const ast::type_name_t& actual, const std::string& error_message);

void validate_variable(validation_t& validation, const ast::var_name_t& var_name);
void validate_variable_declaration(validation_t& validation, ast::declaration_t& declaration);
void validate_function_declaration(validation_t& validation, const ast::function_declaration_t& function_declaration);
void validate_expression(validation_t& validation, ast::expression_t& expression);
void validate_statement(validation_t& validation, ast::statement_t& statement);
void validate_compound_statement(validation_t& validation, ast::compound_statement_t& compound_statement, bool is_function_block = false);
void validate_function_definition(validation_t& validation, const ast::function_definition_t& function_definition);
void validate_function_call(validation_t& validation, const ast::function_call_t& function_call);
void validate_compile_time_expression(validation_t& validation, const ast::expression_t& expression);
void validate_global_variable_declaration(validation_t& validation, const ast::global_variable_declaration_t& declaration);
ast::validated_program_t validate_ast(const ast::program_t& program);


// defined in middle_end/typing/generate_typing.cpp:

// `exp_type` is an outparam
void add_type_to_grouping(const ast::grouping_t& expr, std::optional<ast::type_name_t>& exp_type);
void add_type_to_unary_expression(const ast::unary_expression_t& expr, std::optional<ast::type_name_t>& exp_type);
void add_type_to_binary_expression(const ast::binary_expression_t& expr, std::optional<ast::type_name_t>& exp_type);
void add_type_to_ternary(const ast::ternary_expression_t& expr, std::optional<ast::type_name_t>& exp_type);
void add_type_to_function_call(const validation_t& validation, const ast::function_call_t& expr, std::optional<ast::type_name_t>& exp_type);
void add_type_to_variable(const validation_t& validation, const ast::var_name_t& expr, std::optional<ast::type_name_t>& exp_type);
