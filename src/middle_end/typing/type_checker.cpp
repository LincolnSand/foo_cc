#include "type_checker.hpp"


bool compare_type_names(const ast::type_name_t& lhs, const ast::type_name_t& rhs) {
    if(lhs.token_type == rhs.token_type) {
        if(lhs.type_name == rhs.type_name) {
            return true;
        }
    }
    return false;
}

void type_check_expression(ast::expression_t& expression) {
    
}
void type_check_statement(ast::statement_t& statement, const ast::type_name_t& function_return_type) {
    std::visit(overloaded{
        [&function_return_type](ast::return_statement_t& statement) {
            if(!compare_type_names(statement.expr.type.value(), function_return_type)) {
                statement.expr = ast::expression_t{ std::make_shared<ast::convert_t>(ast::convert_t{std::move(statement.expr)}), function_return_type };
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

        if(!compare_type_names(declaration.type_name, declaration.value.value().type.value())) {
            declaration.value = ast::expression_t{ std::make_shared<ast::convert_t>(ast::convert_t{std::move(declaration.value.value())}), declaration.type_name };
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
