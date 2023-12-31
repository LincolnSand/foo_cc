#include "type_checker.hpp"


bool is_arithmetic(const ast::type_t& lhs) {
    switch(lhs.type_category) {
        case ast::type_category_t::INT:
        case ast::type_category_t::UNSIGNED_INT:
        case ast::type_category_t::FLOATING:
            return true;
    }
    return false;
}
bool is_convertible(const ast::type_t& lhs, const ast::type_t& rhs) {
    if(lhs.type_category == rhs.type_category) { // TODO: this will give false positives for structs
        return true;
    }

    if(is_arithmetic(lhs) && is_arithmetic(rhs)) {
        return true;
    }

    // TODO: check if lhs and rhs are typedefs/type aliases of each other

    return false;
}

void type_check_unary_expression(std::optional<ast::type_t>& type, ast::unary_expression_t& unary_exp) {
    switch(unary_exp.op) {
        case ast::unary_operator_token_t::PLUS_PLUS:
        case ast::unary_operator_token_t::MINUS_MINUS:
            if(!is_integral(unary_exp.exp.type.value())) {
                throw std::runtime_error("`++` and `--` are only supported for integer and unsigned integer types.");
            }
            type = unary_exp.exp.type.value();
            break;
        case ast::unary_operator_token_t::PLUS:
        case ast::unary_operator_token_t::MINUS:
            if(is_arithmetic(unary_exp.exp.type.value())) {
                type = unary_exp.exp.type.value();
            } else {
                throw std::runtime_error("Unary `+` and `-` are only supported for primitive types.");
            }
            break;
        case ast::unary_operator_token_t::LOGICAL_NOT: // TODO: add support for bools
            if(is_arithmetic(unary_exp.exp.type.value())) {
                type = make_primitive_type_t(ast::type_category_t::INT, "int", sizeof(std::int32_t), alignof(std::int32_t));
            } else {
                throw std::runtime_error("Logical not is only supported for primitive types.");
            }
            break;
        case ast::unary_operator_token_t::BITWISE_NOT:
            if(is_integral(unary_exp.exp.type.value())) {
                throw std::runtime_error("`++` and `--` are only supported for integer types.");
            }
            type = unary_exp.exp.type.value();
            break;
        default:
            throw std::logic_error("Invalid unary operator.");
    }
}
void type_check_binary_expression(std::optional<ast::type_t>& type, ast::binary_expression_t& binary_exp) {
    switch(binary_exp.op) {
        case ast::binary_operator_token_t::MULTIPLY:
        case ast::binary_operator_token_t::DIVIDE:
        case ast::binary_operator_token_t::PLUS:
        case ast::binary_operator_token_t::MINUS:
            // TODO: double check this logic
            if(is_arithmetic(binary_exp.left.type.value()) && is_arithmetic(binary_exp.right.type.value())) {
                if(compare_type_names(binary_exp.left.type.value(), binary_exp.right.type.value())) {
                    type = binary_exp.left.type.value();
                } else if(binary_exp.left.type.value().type_category == ast::type_category_t::FLOATING) { // TODO: refactor and remove code duplication by checking if left.type.type_category == right.type.type_category
                    if(binary_exp.right.type.value().type_category == ast::type_category_t::FLOATING) {
                        if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                            type = binary_exp.left.type.value();
                            binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                        } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                            type = binary_exp.right.type.value();
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                        } else {
                            type = binary_exp.left.type.value();
                        }
                    } else {
                        type = binary_exp.left.type.value();
                        binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                    }
                } else if(binary_exp.right.type.value().type_category == ast::type_category_t::FLOATING) {
                    if(binary_exp.left.type.value().type_category == ast::type_category_t::FLOATING) {
                        if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                            type = binary_exp.left.type.value();
                            binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                        } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                            type = binary_exp.right.type.value();
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                        } else {
                            type = binary_exp.left.type.value();
                        }
                    } else {
                        type = binary_exp.right.type.value();
                        binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                    }
                } else if(binary_exp.left.type.value().type_category == ast::type_category_t::UNSIGNED_INT) {
                    if(binary_exp.right.type.value().type_category == ast::type_category_t::UNSIGNED_INT) {
                        if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                            type = binary_exp.left.type.value();
                            binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                        } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                            type = binary_exp.right.type.value();
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                        } else {
                            type = binary_exp.left.type.value();
                        }
                    } else {
                        assert(binary_exp.right.type.value().type_category == ast::type_category_t::INT);
                        if(binary_exp.left.type.value().size >= binary_exp.right.type.value().size) {
                            type = binary_exp.left.type.value();
                            binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                        } else {
                            // If the signed type can represent all values of the unsigned type, then the operand with the unsigned type is implicitly converted to the signed type.
                            //   Else, both operands undergo implicit conversion to the unsigned type counterpart of the signed operand's type.
                            // -- https://en.cppreference.com/w/c/language/conversion

                            // I don't know how that "Else," clause could ever be triggered since if the signed type has even one more bit, it can represent all values of the unsigned type,
                            //  so ostensibly if the size of the signed type is strictly greater than the size of the unsigned type, then the signed type can represent all values of the unsigned type since size is in bytes.
                            //  But maybe I am missing something and it is something I need to figure out, but for right now, I am acting as if that "Else," clause is unreachable.
                            type = binary_exp.right.type.value();
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                        }
                    }
                } else if(binary_exp.right.type.value().type_category == ast::type_category_t::UNSIGNED_INT) {
                    if(binary_exp.left.type.value().type_category == ast::type_category_t::UNSIGNED_INT) {
                        if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                            type = binary_exp.left.type.value();
                            binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                        } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                            type = binary_exp.right.type.value();
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                        } else {
                            type = binary_exp.left.type.value();
                        }
                    } else {
                        assert(binary_exp.left.type.value().type_category == ast::type_category_t::INT);
                        if(binary_exp.right.type.value().size >= binary_exp.left.type.value().size) {
                            type = binary_exp.right.type.value();
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                        } else {
                            // See explanation above.
                            type = binary_exp.left.type.value();
                            binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                        }
                    }
                } else if(binary_exp.left.type.value().type_category == ast::type_category_t::INT && binary_exp.right.type.value().type_category == ast::type_category_t::INT) {
                    if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                        type = binary_exp.left.type.value();
                        binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                    } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                        type = binary_exp.right.type.value();
                        binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                    } else {
                        type = binary_exp.left.type.value();
                    }
                } else {
                    throw std::runtime_error("Unsupported types used for binary operator.");
                }
            } else {
                throw std::runtime_error("Types provided to binary operator are not arithmetic.");
            }
            break;


        case ast::binary_operator_token_t::MODULO:
            if(is_integral(binary_exp.left.type.value()) && is_integral(binary_exp.right.type.value())) {
                if((binary_exp.left.type.value().type_category == binary_exp.right.type.value().type_category)) {
                    if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                        type = binary_exp.left.type.value();
                        binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                    } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                        type = binary_exp.right.type.value();
                        binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                    } else {
                        type = binary_exp.left.type.value();
                    }
                } else if(binary_exp.left.type.value().type_category == ast::type_category_t::UNSIGNED_INT) {
                    assert(binary_exp.right.type.value().type_category == ast::type_category_t::INT);
                    if(binary_exp.left.type.value().size >= binary_exp.right.type.value().size) {
                        type = binary_exp.left.type.value();
                        binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                    } else {
                        type = binary_exp.right.type.value();
                        binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                    }
                } else {
                    assert(binary_exp.right.type.value().type_category == ast::type_category_t::UNSIGNED_INT);
                    assert(binary_exp.left.type.value().type_category == ast::type_category_t::INT);
                    if(binary_exp.right.type.value().size >= binary_exp.left.type.value().size) {
                        type = binary_exp.right.type.value();
                        binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                    } else {
                        type = binary_exp.left.type.value();
                        binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                    }
                }
            } else {
                throw std::runtime_error("Unsupported types used for modulo binary operator.");
            }
            break;


        case ast::binary_operator_token_t::LEFT_BITSHIFT:
        case ast::binary_operator_token_t::RIGHT_BITSHIFT:
            if(is_integral(binary_exp.left.type.value()) && is_integral(binary_exp.right.type.value())) {
                type = binary_exp.left.type.value();
            } else {
                throw std::runtime_error("Unsupported types used for bitshift operator.");
            }
            break;

        case ast::binary_operator_token_t::BITWISE_AND:
        case ast::binary_operator_token_t::BITWISE_XOR:
        case ast::binary_operator_token_t::BITWISE_OR:
            if(is_integral(binary_exp.left.type.value()) && is_integral(binary_exp.right.type.value())) {
                if(compare_type_names(binary_exp.left.type.value(), binary_exp.right.type.value())) {
                    type = binary_exp.left.type.value();
                } else if(binary_exp.left.type.value().type_category == binary_exp.right.type.value().type_category) {
                    if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                        type = binary_exp.left.type.value();
                        binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                    } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                        type = binary_exp.right.type.value();
                        binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                    } else {
                        type = binary_exp.left.type.value();
                    }
                } else if(binary_exp.left.type.value().type_category == ast::type_category_t::UNSIGNED_INT) {
                    assert(binary_exp.right.type.value().type_category == ast::type_category_t::INT);
                    if(binary_exp.left.type.value().size >= binary_exp.right.type.value().size) {
                        type = binary_exp.left.type.value();
                        binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                    } else {
                        type = binary_exp.right.type.value();
                        binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                    }
                } else {
                    assert(binary_exp.right.type.value().type_category == ast::type_category_t::UNSIGNED_INT);
                    assert(binary_exp.left.type.value().type_category == ast::type_category_t::INT);
                    if(binary_exp.right.type.value().size >= binary_exp.left.type.value().size) {
                        type = binary_exp.right.type.value();
                        binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                    } else {
                        type = binary_exp.left.type.value();
                        binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                    }
                }
            } else {
                throw std::runtime_error("Unsupported types used for bitwise operator.");
            }
            break;


        case ast::binary_operator_token_t::LESS_THAN:
        case ast::binary_operator_token_t::LESS_THAN_EQUAL:
        case ast::binary_operator_token_t::GREATER_THAN:
        case ast::binary_operator_token_t::GREATER_THAN_EQUAL:
        case ast::binary_operator_token_t::EQUAL:
        case ast::binary_operator_token_t::NOT_EQUAL:
            if(is_arithmetic(binary_exp.left.type.value()) && is_arithmetic(binary_exp.right.type.value())) {
                // TODO: support booleans
                type = make_primitive_type_t(ast::type_category_t::INT, "int", sizeof(std::int32_t), alignof(std::int32_t));

                if(!compare_type_names(binary_exp.left.type.value(), binary_exp.right.type.value())) {
                    if(binary_exp.left.type.value().type_category == ast::type_category_t::FLOATING) {
                        if(binary_exp.right.type.value().type_category == ast::type_category_t::FLOATING) {
                            if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                                binary_exp.right = make_convert_t(std::move(binary_exp.right), binary_exp.left.type.value());
                            } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                                binary_exp.left = make_convert_t(std::move(binary_exp.left), binary_exp.right.type.value());
                            }
                        } else {
                            binary_exp.right = make_convert_t(std::move(binary_exp.right), binary_exp.left.type.value());
                        }
                    } else if(binary_exp.right.type.value().type_category == ast::type_category_t::FLOATING) {
                        if(binary_exp.left.type.value().type_category == ast::type_category_t::FLOATING) {
                            if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                                binary_exp.right = make_convert_t(std::move(binary_exp.right), binary_exp.left.type.value());
                            } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                                binary_exp.left = make_convert_t(std::move(binary_exp.left), binary_exp.right.type.value());
                            }
                        } else {
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), binary_exp.right.type.value());
                        }
                    } else if(binary_exp.left.type.value().type_category == binary_exp.right.type.value().type_category) {
                        if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                            binary_exp.right = make_convert_t(std::move(binary_exp.right), binary_exp.left.type.value());
                        } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), binary_exp.right.type.value());
                        } else {
                            // same size and same category of type, but technically different, arbitrarily convert to one of them so the types match
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), binary_exp.right.type.value());
                        }
                    } else if(binary_exp.left.type.value().type_category == ast::type_category_t::UNSIGNED_INT) {
                        assert(binary_exp.right.type.value().type_category == ast::type_category_t::INT);
                        if(binary_exp.left.type.value().size >= binary_exp.right.type.value().size) {
                            binary_exp.right = make_convert_t(std::move(binary_exp.right), binary_exp.left.type.value());
                        } else {
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), binary_exp.right.type.value());
                        }
                    } else {
                        assert(binary_exp.right.type.value().type_category == ast::type_category_t::UNSIGNED_INT);
                        assert(binary_exp.left.type.value().type_category == ast::type_category_t::INT);
                        if(binary_exp.right.type.value().size >= binary_exp.left.type.value().size) {
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), binary_exp.right.type.value());
                        } else {
                            binary_exp.right = make_convert_t(std::move(binary_exp.right), binary_exp.left.type.value());
                        }
                    }
                }
            } else {
                throw std::runtime_error("Unsupported types used for relational binary operator.");
            }
            break;


        case ast::binary_operator_token_t::LOGICAL_AND:
        case ast::binary_operator_token_t::LOGICAL_OR:
            if(is_arithmetic(binary_exp.left.type.value()) && is_arithmetic(binary_exp.right.type.value())) {
                type = make_primitive_type_t(ast::type_category_t::INT, "int", sizeof(std::int32_t), alignof(std::int32_t));

                if(binary_exp.left.type.value().type_name != "int") {
                    binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
                }
                if(binary_exp.right.type.value().type_name != "int") {
                    binary_exp.right = make_convert_t(std::move(binary_exp.right), type.value());
                }
            } else {
                throw std::runtime_error("Unsupported types used for logical binary operator.");
            }
            break;


        case ast::binary_operator_token_t::ASSIGNMENT:
            if(is_convertible(binary_exp.left.type.value(), binary_exp.right.type.value())) {
                type = binary_exp.left.type.value();
                if(!compare_type_names(binary_exp.left.type.value(), binary_exp.right.type.value())) {
                    binary_exp.right = make_convert_t(std::move(binary_exp.right), binary_exp.left.type.value());
                }
            } else {
                throw std::runtime_error("ASSIGNMENT: Cannot convert from type [" + binary_exp.left.type.value().type_name + "] to type [" + binary_exp.right.type.value().type_name + "].");
            }
            break;


        case ast::binary_operator_token_t::COMMA:
            if(!is_convertible(binary_exp.left.type.value(), binary_exp.right.type.value())) {
                throw std::runtime_error("COMMA: Cannot convert from type [" + binary_exp.left.type.value().type_name + "] to type [" + binary_exp.right.type.value().type_name + "].");
            }
            type = binary_exp.right.type.value();
            if(!compare_type_names(binary_exp.left.type.value(), binary_exp.right.type.value())) {
                binary_exp.left = make_convert_t(std::move(binary_exp.left), type.value());
            }
            break;


        default:
            throw std::logic_error("Unimplemented or unsupported binary operator.");
    }
}
void type_check_ternary_expression(std::optional<ast::type_t>& type, ast::ternary_expression_t& ternary_exp) {
    if(!is_convertible(ternary_exp.condition.type.value(), make_primitive_type_t(ast::type_category_t::INT, "int", sizeof(std::int32_t), alignof(std::int32_t)))) {
        throw std::runtime_error("Condition of ternary expression is of type: [" + ternary_exp.condition.type.value().type_name + "], which is not truthy.");
    }
    if(!compare_type_names(ternary_exp.condition.type.value(), make_primitive_type_t(ast::type_category_t::INT, "int", sizeof(std::int32_t), alignof(std::int32_t)))) {
        ternary_exp.condition = make_convert_t(std::move(ternary_exp.condition), make_primitive_type_t(ast::type_category_t::INT, "int", sizeof(std::int32_t), alignof(std::int32_t)));
    }

    if(is_convertible(ternary_exp.if_true.type.value(), ternary_exp.if_true.type.value())) {
        if(compare_type_names(ternary_exp.if_true.type.value(), ternary_exp.if_true.type.value())) {
            type = ternary_exp.if_true.type.value();
        } else if(ternary_exp.if_true.type.value().type_category == ternary_exp.if_false.type.value().type_category) {
            if(ternary_exp.if_true.type.value().size >= ternary_exp.if_false.type.value().size) {
                type = ternary_exp.if_true.type.value();
                ternary_exp.if_false = make_convert_t(std::move(ternary_exp.if_false), type.value());
            } else {
                type = ternary_exp.if_false.type.value();
                ternary_exp.if_true = make_convert_t(std::move(ternary_exp.if_true), type.value());
            }
        } else if(ternary_exp.if_true.type.value().type_category == ast::type_category_t::FLOATING) {
            type = ternary_exp.if_true.type.value();
            ternary_exp.if_false = make_convert_t(std::move(ternary_exp.if_false), type.value());
        } else if(ternary_exp.if_false.type.value().type_category == ast::type_category_t::FLOATING) {
            type = ternary_exp.if_false.type.value();
            ternary_exp.if_true = make_convert_t(std::move(ternary_exp.if_true), type.value());
        } else if(ternary_exp.if_true.type.value().type_category == ast::type_category_t::UNSIGNED_INT) {
            assert(ternary_exp.if_false.type.value().type_category == ast::type_category_t::INT);
            if(ternary_exp.if_true.type.value().size >= ternary_exp.if_false.type.value().size) {
                type = ternary_exp.if_true.type.value();
                ternary_exp.if_false = make_convert_t(std::move(ternary_exp.if_false), type.value());
            } else {
                type = ternary_exp.if_false.type.value();
                ternary_exp.if_true = make_convert_t(std::move(ternary_exp.if_true), type.value());
            }
        } else if(ternary_exp.if_false.type.value().type_category == ast::type_category_t::UNSIGNED_INT) {
            assert(ternary_exp.if_true.type.value().type_category == ast::type_category_t::INT);
            if(ternary_exp.if_false.type.value().size >= ternary_exp.if_true.type.value().size) {
                type = ternary_exp.if_false.type.value();
                ternary_exp.if_true = make_convert_t(std::move(ternary_exp.if_true), type.value());
            } else {
                type = ternary_exp.if_true.type.value();
                ternary_exp.if_false = make_convert_t(std::move(ternary_exp.if_false), type.value());
            }
        } else {
            throw std::logic_error("Unsupported types used for ternary operator body.");
        }
    } else {
        throw std::runtime_error("Body of ternary expression types are not convertible.");
    }
}

void type_check_expression(ast::expression_t& expression) {
    std::visit(overloaded{
        [&expression](const std::shared_ptr<ast::grouping_t>& grouping_exp) {
            type_check_expression(grouping_exp->expr);
            expression.type = grouping_exp->expr.type.value();
        },
        [&expression](const std::shared_ptr<ast::convert_t>& convert_exp) {
            throw std::runtime_error("User casts not yet supported.");
        },
        [&expression](const std::shared_ptr<ast::unary_expression_t>& unary_exp) {
            type_check_expression(unary_exp->exp);
            type_check_unary_expression(expression.type, *unary_exp);
        },
        [&expression](const std::shared_ptr<ast::binary_expression_t>& binary_exp) {
            type_check_expression(binary_exp->left);
            type_check_expression(binary_exp->right);
            type_check_binary_expression(expression.type, *binary_exp);
        },
        [&expression](const std::shared_ptr<ast::ternary_expression_t>& ternary_exp) {
            type_check_expression(ternary_exp->condition);
            type_check_expression(ternary_exp->if_true);
            type_check_expression(ternary_exp->if_false);
            type_check_ternary_expression(expression.type, *ternary_exp);
        },
        [](const std::shared_ptr<ast::function_call_t>& function_call_exp) {
            for(auto& param : function_call_exp->params) {
                type_check_expression(param);
            }
            // No need to set the type of the current expression since function calls are root expressions (aside from the expressions being passed as parameters)

            // TODO: We should type check function call expression params *after* they have been type checked instead of before.
            // TODO: Currently you cannot put expressions as function call params that don't exactly match the type of the function param and
            // TODO:  which aren't `ast::function_call_t`, `ast::constant_t`, or `ast::variable_access_t` (other types of expressions are not yet typed and are thus currently unsupported).
        },
        [](const ast::constant_t& constant_exp) {
            // Nothing to do as this is a root expression
        },
        [](const ast::variable_access_t& var_name_exp) {
            // Nothing to do as this is a root expression
        }
    }, expression.expr);
}
void type_check_statement(ast::statement_t& statement, const ast::type_t& function_return_type) {
    std::visit(overloaded{
        [&function_return_type](ast::return_statement_t& statement) {
            type_check_expression(statement.expr);
            if(is_convertible(function_return_type, statement.expr.type.value())) {
                if(!compare_type_names(statement.expr.type.value(), function_return_type)) {
                    statement.expr = make_convert_t(std::move(statement.expr), function_return_type);
                }
            } else {
                throw std::runtime_error("RETURN: Cannot convert from type [" + statement.expr.type.value().type_name + "] to type [" + function_return_type.type_name + "].");
            }
        },
        [](ast::expression_statement_t& statement) {
            if(statement.expr.has_value()) {
                type_check_expression(statement.expr.value());
            }
        },
        [&function_return_type](std::shared_ptr<ast::if_statement_t>& statement) {
            type_check_expression(statement->if_exp);
            type_check_statement(statement->if_body, function_return_type);
            if(statement->else_body.has_value()) {
                type_check_statement(statement->else_body.value(), function_return_type);
            }
        },
        [&function_return_type](std::shared_ptr<ast::compound_statement_t>& statement) {
            type_check_compound_statement(*statement, function_return_type);
        }
    }, statement);
}
void type_check_declaration(ast::declaration_t& declaration) {
    if(declaration.value.has_value()) {
        type_check_expression(declaration.value.value());

        if(is_convertible(declaration.type_name, declaration.value.value().type.value())) {
            if(!compare_type_names(declaration.type_name, declaration.value.value().type.value())) {
                declaration.value = make_convert_t(std::move(declaration.value.value()), declaration.type_name);
            }
        } else {
            throw std::runtime_error("DECLARATION: Cannot convert from type [" + declaration.value.value().type.value().type_name + "] to type [" + declaration.type_name.type_name + "].");
        }
    }
}
void type_check_compound_statement(ast::compound_statement_t& compound_statement, const ast::type_t function_return_type) {
    for(auto& stmt : compound_statement.stmts) {
        std::visit(overloaded{
            [&function_return_type](ast::statement_t& statement) {
                type_check_statement(statement, function_return_type);
            },
            [](ast::declaration_t& declaration) {
                type_check_declaration(declaration);
            }
        }, stmt);
    }
}
void type_check_function_definition(ast::function_definition_t& function_definition) {
    type_check_compound_statement(function_definition.statements, function_definition.return_type);
}
ast::type_t get_type_from_constant_value(const ast::constant_t& value) {
    return std::visit(overloaded{
        [](char c) {
            return make_primitive_type_t(ast::type_category_t::INT, "char", sizeof(char), alignof(char));
        },
        [](signed char c) {
            return make_primitive_type_t(ast::type_category_t::INT, "signed char", sizeof(signed char), alignof(signed char));
        },
        [](unsigned char c) {
            return make_primitive_type_t(ast::type_category_t::UNSIGNED_INT, "unsigned char", sizeof(unsigned char), alignof(unsigned char));
        },
        [](short s) {
            return make_primitive_type_t(ast::type_category_t::INT, "short", sizeof(short), alignof(short));
        },
        [](unsigned short s) {
            return make_primitive_type_t(ast::type_category_t::UNSIGNED_INT, "unsigned short", sizeof(unsigned short), alignof(unsigned short));
        },
        [](int i) {
            return make_primitive_type_t(ast::type_category_t::INT, "int", sizeof(int), alignof(int));
        },
        [](unsigned int i) {
            return make_primitive_type_t(ast::type_category_t::UNSIGNED_INT, "unsigned int", sizeof(unsigned int), alignof(unsigned int));
        },
        [](long l) {
            return make_primitive_type_t(ast::type_category_t::INT, "long", sizeof(long), alignof(long));
        },
        [](unsigned long l) {
            return make_primitive_type_t(ast::type_category_t::UNSIGNED_INT, "unsigned long", sizeof(unsigned long), alignof(unsigned long));
        },
        [](long long ll) {
            return make_primitive_type_t(ast::type_category_t::INT, "long long", sizeof(long long), alignof(long long));
        },
        [](unsigned long long ll) {
            return make_primitive_type_t(ast::type_category_t::UNSIGNED_INT, "unsigned long long", sizeof(unsigned long long), alignof(unsigned long long));
        },
        [](float f) {
            return make_primitive_type_t(ast::type_category_t::FLOATING, "float", sizeof(float), alignof(float));
        },
        [](double d) {
            return make_primitive_type_t(ast::type_category_t::FLOATING, "double", sizeof(double), alignof(double));
        },
        [](long double ld) {
            return make_primitive_type_t(ast::type_category_t::FLOATING, "long double", sizeof(long double), alignof(long double));
        },
        [](const auto&) {
            throw std::logic_error("Unsupported constant type.");
        }
    }, value.value);
}
void type_check(ast::validated_program_t& validated_program) {
    for(auto& e : validated_program.top_level_declarations) {
        std::visit(overloaded{
            [](ast::function_definition_t& function_definition) {
                type_check_function_definition(function_definition);
            },
            [](ast::global_variable_declaration_t& global_var_def) {
                // TODO: move compile time evaluation of global variables to here from `validate_ast.cpp`
                if(global_var_def.value.has_value()) {
                    if(!global_var_def.value.value().type.has_value()) {
                        auto expression_value = evaluate_expression(global_var_def.value.value());
                        ast::type_t expression_type = get_type_from_constant_value(expression_value);
                        global_var_def.value.value() = ast::expression_t{std::move(expression_value), std::move(expression_type)};
                    }
                    if(is_convertible(global_var_def.type_name, global_var_def.value.value().type.value())) {
                        if(!compare_type_names(global_var_def.type_name, global_var_def.value.value().type.value())) {
                            global_var_def.value = make_convert_t(std::move(global_var_def.value.value()), global_var_def.type_name);
                        }
                    } else {
                        throw std::runtime_error("DECLARATION: Cannot convert from type [" + global_var_def.value.value().type.value().type_name + "] to type [" + global_var_def.type_name.type_name + "].");
                    }
                }
            }
        }, e);
    }
}
