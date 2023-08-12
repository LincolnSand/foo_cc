#pragma once


#include <stack>
#include <variant>
#include <stdexcept>

#include <frontend/ast/ast.hpp>


ast::constant_t evaluate_unary_expression(const ast::constant_t& operand, const ast::unary_operator_token_t operator_token);
ast::constant_t evaluate_binary_expression(const ast::constant_t& left, const ast::constant_t& right, const ast::binary_operator_token_t operator_token);
ast::constant_t evaluate_ternary_expression(const ast::constant_t& condition, const ast::constant_t& if_true, const ast::constant_t& if_false);

ast::constant_t evaluate_expression(const ast::expression_t& expression);
