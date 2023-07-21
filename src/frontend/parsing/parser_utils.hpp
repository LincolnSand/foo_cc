#pragma once


#include <stdexcept>
#include <utility>
#include <memory>

#include <frontend/lexing/lexer.hpp>
#include "ast.hpp"


bool is_unary_operator(token_t token);
bool is_constant(token_t token);

ast::unary_op_t convert_to_unary_token(token_t token);
ast::times_divide_t convert_to_times_divide_token(token_t token);
ast::plus_minus_t convert_to_plus_minus_token(token_t token);
ast::relational_t convert_to_relational_token(token_t token);
ast::equality_t convert_to_equality_token(token_t token);
ast::logical_and_t convert_to_logical_and_token(token_t token);
ast::logical_or_t convert_to_logical_or_token(token_t token);

std::shared_ptr<ast::grouping_t> make_grouping(ast::expression_t expr);
std::shared_ptr<ast::unary_op_expression_t> make_unary_expression(ast::unary_op_t op, ast::factor_t factor);
std::shared_ptr<ast::times_divide_binary_expression_t> make_times_divide_binary_expression(ast::times_divide_expression_t first_param, ast::times_divide_t op, ast::times_divide_expression_t second_param);
std::shared_ptr<ast::plus_minus_binary_expression_t> make_plus_minus_binary_expression(ast::plus_minus_expression_t first_param, ast::plus_minus_t op, ast::plus_minus_expression_t second_param);
std::shared_ptr<ast::relational_binary_expression_t> make_relational_binary_expression(ast::relational_expression_t first_param, ast::relational_t op, ast::relational_expression_t second_param);
std::shared_ptr<ast::equality_binary_expression_t> make_equality_binary_expression(ast::equality_expression_t first_param, ast::equality_t op, ast::equality_expression_t second_param);
std::shared_ptr<ast::logical_and_binary_expression_t> make_logical_and_binary_expression(ast::logical_and_expression_t first_param, ast::logical_and_t op, ast::logical_and_expression_t second_param);
std::shared_ptr<ast::logical_or_binary_expression_t> make_logical_or_binary_expression(ast::logical_or_expression_t first_param, ast::logical_or_t op, ast::logical_or_expression_t second_param);
