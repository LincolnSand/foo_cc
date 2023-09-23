#include "ast_printer.hpp"


const char* get_unary_op_name(const ast::unary_operator_token_t op) {
    switch(op) {
        case ast::unary_operator_token_t::PLUS_PLUS:
            return "++";
        case ast::unary_operator_token_t::MINUS_MINUS:
            return "--";
        case ast::unary_operator_token_t::PLUS:
            return "+";
        case ast::unary_operator_token_t::MINUS:
            return "-";
        case ast::unary_operator_token_t::LOGICAL_NOT:
            return "!";
        case ast::unary_operator_token_t::BITWISE_NOT:
            return "~";
    }
    throw std::runtime_error("Invalid unary operator.");
}
const char* get_binary_op_name(const ast::binary_operator_token_t op) {
    switch(op) {
        case ast::binary_operator_token_t::MULTIPLY:
            return "*";
        case ast::binary_operator_token_t::DIVIDE:
            return "/";
        case ast::binary_operator_token_t::MODULO:
            return "%";
        case ast::binary_operator_token_t::PLUS:
            return "+";
        case ast::binary_operator_token_t::MINUS:
            return "-";
        case ast::binary_operator_token_t::LEFT_BITSHIFT:
            return "<<";
        case ast::binary_operator_token_t::RIGHT_BITSHIFT:
            return ">>";
        case ast::binary_operator_token_t::LESS_THAN:
            return "<";
        case ast::binary_operator_token_t::LESS_THAN_EQUAL:
            return "<=";
        case ast::binary_operator_token_t::GREATER_THAN:
            return ">";
        case ast::binary_operator_token_t::GREATER_THAN_EQUAL:
            return ">=";
        case ast::binary_operator_token_t::EQUAL:
            return "==";
        case ast::binary_operator_token_t::NOT_EQUAL:
            return "!=";
        case ast::binary_operator_token_t::BITWISE_AND:
            return "&";
        case ast::binary_operator_token_t::BITWISE_XOR:
            return "^";
        case ast::binary_operator_token_t::BITWISE_OR:
            return "|";
        case ast::binary_operator_token_t::LOGICAL_AND:
            return "&&";
        case ast::binary_operator_token_t::LOGICAL_OR:
            return "||";
        case ast::binary_operator_token_t::ASSIGNMENT:
            return "=";
        case ast::binary_operator_token_t::COMMA:
            return ",";
    }
    throw std::runtime_error("Invalid binary operator.");
}

void print_expression(const bool has_types, const ast::expression_t& expr) {
    std::visit(overloaded{
        [has_types](const std::shared_ptr<ast::grouping_t>& grouping) -> void {
            std::cout << "(grouping: ";
            print_expression(has_types, grouping->expr);
        },
        [has_types](const std::shared_ptr<ast::binary_expression_t>& binary_exp) -> void {
            std::cout << "(binary_exp: ";
            std::cout << "[" << get_binary_op_name(binary_exp->op) << ']';
            print_expression(has_types, binary_exp->left);
            print_expression(has_types, binary_exp->right);
        },
        [has_types](const std::shared_ptr<ast::unary_expression_t>& unary_exp) -> void {
            std::cout << "(unary_exp: ";
            std::cout << "[" << get_unary_op_name(unary_exp->op) << ']';
            print_expression(has_types, unary_exp->exp);
        },
        [has_types](const std::shared_ptr<ast::ternary_expression_t>& ternary_exp) -> void {
            std::cout << "(ternary_exp: ";
            print_expression(has_types, ternary_exp->condition);
            print_expression(has_types, ternary_exp->if_true);
            print_expression(has_types, ternary_exp->if_false);
        },
        [has_types](const std::shared_ptr<ast::function_call_t>& function_call) -> void {
            std::cout << "(function call: ";
            std::cout << function_call->function_name;
            std::cout << '(';
            for(auto i = 0; i < function_call->params.size(); ++i) {
                print_expression(has_types, function_call->params[i]);
                if(i != function_call->params.size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << ')';
        },
        [](const ast::constant_t& constant) -> void {
            std::cout << "(constant: ";
            std::visit(overloaded{
                [](const auto& value) {
                    std::cout << value;
                }
            }, constant.value);
        },
        [](const ast::var_name_t& var_name) -> void {
            std::cout << "(identifier: ";
            std::cout << var_name;
        },
        [has_types](const std::shared_ptr<ast::convert_t>& convert) -> void {
            std::cout << "(convert: ";
            print_expression(has_types, convert->expr);
        }
    }, expr.expr);
    if(has_types) {
        if(expr.type.has_value()) {
            std::cout << " :: " << expr.type.value().type_name;
        } else {
            std::cout << " :: " << "UNDEFINED";
        }
    }
    std::cout << ')';
}

void print_return_statement(const bool has_types, const ast::return_statement_t& return_stmt) {
    std::cout << "(return: ";
    print_expression(has_types, return_stmt.expr);
    std::cout << ')';
}
void print_if_statement(const bool has_types, const ast::if_statement_t& if_statement) {
    std::cout << "(if: ";
    std::cout << "(condition: ";
    print_expression(has_types, if_statement.if_exp);
    std::cout << ")";
    print_statement(has_types, if_statement.if_body, true);
    if(if_statement.else_body.has_value()) {
        std::cout << " else ";
        print_statement(has_types, if_statement.else_body.value(), true);
    }
    std::cout << ')';
}
void print_statement(const bool has_types, const ast::statement_t& stmt, const bool is_nested, const bool is_last_statement) {
    std::visit(overloaded{
        [has_types, is_last_statement](const ast::return_statement_t& stmt) {
            print_return_statement(has_types, stmt);
            if(!is_last_statement) {
                std::cout << '\n';
            }
        },
        [has_types, is_last_statement](const ast::expression_statement_t& stmt) {
            if(stmt.expr.has_value()) {
                print_expression(has_types, stmt.expr.value());
                if(!is_last_statement) {
                    std::cout << '\n';
                }
            }
        },
        [has_types, is_last_statement, is_nested](const std::shared_ptr<ast::if_statement_t>& stmt) {
            print_if_statement(has_types, *stmt);
            if(!is_last_statement || is_nested) {
                std::cout << '\n';
            }
        },
        [has_types](const std::shared_ptr<ast::compound_statement_t>& stmt) {
            print_compound_statement(has_types, *stmt, true);
        }
    }, stmt);
}

void print_declaration(const bool has_types, const ast::declaration_t& declaration, const bool is_last_statement) {
    std::cout << "(declaration: ";
    std::cout << declaration.type_name.type_name;
    std::cout << ' ';
    std::cout << declaration.var_name;
    if(declaration.value.has_value()) {
        std::cout << " = ";
        print_expression(has_types, declaration.value.value());
    }
    std::cout << ')';
    if(!is_last_statement) {
        std::cout << '\n';
    }
}
void print_compound_statement(const bool has_types, const ast::compound_statement_t& declaration_statement, const bool is_nested) {
    for(auto i = 0; i < declaration_statement.stmts.size(); ++i) {
        const bool is_last_stmt = (i == (declaration_statement.stmts.size() - 1));
        std::visit(overloaded{
            [has_types, is_last_stmt, is_nested](const ast::statement_t& stmt) {
                print_statement(has_types, stmt, is_nested, is_last_stmt);
            },
            [has_types, is_last_stmt](const ast::declaration_t& declaration) {
                print_declaration(has_types, declaration, is_last_stmt);
            }
        }, declaration_statement.stmts[i]);
    }
}
void print_function_decl(const ast::function_declaration_t& function_declaration) {
    std::cout << "(" << function_declaration.return_type.type_name << ' ' << function_declaration.function_name << '(';
    for(auto i = 0; i < function_declaration.params.size(); ++i) {
        std::cout << function_declaration.params[i].type_name;
        if(i != function_declaration.params.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "))\n";
}
void print_function_definition(const bool has_types, const ast::function_definition_t& function_definition) {
    std::cout << "(" << function_definition.return_type.type_name << ' ' << function_definition.function_name << '(';
    for(auto i = 0; i < function_definition.params.size(); ++i) {
        std::cout << function_definition.params[i].first.type_name;
        if(function_definition.params[i].second.has_value()) {
            std::cout << ' ' << function_definition.params[i].second.value();
        }
        if(i != function_definition.params.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "): ";
    if(function_definition.statements.stmts.size() > 0) {
        std::cout << '\n';
    }
    print_compound_statement(has_types, function_definition.statements, false);
    std::cout << ")\n";
}

void print_global_variable_definition(const ast::global_variable_declaration_t& global_var_def) {
    std::cout << "(global_var: ";
    std::cout << global_var_def.type_name.type_name;
    std::cout << ' ';
    std::cout << global_var_def.var_name;
    if(global_var_def.value.has_value()) {
        std::cout << " = ";
        print_expression(true, ast::expression_t{global_var_def.value.value()});
    }
    std::cout << ')';
    std::cout << '\n';
}
void print_validated_ast(const ast::validated_program_t& validated_program) {
    for(const auto& e : validated_program.top_level_declarations) {
        std::visit(overloaded{
            [](const ast::function_definition_t& function_def) {
                print_function_definition(true, function_def);
            },
            [](const ast::global_variable_declaration_t& global_var_def) {
                print_global_variable_definition(global_var_def);
            }
        }, e);
    }
    std::cout << '\n';
}

