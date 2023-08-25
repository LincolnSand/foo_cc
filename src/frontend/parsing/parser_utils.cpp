#include "parser_utils.hpp"


bool is_constant(token_t token) {
    return token.token_type == token_type_t::INT_CONSTANT || token.token_type == token_type_t::DOUBLE_CONSTANT || token.token_type == token_type_t::CHAR_CONSTANT;
}
bool is_var_name(token_t token) {
    return token.token_type == token_type_t::IDENTIFIER;
}

ast::var_name_t validate_lvalue_expression_exp(const ast::expression_t& expr) {
    return std::visit(overloaded{
        [](const std::shared_ptr<ast::grouping_t>& grouping) -> ast::var_name_t {
            return validate_lvalue_expression_exp(grouping->expr);
        },
        [](const std::shared_ptr<ast::unary_expression_t>& unary_exp) -> ast::var_name_t {
            switch(unary_exp->op) {
                case ast::unary_operator_token_t::PLUS_PLUS:
                case ast::unary_operator_token_t::MINUS_MINUS:
                    if(unary_exp->fixity == ast::unary_operator_fixity_t::PREFIX) {
                        return validate_lvalue_expression_exp(unary_exp->exp);
                    }
            }
            throw std::runtime_error("Cannot assign to unary operator of type [" + std::to_string(static_cast<std::uint16_t>(unary_exp->op)) + "].");
            return "";
        },
        [](const std::shared_ptr<ast::binary_expression_t>& binary_exp) -> ast::var_name_t {
            switch(binary_exp->op) {
                case ast::binary_operator_token_t::ASSIGNMENT:
                    return validate_lvalue_expression_exp(binary_exp->left);
                case ast::binary_operator_token_t::COMMA:
                    return validate_lvalue_expression_exp(binary_exp->right);
            }
            throw std::runtime_error("Cannot assign to binary operator of type [" + std::to_string(static_cast<std::uint16_t>(binary_exp->op)) + "].");
        },
        // TODO: Check if C has lvalue ternary expressions. Currently only rvalue ternary expressions are supported. I believe only C++ has lvalue expressions, but I need to double check.
        [](const std::shared_ptr<ast::ternary_expression_t>& ternary_exp) -> ast::var_name_t {
            throw std::runtime_error("Cannot assign to ternary operator.");
            return "";
        },
        [](const std::shared_ptr<ast::function_call_t>& function_call) -> ast::var_name_t {
            throw std::runtime_error("Cannot assign to function call.");
            return "";
        },
        [](const ast::constant_t& constant) -> ast::var_name_t {
            throw std::runtime_error("You cannot assign to a constant.");
            return "";
        },
        [](const ast::var_name_t& var_name) -> ast::var_name_t {
            return var_name;
        },
        [](const std::shared_ptr<ast::convert_t>& convert) -> ast::var_name_t {
            throw std::runtime_error("Cannot assign to cast.");
            return "";
        }
    }, expr.expr);
}

std::unique_ptr<ast::grouping_t> make_grouping(ast::expression_t&& exp) {
    return std::make_unique<ast::grouping_t>(ast::grouping_t{std::move(exp)});
}
