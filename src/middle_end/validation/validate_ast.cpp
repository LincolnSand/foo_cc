#include "validate_ast.hpp"


void validate_type_name(const ast::type_t& expected, const ast::type_t& actual, const std::string& error_message) {
    if(expected.token_type != actual.token_type) { // optimization to avoid having to do string comparisons for built-in types
        if(expected.type_name != actual.type_name) {
            throw std::runtime_error(error_message);
        }
    }
}
void validate_variable(validation_t& validation, const ast::var_name_t& var_name) {
    if(!validation.variable_lookup.contains_in_accessible_scopes(var_name) && !utils::contains(validation.global_variable_declarations, var_name) && !utils::contains(validation.global_variable_definitions, var_name)) {
        throw std::runtime_error("Variable [" + var_name + "] is not declared in currently accessible scopes.");
    }
    // TODO: type check the variables once we add more types
}
void validate_variable_declaration(validation_t& validation, ast::declaration_t& declaration) {
    if(validation.variable_lookup.contains_in_lowest_scope(declaration.var_name)) {
        throw std::runtime_error("Variable " + declaration.var_name + " already declared in current scope.");
    }

    validation.variable_lookup.add_new_variable_in_current_scope(declaration.var_name, declaration.type_name);

    if(declaration.value.has_value()) {
        validate_expression(validation, declaration.value.value());
    }
}
void validate_function_declaration(validation_t& validation, const ast::function_declaration_t& function_declaration) {
    if(utils::contains(validation.global_variable_declarations, function_declaration.function_name) || utils::contains(validation.global_variable_definitions, function_declaration.function_name)) {
        throw "Function [" + function_declaration.function_name + "] is already declared as a global variable.";
    }

    if(utils::contains(validation.function_declarations_lookup, function_declaration.function_name)) {
        const auto existing_function_declaration = validation.function_declarations_lookup.at(function_declaration.function_name);
        validate_type_name(function_declaration.return_type, existing_function_declaration.return_type, "Function [" + function_declaration.function_name + "] return type mismatch.");
        if(function_declaration.params.size() != existing_function_declaration.params.size()) {
            throw std::runtime_error("Function [" + function_declaration.function_name + "] param count mismatch.");
        }
        for(std::uint32_t i = 0u; i < function_declaration.params.size(); ++i) {
            validate_type_name(function_declaration.params[i], existing_function_declaration.params[i], "Function [" + function_declaration.function_name + "] param type mismatch.");
        }
    } else if(utils::contains(validation.function_definitions_lookup, function_declaration.function_name)) {
        const auto existing_function_definition = validation.function_definitions_lookup.at(function_declaration.function_name);
        validate_type_name(function_declaration.return_type, existing_function_definition.return_type, "Function [" + function_declaration.function_name + "] return type mismatch.");
        if(function_declaration.params.size() != existing_function_definition.params.size()) {
            throw std::runtime_error("Function [" + function_declaration.function_name + "] param count mismatch.");
        }
        for(std::uint32_t i = 0u; i < function_declaration.params.size(); ++i) {
            validate_type_name(function_declaration.params[i], existing_function_definition.params[i].first, "Function [" + function_declaration.function_name + "] param type mismatch.");
        }
    } else {
        validation.function_declarations_lookup.insert({function_declaration.function_name, function_declaration});
    }
}
void validate_expression(validation_t& validation, ast::expression_t& expression) {
    std::visit(overloaded{
        [&validation, &expression](const std::shared_ptr<ast::grouping_t>& grouping_exp) {
            validate_expression(validation, grouping_exp->expr);
        },
        [&validation, &expression](const std::shared_ptr<ast::unary_expression_t>& unary_exp) {
            validate_expression(validation, unary_exp->exp);
        },
        [&validation, &expression](const std::shared_ptr<ast::binary_expression_t>& bin_exp) {
            validate_expression(validation, bin_exp->left);
            validate_expression(validation, bin_exp->right);
        },
        [&validation, &expression](const std::shared_ptr<ast::ternary_expression_t>& ternary) {
            validate_expression(validation, ternary->condition);
            validate_expression(validation, ternary->if_true);
            validate_expression(validation, ternary->if_false);
        },
        [&validation, &expression](const std::shared_ptr<ast::function_call_t>& func_call) {
            validate_function_call(validation, *func_call);
            add_type_to_function_call(validation, *func_call, expression.type); // will set `expression.type` to the return type of the accompanying function
        },
        [&validation](const ast::constant_t& expression) {
            // TODO: consider adding types to constant expressions here instead of earlier in the initial parsing pass
        },
        [&validation, &expression](const ast::var_name_t& var_name) {
            validate_variable(validation, var_name);
            add_type_to_variable(validation, var_name, expression.type); // will set `expression.type` to the type of the variable from its declaration
        },
        [](const std::shared_ptr<ast::convert_t>& convert_exp) {
            throw std::runtime_error("User casts not yet supported.");
        }
    }, expression.expr);
}
void validate_statement(validation_t& validation, ast::statement_t& statement) {
    std::visit(overloaded{
        [&validation](ast::return_statement_t& statement) {
            validate_expression(validation, statement.expr);
        },
        [&validation](ast::expression_statement_t& statement) {
            if(statement.expr.has_value()) {
                validate_expression(validation, statement.expr.value());
            }
        },
        [&validation](const std::shared_ptr<ast::if_statement_t>& statement) {
            validate_expression(validation, statement->if_exp);
            validate_statement(validation, statement->if_body);
            if(statement->else_body.has_value()) {
                validate_statement(validation, statement->else_body.value());
            }
        },
        [&validation](const std::shared_ptr<ast::compound_statement_t>& statement) {
            validate_compound_statement(validation, *statement);
        }
    }, statement);
}
void validate_compound_statement(validation_t& validation, ast::compound_statement_t& compound_statement, bool is_function_block) {
    if(!is_function_block) {
        validation.variable_lookup.create_new_scope();
    }
    for(auto& e : compound_statement.stmts) {
        std::visit(overloaded{
            [&validation](ast::statement_t& statement) {
                validate_statement(validation, statement);
            },
            [&validation](ast::declaration_t& declaration) {
                validate_variable_declaration(validation, declaration);
            },
        }, e);
    }
    if(!is_function_block) {
        validation.variable_lookup.destroy_current_scope();
    }
}
void validate_function_definition(validation_t& validation, const ast::function_definition_t& function_definition) {
    if(utils::contains(validation.global_variable_declarations, function_definition.function_name) || utils::contains(validation.global_variable_definitions, function_definition.function_name)) {
        throw "Function [" + function_definition.function_name + "] is already declared as a global variable.";
    }

    if(utils::contains(validation.function_definitions_lookup, function_definition.function_name)) {
        throw std::runtime_error("Function [" + function_definition.function_name + "] already defined.");
    }
    if(utils::contains(validation.function_declarations_lookup, function_definition.function_name)) {
        const auto existing_function_declaration = validation.function_declarations_lookup.at(function_definition.function_name);
        validate_type_name(function_definition.return_type, existing_function_declaration.return_type, "Function [" + function_definition.function_name + "] return type mismatch.");
        if(function_definition.params.size() != existing_function_declaration.params.size()) {
            throw std::runtime_error("Function [" + function_definition.function_name + "] param count mismatch.");
        }
        for(std::uint32_t i = 0u; i < function_definition.params.size(); ++i) {
            validate_type_name(function_definition.params[i].first, existing_function_declaration.params[i], "Function [" + function_definition.function_name + "] param type mismatch.");
        }
    }
    validation.function_definitions_lookup.insert({function_definition.function_name, function_definition});
    validation.variable_lookup.create_new_scope();
    for(const auto param : function_definition.params) {
        if(param.second.has_value()) {
            validation.variable_lookup.add_new_variable_in_current_scope(param.second.value(), param.first);
        }
    }
    auto& validation_function_definition = validation.function_definitions_lookup.at(function_definition.function_name);
    validate_compound_statement(validation, validation_function_definition.statements, true);
    validation.variable_lookup.destroy_current_scope();
}
void validate_function_call(validation_t& validation, const ast::function_call_t& function_call) {
    if(utils::contains(validation.function_declarations_lookup, function_call.function_name)) {
        const auto declaration = validation.function_declarations_lookup.at(function_call.function_name);
        if(declaration.params.size() != function_call.params.size()) {
            throw std::runtime_error("Function [" + function_call.function_name + "] param count mismatch.");
        }
        // TODO: type check parameters to function call
    } else if(utils::contains(validation.function_definitions_lookup, function_call.function_name)) {
        const auto definition = validation.function_definitions_lookup.at(function_call.function_name);
        if(definition.params.size() != function_call.params.size()) {
            throw std::runtime_error("Function [" + function_call.function_name + "] param count mismatch.");
        }
        // TODO: type check parameters to function call
    } else {
        throw std::runtime_error("Function [" + function_call.function_name + "] not declared or defined.");
    }
}
void validate_compile_time_expression(validation_t& validation, const ast::expression_t& expression) {
    std::visit(overloaded{
        [&validation](const std::shared_ptr<ast::grouping_t>& expression) {
            validate_compile_time_expression(validation, expression->expr);
        },
        [&validation](const std::shared_ptr<ast::unary_expression_t>& expression) {
            if(expression->op == ast::unary_operator_token_t::PLUS_PLUS || expression->op == ast::unary_operator_token_t::MINUS_MINUS) {
                throw std::runtime_error("`++` and `--` not supported in compile time expressions.");
            }
            validate_compile_time_expression(validation, expression->exp);
        },
        [&validation](const std::shared_ptr<ast::binary_expression_t>& expression) {
            if(expression->op == ast::binary_operator_token_t::ASSIGNMENT) {
                throw std::runtime_error("Assignment not supported in compile time expressions.");
            }
            validate_compile_time_expression(validation, expression->left);
            validate_compile_time_expression(validation, expression->right);
        },
        [&validation](const std::shared_ptr<ast::ternary_expression_t>& expression) {
            validate_compile_time_expression(validation, expression->condition);
            validate_compile_time_expression(validation, expression->if_true);
            validate_compile_time_expression(validation, expression->if_false);
        },
        [](const std::shared_ptr<ast::function_call_t>& expression) {
            throw std::runtime_error("Function calls not supported in compile time expressions.");
        },
        [](const ast::constant_t& expression) {
            // totally fine, no need to go further as this is a terminal node of the AST
        },
        [](const ast::var_name_t& expression) {
            throw std::runtime_error("Variables not supported in compile time expressions.");
            // TODO: Maybe support referencing other global variables???? Check the C standard to see what is considered valid.
        },
        [&validation](const std::shared_ptr<ast::convert_t>& expression) {
            validate_compile_time_expression(validation, expression->expr);
        }
    }, expression.expr);
}
void validate_global_variable_declaration(validation_t& validation, const ast::global_variable_declaration_t& declaration) {
    if(utils::contains(validation.function_declarations_lookup, declaration.var_name) || utils::contains(validation.function_definitions_lookup, declaration.var_name)) {
        throw std::runtime_error("Global variable [" + declaration.var_name + "] already declared as a function.");
    }

    if(declaration.value.has_value()) {
        if(utils::contains(validation.global_variable_definitions, declaration.var_name)) {
            throw std::runtime_error("Global variable [" + declaration.var_name + "] already defined.");
        }

        if(utils::contains(validation.global_variable_declarations, declaration.var_name)) {
            auto existing_declaration = validation.global_variable_declarations.at(declaration.var_name);

            validate_type_name(existing_declaration.type_name, declaration.type_name, "Mismatched global variable type.");
        }

        validate_compile_time_expression(validation, declaration.value.value());

        validation.global_variable_definitions.insert({declaration.var_name, declaration}); // idempotent operation
    } else { // this part of the C standard diverges from C++ since you are not required to preface declarations with `extern` in C, but you are in C++.
        if(utils::contains(validation.global_variable_declarations, declaration.var_name)) {
            auto existing_declaration = validation.global_variable_declarations.at(declaration.var_name);

            validate_type_name(existing_declaration.type_name, declaration.type_name, "Mismatched global variable type.");
        }
        if(utils::contains(validation.global_variable_definitions, declaration.var_name)) {
            auto existing_definition = validation.global_variable_definitions.at(declaration.var_name);

            validate_type_name(existing_definition.type_name, declaration.type_name, "Mismatched global variable type.");
        }

        validation.global_variable_declarations.insert({declaration.var_name, declaration}); // idempotent operation
    }
}
ast::validated_program_t validate_ast(const ast::program_t& program) {
    validation_t validation;
    for(const auto& top_level_decl : program.top_level_declarations) {
        std::visit(overloaded{
            [&validation](const ast::function_declaration_t& function_decl) {
                validate_function_declaration(validation, function_decl);
            },
            [&validation](const ast::function_definition_t& function_def) {
                validate_function_definition(validation, function_def);
            },
            [&validation](const ast::global_variable_declaration_t& declaration) {
                validate_global_variable_declaration(validation, declaration);
            }
        }, top_level_decl);
    }

    ast::validated_program_t validated_program{};
    for(const auto& it : validation.function_definitions_lookup) {
        validated_program.top_level_declarations.push_back(it.second);
    }
    for(const auto& it : validation.global_variable_definitions) {
        ast::validated_global_variable_definition_t validated_definition{it.second.type_name, it.first, evaluate_expression(it.second.value.value())};

        validated_program.top_level_declarations.push_back(validated_definition);
    }
    for(const auto& it : validation.global_variable_declarations) {
        if(!utils::contains(validation.global_variable_definitions, it.first)) {
            ast::validated_global_variable_definition_t validated_definition{it.second.type_name, it.first, ast::constant_t{0}};

            validated_program.top_level_declarations.push_back(validated_definition);
        }
    }
    return validated_program;
}
