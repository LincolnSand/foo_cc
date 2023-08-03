#include "ast.hpp"


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
    throw std::runtime_error("invalid unary operator.");
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
    throw std::runtime_error("invalid binary operator.");
}

void print_expression(const ast::expression_t& expr) {
    std::visit(overloaded{
        [](const std::shared_ptr<ast::grouping_t>& grouping) -> void {
            std::cout << "(grouping: ";
            print_expression(grouping->expr);
            std::cout << ')';
        },
        [](const std::shared_ptr<ast::binary_expression_t>& binary_exp) -> void {
            std::cout << "(binary_exp: ";
            std::cout << "[" << get_binary_op_name(binary_exp->op) << ']';
            print_expression(binary_exp->left);
            print_expression(binary_exp->right);
            std::cout << ')';
        },
        [](const std::shared_ptr<ast::unary_expression_t>& unary_exp) -> void {
            std::cout << "(unary_exp: ";
            std::cout << "[" << get_unary_op_name(unary_exp->op) << ']';
            print_expression(unary_exp->exp);
            std::cout << ')';
        },
        [](const ast::constant_t& constant) -> void {
            std::cout << "(constant: ";
            std::cout << constant.value;
            std::cout << ')';
        },
        [](const ast::var_name_t& var_name) -> void {
            std::cout << "(identifier: ";
            std::cout << var_name;
            std::cout << ')';
        }
    }, expr);
}

void print_declaration(const ast::declaration_t& declaration) {
    std::cout << "(declaration: ";
    std::cout << declaration.var_name;
    if(declaration.value.has_value()) {
        std::cout << " = ";
        print_expression(declaration.value.value());
    }
    std::cout << ')';
}
void print_return_stmt(const ast::return_statement_t& return_stmt) {
    std::cout << "(return: ";
    print_expression(return_stmt.expr);
    std::cout << ')';
}
void print_stmt(const ast::statement_t& stmt) {
    std::visit(overloaded{
        [](const ast::return_statement_t& stmt) {
            print_return_stmt(stmt);
        },
        [](const ast::declaration_t& stmt) {
            print_declaration(stmt);
        },
        [](const ast::expression_t& stmt) {
            print_expression(stmt);
        }
    }, stmt);
}
void print_func_decl(const ast::function_declaration_t& func_decl) {
    std::cout << "(" << func_decl.func_name << ": ";
    for(const auto& stmt : func_decl.statements) {
        print_stmt(stmt);
    }
    std::cout << ')';
}
void print_ast(const ast::program_t& program) {
    print_func_decl(program.function_declaration);
    std::cout << '\n';
}
