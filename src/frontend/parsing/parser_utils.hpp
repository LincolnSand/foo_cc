#pragma once


#include <stdexcept>
#include <utility>
#include <memory>

#include <frontend/lexing/lexer.hpp>
#include <frontend/ast/ast.hpp>
#include <utils/common.hpp>


bool is_constant(token_t token);
bool is_var_name(token_t token);

ast::var_name_t validate_lvalue_expression_exp(const ast::expression_t& expr);

std::unique_ptr<ast::grouping_t> make_grouping(ast::expression_t&& exp);
