#include "validate_ast.hpp"


void validate_type(const ast::type_name_t& expected, const ast::type_name_t& actual, const std::string& error_message) {
    if(expected.token_type != actual.token_type) { // optimization to avoid having to do string comparisons for built-in types
        if(expected.token_text != actual.token_text) {
            throw std::runtime_error(error_message);
        }
    }
}
void validate_variable(validation_t& validation, const ast::var_name_t& var_name) {
    if(!validation.variable_lookup.contains_in_accessible_scopes(var_name)) {
        throw std::runtime_error("Variable [" + var_name + "] is not declared in currently accessible scopes.");
    }
    // TODO: type check the variables once we add more types
}
void validate_variable_declaration(validation_t& validation, const ast::declaration_t& declaration) {
    if(validation.variable_lookup.contains_in_lowest_scope(declaration.var_name)) {
        throw std::runtime_error("Variable " + declaration.var_name + " already declared in current scope.");
    }

    validation.variable_lookup.add_new_variable_in_current_scope(declaration.var_name, declaration.type_name);
}
void validate_function_declaration(validation_t& validation, const ast::function_declaration_t& function_declaration) {
    if(utils::contains(validation.declarations_lookup, function_declaration.function_name)) {
        const auto existing_function_declaration = validation.declarations_lookup.at(function_declaration.function_name);
        validate_type(function_declaration.return_type, existing_function_declaration.return_type, "Function [" + function_declaration.function_name + "] return type mismatch.");
        if(function_declaration.params.size() != existing_function_declaration.params.size()) {
            throw std::runtime_error("Function [" + function_declaration.function_name + "] param count mismatch.");
        }
        for(std::uint32_t i = 0u; i < function_declaration.params.size(); ++i) {
            validate_type(function_declaration.params[i], existing_function_declaration.params[i], "Function [" + function_declaration.function_name + "] param type mismatch.");
        }
    } else if(utils::contains(validation.definitions_lookup, function_declaration.function_name)) {
        const auto existing_function_definition = validation.definitions_lookup.at(function_declaration.function_name);
        validate_type(function_declaration.return_type, existing_function_definition.return_type, "Function [" + function_declaration.function_name + "] return type mismatch.");
        if(function_declaration.params.size() != existing_function_definition.params.size()) {
            throw std::runtime_error("Function [" + function_declaration.function_name + "] param count mismatch.");
        }
        for(std::uint32_t i = 0u; i < function_declaration.params.size(); ++i) {
            validate_type(function_declaration.params[i], existing_function_definition.params[i].first, "Function [" + function_declaration.function_name + "] param type mismatch.");
        }
    } else {
        validation.declarations_lookup.insert({function_declaration.function_name, function_declaration});
    }
}
void validate_expression(validation_t& validation, const ast::expression_t& expression) {
    std::visit(overloaded{
        [&validation](const std::shared_ptr<ast::grouping_t>& expression) {
            validate_expression(validation, expression->expr);
        },
        [&validation](const std::shared_ptr<ast::unary_expression_t>& expression) {
            validate_expression(validation, expression->exp);
        },
        [&validation](const std::shared_ptr<ast::binary_expression_t>& expression) {
            validate_expression(validation, expression->left);
            validate_expression(validation, expression->right);
        },
        [&validation](const std::shared_ptr<ast::ternary_expression_t>& expression) {
            validate_expression(validation, expression->condition);
            validate_expression(validation, expression->if_true);
            validate_expression(validation, expression->if_false);
        },
        [&validation](const std::shared_ptr<ast::function_call_t>& expression) {
            validate_function_call(validation, *expression);
        },
        [&validation](const ast::constant_t& expression) {},
        [&validation](const ast::var_name_t& expression) {
            validate_variable(validation, expression);
        }
    }, expression);
}
void validate_statement(validation_t& validation, const ast::statement_t& statement) {
    std::visit(overloaded{
        [&validation](const ast::return_statement_t& statement) {
            validate_expression(validation, statement.expr);
        },
        [&validation](const ast::expression_statement_t& statement) {
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
void validate_compound_statement(validation_t& validation, const ast::compound_statement_t& compound_statement, bool is_function_block) {
    if(!is_function_block) {
        validation.variable_lookup.create_new_scope();
    }
    for(const auto e : compound_statement.stmts) {
        std::visit(overloaded{
            [&validation](const ast::statement_t& statement) {
                validate_statement(validation, statement);
            },
            [&validation](const ast::declaration_t& declaration) {
                validate_variable_declaration(validation, declaration);
            },
        }, e);
    }
    if(!is_function_block) {
        validation.variable_lookup.destroy_current_scope();
    }
}
void validate_function_definition(validation_t& validation, const ast::function_definition_t& function_definition) {
    if(utils::contains(validation.definitions_lookup, function_definition.function_name)) {
        throw std::runtime_error("Function [" + function_definition.function_name + "] already defined.");
    }
    if(utils::contains(validation.declarations_lookup, function_definition.function_name)) {
        const auto existing_function_declaration = validation.declarations_lookup.at(function_definition.function_name);
        validate_type(function_definition.return_type, existing_function_declaration.return_type, "Function [" + function_definition.function_name + "] return type mismatch.");
        if(function_definition.params.size() != existing_function_declaration.params.size()) {
            throw std::runtime_error("Function [" + function_definition.function_name + "] param count mismatch.");
        }
        for(std::uint32_t i = 0u; i < function_definition.params.size(); ++i) {
            validate_type(function_definition.params[i].first, existing_function_declaration.params[i], "Function [" + function_definition.function_name + "] param type mismatch.");
        }
    }
    validation.definitions_lookup.insert({function_definition.function_name, function_definition});
    validation.variable_lookup.create_new_scope();
    for(const auto param : function_definition.params) {
        if(param.second.has_value()) {
            validation.variable_lookup.add_new_variable_in_current_scope(param.second.value(), param.first);
        }
    }
    validate_compound_statement(validation, function_definition.statements, true);
    validation.variable_lookup.destroy_current_scope();
}
void validate_function_call(validation_t& validation, const ast::function_call_t& function_call) {
    if(utils::contains(validation.declarations_lookup, function_call.function_name)) {
        const auto declaration = validation.declarations_lookup.at(function_call.function_name);
        if(declaration.params.size() != function_call.params.size()) {
            throw std::runtime_error("Function [" + function_call.function_name + "] param count mismatch.");
        }
        // TODO: type check parameters to function call
    } else if(utils::contains(validation.definitions_lookup, function_call.function_name)) {
        const auto definition = validation.definitions_lookup.at(function_call.function_name);
        if(definition.params.size() != function_call.params.size()) {
            throw std::runtime_error("Function [" + function_call.function_name + "] param count mismatch.");
        }
        // TODO: type check parameters to function call
    } else {
        throw std::runtime_error("Function [" + function_call.function_name + "] not declared or defined.");
    }
}
void validate_ast(const ast::program_t& program) {
    validation_t validation;
    for(const auto& function_decl : program.function_declarations) {
        validate_function_declaration(validation, function_decl);
    }
    for(const auto& function_definition : program.function_definitions) {
        validate_function_definition(validation, function_definition);
    }
    for(const auto& declaration : program.declarations) {
        validate_variable_declaration(validation, declaration);
    }
}