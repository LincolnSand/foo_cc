#include "parser_utils.hpp"


bool is_unary_operator(token_t token) {
    switch(token.token_type) {
        case token_type_t::DASH:
        case token_type_t::TILDE:
        case token_type_t::BANG:
            return true;
    }
    return false;
}
bool is_constant(token_t token) {
    return token.token_type == token_type_t::INT_CONSTANT;
}
bool is_var_name(token_t token) {
    return token.token_type == token_type_t::IDENTIFIER;
}










ast::var_name_t validate_lvalue_expression_factor(const ast::factor_t& factor) {
    return std::visit(overloaded{
        [](const std::shared_ptr<ast::grouping_t>& factor) -> ast::var_name_t {
            return validate_lvalue_expression_exp(factor->expr);
        },
        [](const std::shared_ptr<ast::unary_op_expression_t>& factor) -> ast::var_name_t {
            throw std::runtime_error("Unary operator forms an invalid lvalue.");
            return "";
        },
        [](const ast::constant_t& factor) -> ast::var_name_t {
            throw std::runtime_error("Constant forms an invalid lvalue.");
            return "";
        },
        [](const ast::var_name_t& factor) -> ast::var_name_t {
            return factor;
        }
    }, factor);
}

ast::var_name_t validate_lvalue_expression_times_divide(const ast::times_divide_expression_t& expr) {
    return std::visit(overloaded{
        [](const std::shared_ptr<ast::times_divide_binary_expression_t>& expr) -> ast::var_name_t {
            throw std::runtime_error("Times divide binary expression forms an invalid lvalue.");
            return "";
        },
        [](const ast::factor_t& expr) -> ast::var_name_t {
            return validate_lvalue_expression_factor(expr);
        }
    }, expr);
}

ast::var_name_t validate_lvalue_expression_plus_minus(const ast::plus_minus_expression_t& expr) {
    return std::visit(overloaded{
        [](const std::shared_ptr<ast::plus_minus_binary_expression_t>& expr) -> ast::var_name_t {
            throw std::runtime_error("Plus minus binary expression forms an invalid lvalue.");
            return "";
        },
        [](const ast::times_divide_expression_t& expr) -> ast::var_name_t {
            return validate_lvalue_expression_times_divide(expr);
        }
    }, expr);
}

ast::var_name_t validate_lvalue_expression_relational_exp(const ast::relational_expression_t& expr) {
    return std::visit(overloaded{
        [](const std::shared_ptr<ast::relational_binary_expression_t>& expr) -> ast::var_name_t {
            throw std::runtime_error("Relational binary expression forms an invalid lvalue.");
            return "";
        },
        [](const ast::plus_minus_expression_t& expr) -> ast::var_name_t {
            return validate_lvalue_expression_plus_minus(expr);
        }
    }, expr);
}

ast::var_name_t validate_lvalue_expression_equality(const ast::equality_expression_t& expr) {
    return std::visit(overloaded{
        [](const std::shared_ptr<ast::equality_binary_expression_t>& expr) -> ast::var_name_t {
            throw std::runtime_error("Equality binary expression forms an invalid lvalue.");
            return "";
        },
        [](const ast::relational_expression_t& expr) -> ast::var_name_t {
            return validate_lvalue_expression_relational_exp(expr);
        }
    }, expr);
}

ast::var_name_t validate_lvalue_expression_logic_and(const ast::logical_and_expression_t& expr) {
    return std::visit(overloaded{
        [](const std::shared_ptr<ast::logical_and_binary_expression_t>& expr) -> ast::var_name_t {
            throw std::runtime_error("Logical and binary expression forms an invalid lvalue.");
            return "";
        },
        [](const ast::equality_expression_t& expr) -> ast::var_name_t {
            return validate_lvalue_expression_equality(expr);
        }
    }, expr);
}

ast::var_name_t validate_lvalue_expression_logic_or(const ast::logical_or_expression_t& expr) {
    return std::visit(overloaded{
        [](const std::shared_ptr<ast::logical_or_binary_expression_t>& expr) -> ast::var_name_t {
            throw std::runtime_error("Logical or binary expression forms an invalid lvalue.");
            return "";
        },
        [](const ast::logical_and_expression_t& expr) -> ast::var_name_t {
            return validate_lvalue_expression_logic_and(expr);
        }
    }, expr);
}

ast::var_name_t validate_lvalue_expression_assignment(const ast::assignment_t& assignment) {
    return assignment.var_name;
}

ast::var_name_t validate_lvalue_expression_exp(const ast::expression_t& expr) {
    return std::visit(overloaded{
        [](const std::shared_ptr<ast::assignment_t>& assignment) -> ast::var_name_t {
            return validate_lvalue_expression_assignment(*assignment);
        },
        [](const ast::logical_or_expression_t& expr) -> ast::var_name_t {
            return validate_lvalue_expression_logic_or(expr);
        }
    }, expr);
}










ast::unary_op_t convert_to_unary_token(token_t token) {
    switch(token.token_type) {
        case token_type_t::DASH:
            return ast::unary_op_t::NEG;
        case token_type_t::TILDE:
            return ast::unary_op_t::BITWISE_NOT;
        case token_type_t::BANG:
            return ast::unary_op_t::LOGIC_NOT;
    }
    throw std::runtime_error("Invalid unary operator");
}
ast::times_divide_t convert_to_times_divide_token(token_t token) {
    switch(token.token_type) {
        case token_type_t::ASTERISK:
            return ast::times_divide_t::TIMES;
        case token_type_t::SLASH:
            return ast::times_divide_t::DIVIDE;
    }
    throw std::runtime_error("Not either `*` or `/`");
}
ast::plus_minus_t convert_to_plus_minus_token(token_t token) {
    switch(token.token_type) {
        case token_type_t::PLUS:
            return ast::plus_minus_t::PLUS;
        case token_type_t::DASH:
            return ast::plus_minus_t::MINUS;
    }
    throw std::runtime_error("Not either `+` or `-`");
}
ast::relational_t convert_to_relational_token(token_t token) {
    switch(token.token_type) {
        case token_type_t::LESS_THAN:
            return ast::relational_t::LESS_THAN;
        case token_type_t::GREATER_THAN:
            return ast::relational_t::GREATER_THAN;
        case token_type_t::LESS_THAN_EQUAL:
            return ast::relational_t::LESS_THAN_EQUAL;
        case token_type_t::GREATER_THAN_EQUAL:
            return ast::relational_t::GREATER_THAN_EQUAL;
    }
    throw std::runtime_error("Not either `<`, `>`, `<=`, or `>=`");
}
ast::equality_t convert_to_equality_token(token_t token) {
    switch(token.token_type) {
        case token_type_t::EQUAL_EQUAL:
            return ast::equality_t::EQUAL_EQUAL;
        case token_type_t::NOT_EQUAL:
            return ast::equality_t::NOT_EQUAL;
    }
    throw std::runtime_error("Not either `==` or `!=`");
}
ast::logical_and_t convert_to_logical_and_token(token_t token) {
    if(token.token_type == token_type_t::LOGIC_AND) {
        return ast::logical_and_t::AND;
    }
    throw std::runtime_error("Not `&&`");
}
ast::logical_or_t convert_to_logical_or_token(token_t token) {
    if(token.token_type == token_type_t::LOGIC_OR) {
        return ast::logical_or_t::OR;
    }
    throw std::runtime_error("Not `||`");
}

std::shared_ptr<ast::grouping_t> make_grouping(ast::expression_t expr) {
    return std::make_shared<ast::grouping_t>( ast::grouping_t { std::move(expr) } );
}
std::shared_ptr<ast::unary_op_expression_t> make_unary_expression(ast::unary_op_t op, ast::factor_t factor) {
    return std::make_shared<ast::unary_op_expression_t>( ast::unary_op_expression_t { op, std::move(factor) } );
}
std::shared_ptr<ast::times_divide_binary_expression_t> make_times_divide_binary_expression(ast::times_divide_expression_t first_param, ast::times_divide_t op, ast::times_divide_expression_t second_param) {
    return std::make_shared<ast::times_divide_binary_expression_t>( ast::times_divide_binary_expression_t { op, std::move(first_param), std::move(second_param) } );
}
std::shared_ptr<ast::plus_minus_binary_expression_t> make_plus_minus_binary_expression(ast::plus_minus_expression_t first_param, ast::plus_minus_t op, ast::plus_minus_expression_t second_param) {
    return std::make_shared<ast::plus_minus_binary_expression_t>( ast::plus_minus_binary_expression_t { op, std::move(first_param), std::move(second_param) } );
}
std::shared_ptr<ast::relational_binary_expression_t> make_relational_binary_expression(ast::relational_expression_t first_param, ast::relational_t op, ast::relational_expression_t second_param) {
    return std::make_shared<ast::relational_binary_expression_t>( ast::relational_binary_expression_t { op, std::move(first_param), std::move(second_param) } );
}
std::shared_ptr<ast::equality_binary_expression_t> make_equality_binary_expression(ast::equality_expression_t first_param, ast::equality_t op, ast::equality_expression_t second_param) {
    return std::make_shared<ast::equality_binary_expression_t>( ast::equality_binary_expression_t { op, std::move(first_param), std::move(second_param) } );
}
std::shared_ptr<ast::logical_and_binary_expression_t> make_logical_and_binary_expression(ast::logical_and_expression_t first_param, ast::logical_and_t op, ast::logical_and_expression_t second_param) {
    return std::make_shared<ast::logical_and_binary_expression_t>( ast::logical_and_binary_expression_t { op, std::move(first_param), std::move(second_param) } );
}
std::shared_ptr<ast::logical_or_binary_expression_t> make_logical_or_binary_expression(ast::logical_or_expression_t first_param, ast::logical_or_t op, ast::logical_or_expression_t second_param) {
    return std::make_shared<ast::logical_or_binary_expression_t>( ast::logical_or_binary_expression_t { op, std::move(first_param), std::move(second_param) } );
}
std::shared_ptr<ast::assignment_t> make_assignment_expression(ast::expression_t first_param, ast::expression_t second_param) {
    // TODO: validate that `first_param` is a valid lvalue expression
    return std::make_shared<ast::assignment_t>( ast::assignment_t { validate_lvalue_expression_exp(first_param), std::move(second_param) } );
}

ast::expression_t make_constant_expr(const int value) {
    return ast::expression_t {
        ast::logical_or_expression_t {
            ast::logical_and_expression_t {
                ast::equality_expression_t {
                    ast::relational_expression_t {
                        ast::plus_minus_expression_t {
                            ast::times_divide_expression_t {
                                ast::factor_t {
                                    ast::constant_t {
                                        value
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    };
}
