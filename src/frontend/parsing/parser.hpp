#pragma once


#include <stdexcept>
#include <string_view>
#include <string>
#include <unordered_map>
#include <utility>
#include <memory>
#include <cstddef>
#include <cassert>

#include <frontend/lexing/lexer.hpp>
#include <frontend/ast/ast.hpp>
#include <utils/data_structures/random_access_stack.hpp>
#include <backend/interpreter/compile_time_evaluator.hpp>
#include "parser_utils.hpp"
#include <utils/common.hpp>


void validate_type_name(const ast::type_t& expected, const ast::type_t& actual, const std::string& error_message);

std::shared_ptr<ast::grouping_t> parse_grouping(parser_t& parser);

bool is_prefix_op(const token_t& token);
ast::unary_operator_token_t parse_prefix_op(const token_t& token);
ast::precedence_t prefix_binding_power(const ast::unary_operator_token_t token);
std::shared_ptr<ast::unary_expression_t> make_prefix_op(const ast::unary_operator_token_t op, ast::expression_t&& rhs);
ast::var_name_t parse_and_validate_variable(parser_t& parser, const token_t name_token);
std::shared_ptr<ast::function_call_t> parse_and_validate_function_call(parser_t& parser, const token_t name_token);
std::variant<ast::var_name_t, std::shared_ptr<ast::function_call_t>> parse_and_validate_variable_or_function_call(parser_t& parser);
ast::expression_t parse_prefix_expression(parser_t& parser);
bool is_postfix_op(const token_t& token);
ast::unary_operator_token_t parse_postfix_op(const token_t& token);
ast::precedence_t postfix_binding_power(const ast::unary_operator_token_t token);
std::shared_ptr<ast::unary_expression_t> make_postfix_op(const ast::unary_operator_token_t op, ast::expression_t&& lhs);
bool is_infix_binary_op(const token_t& token);
ast::binary_operator_token_t parse_infix_binary_op(const token_t& token);
std::pair<ast::precedence_t, ast::precedence_t> infix_binding_power(const ast::binary_operator_token_t token);
std::shared_ptr<ast::binary_expression_t> make_infix_op(const ast::binary_operator_token_t op, ast::expression_t&& lhs, ast::expression_t&& rhs);
bool is_compound_assignment_op(const token_t& token);
ast::binary_operator_token_t get_op_from_compound_assignment_op(const token_t& token);
std::pair<ast::precedence_t, ast::precedence_t> ternary_binding_power();
std::string get_identifier_from_expression(const ast::expression_t& expr);
ast::expression_t parse_and_validate_expression(parser_t& parser, const ast::precedence_t precedence);
ast::expression_t parse_and_validate_expression(parser_t& parser);

ast::type_t make_primitive_type(token_type_t token_type);

void add_floating_point_types_to_type_table(parser_t& parser);
void add_integer_types_to_type_table(parser_t& parser);
void add_unsigned_integer_types_to_type_table(parser_t& parser);

bool is_a_type(const parser_t& parser);

// checks whether or not the token is either a typedef or primitive type
bool is_a_type_token(parser_t& parser, token_t token);

ast::type_t parse_type_name_from_token(parser_t& parser, token_t token);
ast::type_t parse_struct_name_from_token(parser_t& parser, token_t token);

ast::global_variable_declaration_t parse_global_variable_declaration(parser_t& parser, ast::type_t var_type, token_t name_token);
ast::global_variable_declaration_t parse_global_variable_definition(parser_t& parser, ast::type_t var_type, token_t name_token);

ast::type_t parse_and_validate_typedef_struct_body(parser_t& parser, const ast::type_name_t& name);

ast::type_t parse_and_validate_anonymous_typedef_struct_definition(parser_t& parser);

ast::type_t parse_and_validate_type(parser_t& parser);
ast::type_t parse_typedef_struct_decl_or_def(parser_t& parser);

ast::return_statement_t parse_and_validate_return_statement(parser_t& parser);
ast::expression_statement_t parse_and_validate_expression_statement(parser_t& parser);
ast::if_statement_t parse_and_validate_if_statement(parser_t& parser);
ast::statement_t parse_and_validate_statement(parser_t& parser);
ast::declaration_t parse_and_validate_declaration(parser_t& parser);
ast::compound_statement_t parse_and_validate_compound_statement(parser_t& parser, bool is_function_block = false);
static std::vector<std::pair<ast::type_t, std::optional<ast::var_name_t>>> parse_function_definition_parameter_list(parser_t& parser);
static std::vector<ast::type_t> parse_function_declaration_parameter_list(std::vector<std::pair<ast::type_t, std::optional<ast::var_name_t>>> list_with_names);
ast::function_declaration_t parse_function_declaration(parser_t& parser, const token_t type_token, const token_t name_token, std::vector<std::pair<ast::type_t, std::optional<ast::var_name_t>>>&& param_list);
ast::function_definition_t parse_function_definition(parser_t& parser, const token_t type_token, const token_t name_token, std::vector<std::pair<ast::type_t, std::optional<ast::var_name_t>>>&& param_list);
std::variant<ast::function_declaration_t, ast::function_definition_t> parse_function_decl_or_def(parser_t& parser, const token_t type_token, const token_t name_token);
std::variant<ast::function_declaration_t, ast::function_definition_t, ast::global_variable_declaration_t> parse_function_or_global(parser_t& parser);
ast::type_t parse_and_validate_struct_body(parser_t& parser, const ast::type_name_t& name);
ast::type_t parse_struct(parser_t& parser);
ast::type_t parse_typedef(parser_t& parser);
std::variant<ast::function_declaration_t, ast::function_definition_t, ast::global_variable_declaration_t, ast::type_t> parse_top_level_declaration(parser_t& parser);
ast::validated_program_t parse(parser_t& parser);

// defined in middle_end/typing/generate_typing.cpp:

// `exp_type` is an outparam
void add_type_to_function_call(const validation_t& validation, const ast::function_call_t& expr, std::optional<ast::type_t>& exp_type);
void add_type_to_variable(const validation_t& validation, const ast::var_name_t& expr, std::optional<ast::type_t>& exp_type);
