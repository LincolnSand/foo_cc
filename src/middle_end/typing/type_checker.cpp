#include "type_checker.hpp"


bool is_convertible(const ast::type_name_t& lhs, const ast::type_name_t& rhs) {
    if(compare_type_names(lhs, rhs)) {
        return true;
    }

    if(lhs.token_type == ast::type_category_t::DOUBLE && rhs.token_type == ast::type_category_t::INT) {
        return true;
    }
    if(lhs.token_type == ast::type_category_t::INT && rhs.token_type == ast::type_category_t::DOUBLE) {
        return true;
    }
    // TODO: check if lhs and rhs are typedefs/type aliases of each other

    return false;
}

void type_check_unary_expression(std::optional<ast::type_name_t>& type, ast::unary_expression_t& unary_exp) {
    switch(unary_exp.op) {
        case ast::unary_operator_token_t::PLUS_PLUS:
        case ast::unary_operator_token_t::MINUS_MINUS:
            if(unary_exp.exp.type.value().token_type != ast::type_category_t::INT) {
                throw std::runtime_error("`++` and `--` are only supported for integer types.");
            }
            type = unary_exp.exp.type.value();
            break;
        case ast::unary_operator_token_t::PLUS:
        case ast::unary_operator_token_t::MINUS:
            if(unary_exp.exp.type.value().token_type != ast::type_category_t::INT || unary_exp.exp.type.value().token_type == ast::type_category_t::DOUBLE) {
                type = unary_exp.exp.type.value();
            } else {
                throw std::runtime_error("Unary `+` and `-` are only supported for primitive types.");
            }
            break;
        case ast::unary_operator_token_t::LOGICAL_NOT: // TODO: add support for bools
            if(unary_exp.exp.type.value().token_type != ast::type_category_t::INT || unary_exp.exp.type.value().token_type == ast::type_category_t::DOUBLE) {
                type = ast::type_name_t{ast::type_category_t::INT, "int", sizeof(std::uint64_t), sizeof(std::uint64_t)};
            } else {
                throw std::runtime_error("Logical not is only supported for primitive types.");
            }
            break;
        case ast::unary_operator_token_t::BITWISE_NOT:
            if(unary_exp.exp.type.value().token_type != ast::type_category_t::INT) {
                throw std::runtime_error("`++` and `--` are only supported for integer types.");
            }
            type = unary_exp.exp.type.value();
            break;
        default:
            throw std::logic_error("Invalid unary operator.");
    }
}
void type_check_binary_expression(std::optional<ast::type_name_t>& type, ast::binary_expression_t& binary_exp) {
    switch(binary_exp.op) {
        case ast::binary_operator_token_t::MULTIPLY:
        case ast::binary_operator_token_t::DIVIDE:
        case ast::binary_operator_token_t::PLUS:
        case ast::binary_operator_token_t::MINUS:
            // TODO: double check this logic
            if(compare_type_names(binary_exp.left.type.value(), binary_exp.right.type.value())) {
                type = binary_exp.left.type.value();
            } else if(binary_exp.left.type.value().token_type == ast::type_category_t::DOUBLE) { // TODO: refactor and remove code duplication by checking if left.type.token_type == right.type.token_type
                if(binary_exp.right.type.value().token_type == ast::type_category_t::DOUBLE) {
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
            } else if(binary_exp.right.type.value().token_type == ast::type_category_t::DOUBLE) {
                if(binary_exp.left.type.value().token_type == ast::type_category_t::DOUBLE) {
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
            } else if(binary_exp.left.type.value().token_type == ast::type_category_t::INT && binary_exp.right.type.value().token_type == ast::type_category_t::INT) {
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
            break;


        case ast::binary_operator_token_t::MODULO:
            if(binary_exp.left.type.value().token_type == ast::type_category_t::INT && binary_exp.right.type.value().token_type == ast::type_category_t::INT) {
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
            break;


        case ast::binary_operator_token_t::LEFT_BITSHIFT:
        case ast::binary_operator_token_t::RIGHT_BITSHIFT:
        case ast::binary_operator_token_t::BITWISE_AND:
        case ast::binary_operator_token_t::BITWISE_XOR:
        case ast::binary_operator_token_t::BITWISE_OR:
            if(binary_exp.left.type.value().token_type == ast::type_category_t::INT && binary_exp.right.type.value().token_type == ast::type_category_t::INT) {
                type = binary_exp.left.type.value();
            } else {
                throw std::runtime_error("Unsupported types used for binary operator.");
            }
            break;


        case ast::binary_operator_token_t::LESS_THAN:
        case ast::binary_operator_token_t::LESS_THAN_EQUAL:
        case ast::binary_operator_token_t::GREATER_THAN:
        case ast::binary_operator_token_t::GREATER_THAN_EQUAL:
        case ast::binary_operator_token_t::EQUAL:
        case ast::binary_operator_token_t::NOT_EQUAL:
            if((binary_exp.left.type.value().token_type == ast::type_category_t::INT || binary_exp.left.type.value().token_type == ast::type_category_t::DOUBLE) && (binary_exp.right.type.value().token_type == ast::type_category_t::INT || binary_exp.right.type.value().token_type == ast::type_category_t::DOUBLE)) {
                // TODO: support booleans
                type = ast::type_name_t{ast::type_category_t::INT, "int", sizeof(std::uint64_t), sizeof(std::uint64_t)};

                if(!compare_type_names(binary_exp.left.type.value(), binary_exp.right.type.value())) {
                    if(binary_exp.left.type.value().token_type == ast::type_category_t::DOUBLE) {
                        if(binary_exp.right.type.value().token_type == ast::type_category_t::DOUBLE) {
                            if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                                binary_exp.right = make_convert_t(std::move(binary_exp.right), binary_exp.left.type.value());
                            } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                                binary_exp.left = make_convert_t(std::move(binary_exp.left), binary_exp.right.type.value());
                            }
                        } else {
                            binary_exp.right = make_convert_t(std::move(binary_exp.right), binary_exp.left.type.value());
                        }
                    } else if(binary_exp.right.type.value().token_type == ast::type_category_t::DOUBLE) {
                        if(binary_exp.left.type.value().token_type == ast::type_category_t::DOUBLE) {
                            if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                                binary_exp.right = make_convert_t(std::move(binary_exp.right), binary_exp.left.type.value());
                            } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                                binary_exp.left = make_convert_t(std::move(binary_exp.left), binary_exp.right.type.value());
                            }
                        } else {
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), binary_exp.right.type.value());
                        }
                    } else { // both are of category INT
                        if(binary_exp.left.type.value().size > binary_exp.right.type.value().size) {
                            binary_exp.right = make_convert_t(std::move(binary_exp.right), binary_exp.left.type.value());
                        } else if(binary_exp.left.type.value().size != binary_exp.right.type.value().size) {
                            binary_exp.left = make_convert_t(std::move(binary_exp.left), binary_exp.right.type.value());
                        }
                    }
                }
            } else {
                throw std::runtime_error("Unsupported types used for relational binary operator.");
            }
            break;


        case ast::binary_operator_token_t::LOGICAL_AND:
        case ast::binary_operator_token_t::LOGICAL_OR:
            if((binary_exp.left.type.value().token_type == ast::type_category_t::INT || binary_exp.left.type.value().token_type == ast::type_category_t::DOUBLE) && (binary_exp.right.type.value().token_type == ast::type_category_t::INT || binary_exp.right.type.value().token_type == ast::type_category_t::DOUBLE)) {
                type = ast::type_name_t{ast::type_category_t::INT, "int", sizeof(std::uint64_t), sizeof(std::uint64_t)};

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
                throw std::runtime_error("Cannot convert from type [" + binary_exp.left.type.value().type_name + "] to type [" + binary_exp.right.type.value().type_name + "].");
            }
            break;


        case ast::binary_operator_token_t::COMMA:
            if(!is_convertible(binary_exp.left.type.value(), binary_exp.right.type.value())) {
                throw std::runtime_error("Cannot convert from type [" + binary_exp.left.type.value().type_name + "] to type [" + binary_exp.right.type.value().type_name + "].");
            }
            break;


        default:
            throw std::logic_error("Unimplemented or unsupported binary operator.");
    }
}
void type_check_ternary_expression(std::optional<ast::type_name_t>& type, ast::ternary_expression_t& ternary_exp) {
    if(!is_convertible(ternary_exp.condition.type.value(), ast::type_name_t{ast::type_category_t::INT, "int", sizeof(std::uint64_t), sizeof(std::uint64_t)})) {
        throw std::runtime_error("Condition of ternary expression is of type: [" + ternary_exp.condition.type.value().type_name + "], which is not truthy.");
    } 

    if(is_convertible(ternary_exp.if_true.type.value(), ternary_exp.if_true.type.value())) {
        if(compare_type_names(ternary_exp.if_true.type.value(), ternary_exp.if_true.type.value())) {
            type = ternary_exp.if_true.type.value();
        } else if(ternary_exp.if_true.type.value().token_type == ternary_exp.if_false.type.value().token_type) {
            if(ternary_exp.if_true.type.value().size >= ternary_exp.if_false.type.value().size) {
                type = ternary_exp.if_true.type.value();
                ternary_exp.if_false = make_convert_t(std::move(ternary_exp.if_false), type.value());
            } else {
                type = ternary_exp.if_false.type.value();
                ternary_exp.if_true = make_convert_t(std::move(ternary_exp.if_true), type.value());
            }
        } else if(ternary_exp.if_true.type.value().token_type == ast::type_category_t::DOUBLE) {
            type = ternary_exp.if_true.type.value();
            ternary_exp.if_false = make_convert_t(std::move(ternary_exp.if_false), type.value());
        } else if(ternary_exp.if_false.type.value().token_type == ast::type_category_t::DOUBLE) {
            type = ternary_exp.if_false.type.value();
            ternary_exp.if_true = make_convert_t(std::move(ternary_exp.if_true), type.value());
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
            // TODO:  which aren't `ast::function_call_t`, `ast::constant_t`, or `ast::var_name_t` (other types of expressions are not yet typed and are thus currently unsupported).
        },
        [](const ast::constant_t& constant_exp) {
            // Nothing to do as this is a root expression
        },
        [](const ast::var_name_t& var_name_exp) {
            // Nothing to do as this is a root expression
        }
    }, expression.expr);
}
void type_check_statement(ast::statement_t& statement, const ast::type_name_t& function_return_type) {
    std::visit(overloaded{
        [&function_return_type](ast::return_statement_t& statement) {
            if(is_convertible(function_return_type, statement.expr.type.value())) {
                if(!compare_type_names(statement.expr.type.value(), function_return_type)) {
                    statement.expr = make_convert_t(std::move(statement.expr), function_return_type);
                }
            } else {
                throw std::runtime_error("Cannot convert from type [" + statement.expr.type.value().type_name + "] to type [" + function_return_type.type_name + "].");
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
                declaration.value = ast::expression_t{ std::make_shared<ast::convert_t>(ast::convert_t{std::move(declaration.value.value())}), declaration.type_name };
            }
        } else {
            throw std::runtime_error("Cannot convert from type [" + declaration.value.value().type.value().type_name + "] to type [" + declaration.type_name.type_name + "].");
        }
    }
}
void type_check_compound_statement(ast::compound_statement_t& compound_statement, const ast::type_name_t function_return_type) {
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
void type_check(ast::validated_program_t& validated_program) {
    for(auto& e : validated_program.top_level_declarations) {
        std::visit(overloaded{
            [](ast::function_definition_t& function_definition) {
                type_check_function_definition(function_definition);
            },
            [](ast::validated_global_variable_definition_t& global_var_def) {
                // TODO: move compile time evaluation of global variables to here from `validate_ast.cpp`
            }
        }, e);
    }
}
