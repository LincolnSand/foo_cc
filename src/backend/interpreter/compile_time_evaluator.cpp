#include "compile_time_evaluator.hpp"


ast::constant_t evaluate_unary_expression(const ast::constant_t& operand, const ast::unary_operator_token_t operator_token) {
    return std::visit(overloaded{
        [operator_token](const int unwrapped_operand) {
            switch(operator_token) {
                case ast::unary_operator_token_t::PLUS:
                    return ast::constant_t{unwrapped_operand};
                case ast::unary_operator_token_t::MINUS:
                    return  ast::constant_t{-unwrapped_operand};
                case ast::unary_operator_token_t::LOGICAL_NOT:
                    return  ast::constant_t{!unwrapped_operand};
                case ast::unary_operator_token_t::BITWISE_NOT:
                    return  ast::constant_t{~unwrapped_operand};
            }
            throw std::logic_error("Unsupported unary operator.");
        },
        [operator_token](const auto& unwrapped_operand) {
            switch(operator_token) {
                case ast::unary_operator_token_t::PLUS:
                    return ast::constant_t{unwrapped_operand};
                case ast::unary_operator_token_t::MINUS:
                    return  ast::constant_t{-unwrapped_operand};
                case ast::unary_operator_token_t::LOGICAL_NOT:
                    return  ast::constant_t{!unwrapped_operand};
            }
            throw std::logic_error("Unsupported unary operator.");
        }
    }, operand.value);
}
ast::constant_t evaluate_binary_expression(const ast::constant_t& left, const ast::constant_t& right, const ast::binary_operator_token_t operator_token) {
    // TODO: Add support for different types (e.g. double) as well as operands of different types (e.g. double + int).
    return std::visit(overloaded{
        [operator_token, right](const int unwrapped_left) {
            return std::visit(overloaded{
                [operator_token, unwrapped_left](const int unwrapped_right) -> ast::constant_t {
                    switch(operator_token) {
                        case ast::binary_operator_token_t::MULTIPLY:
                            return ast::constant_t{unwrapped_left * unwrapped_right};
                        case ast::binary_operator_token_t::DIVIDE:
                            if(unwrapped_right == 0) {
                                throw std::runtime_error("Division by zero.");
                            }
                            return ast::constant_t{unwrapped_left / unwrapped_right};
                        case ast::binary_operator_token_t::MODULO:
                            if(unwrapped_right == 0) {
                                throw std::runtime_error("Division by zero.");
                            }
                            return ast::constant_t{unwrapped_left % unwrapped_right};
                        case ast::binary_operator_token_t::PLUS:
                            return ast::constant_t{unwrapped_left + unwrapped_right};
                        case ast::binary_operator_token_t::MINUS:
                            return ast::constant_t{unwrapped_left - unwrapped_right};
                        case ast::binary_operator_token_t::LEFT_BITSHIFT:
                            return ast::constant_t{unwrapped_left << unwrapped_right};
                        case ast::binary_operator_token_t::RIGHT_BITSHIFT:
                            return ast::constant_t{unwrapped_left >> unwrapped_right};
                        case ast::binary_operator_token_t::LESS_THAN:
                            return ast::constant_t{unwrapped_left < unwrapped_right};
                        case ast::binary_operator_token_t::LESS_THAN_EQUAL:
                            return ast::constant_t{unwrapped_left <= unwrapped_right};
                        case ast::binary_operator_token_t::GREATER_THAN:
                            return ast::constant_t{unwrapped_left > unwrapped_right};
                        case ast::binary_operator_token_t::GREATER_THAN_EQUAL:
                            return ast::constant_t{unwrapped_left >= unwrapped_right};
                        case ast::binary_operator_token_t::EQUAL:
                            return ast::constant_t{unwrapped_left == unwrapped_right};
                        case ast::binary_operator_token_t::NOT_EQUAL:
                            return ast::constant_t{unwrapped_left != unwrapped_right};
                        case ast::binary_operator_token_t::BITWISE_AND:
                            return ast::constant_t{unwrapped_left & unwrapped_right};
                        case ast::binary_operator_token_t::BITWISE_XOR:
                            return ast::constant_t{unwrapped_left ^ unwrapped_right};
                        case ast::binary_operator_token_t::BITWISE_OR:
                            return ast::constant_t{unwrapped_left | unwrapped_right};
                        // We don't need to handle short circuiting for the subexpressions of logical and & or because assignment, function calls, and other side effect producing operators (e.g. `++`, `--`)
                        //  are disallowed at compile time. So it doesn't matter if we fully evaluate both subexpressions.
                        case ast::binary_operator_token_t::LOGICAL_AND:
                            return ast::constant_t{unwrapped_left && unwrapped_right};
                        case ast::binary_operator_token_t::LOGICAL_OR:
                            return ast::constant_t{unwrapped_left || unwrapped_right};
                        // Because of the aforementioned reasoning regarding short circuiting, using the comma operator at compile time is silly and has no purpose, but it is still valid and supported anyways.
                        case ast::binary_operator_token_t::COMMA:
                            return ast::constant_t{unwrapped_right};
                    }
                    throw std::logic_error("Unsupported binary operator.");
                },
                [](const auto&) -> ast::constant_t {
                    throw std::runtime_error("Operands are of different types.");
                    return {};
                }
            }, right.value);
        },
        [](const auto& unwrapped_left) -> ast::constant_t {
            // TODO: implement at some point
            throw std::runtime_error("Currently only integer types are supported at compile time.");
            return {};
        }
    }, left.value);
}
ast::constant_t evaluate_ternary_expression(const ast::constant_t& condition, const ast::constant_t& if_true, const ast::constant_t& if_false) {
    return std::visit(overloaded{
        [&if_true, &if_false](const auto& unwrapped_condition) {
            return std::visit(overloaded{
                [&unwrapped_condition, &if_false](const auto& unwrapped_if_true) {
                    return std::visit(overloaded{
                        [&unwrapped_condition, &unwrapped_if_true](const auto& unwrapped_if_false) {
                            return ast::constant_t{unwrapped_condition ? unwrapped_if_true : unwrapped_if_false};
                        }
                    }, if_false.value);
                }
            }, if_true.value);
        }
    }, condition.value);
}

ast::constant_t evaluate_expression(const ast::expression_t& expression) {
    return std::visit(overloaded{
        [](const std::shared_ptr<ast::grouping_t>& expression) -> ast::constant_t {
            return evaluate_expression(expression->expr);
        },
        [](const std::shared_ptr<ast::unary_expression_t>& expression) -> ast::constant_t {
            if(expression->op == ast::unary_operator_token_t::PLUS_PLUS || expression->op == ast::unary_operator_token_t::MINUS_MINUS) {
                throw std::runtime_error("`++` and `--` not supported in compile time expressions.");
            }
            const ast::constant_t operand = evaluate_expression(expression->exp);
            return evaluate_unary_expression(operand, expression->op);
        },
        [](const std::shared_ptr<ast::binary_expression_t>& expression) -> ast::constant_t {
            if(expression->op == ast::binary_operator_token_t::ASSIGNMENT) {
                throw std::runtime_error("Assignment not supported in compile time expressions.");
            }
            const ast::constant_t left = evaluate_expression(expression->left);
            const ast::constant_t right = evaluate_expression(expression->right);
            return evaluate_binary_expression(left, right, expression->op);
        },
        [](const std::shared_ptr<ast::ternary_expression_t>& expression) -> ast::constant_t {
            const ast::constant_t condition = evaluate_expression(expression->condition);
            const ast::constant_t if_true = evaluate_expression(expression->if_true);
            const ast::constant_t if_false = evaluate_expression(expression->if_false);
            return evaluate_ternary_expression(condition, if_true, if_false);
        },
        [](const ast::constant_t& expression) -> ast::constant_t {
            return expression;
        },
        [](const std::shared_ptr<ast::convert_t>& expression) -> ast::constant_t {
            throw std::runtime_error("Casts not yet implemented at compile time.");
            return {};
        },
        [](const auto&) -> ast::constant_t {
            throw std::runtime_error("Expression evaluation not supported at compile time.");
            return {};
        }
    }, expression.expr);
}
