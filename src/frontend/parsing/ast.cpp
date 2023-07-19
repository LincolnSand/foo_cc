#include "ast.hpp"


void print_constant(const ast::constant_t& constant) {
    std::cout << "(constant ";
    std::cout << constant.value;
    std::cout << ')';
}

void print_unary_op(const ast::unop_t& op) {
    std::cout << "(unary_op ";
    std::cout << static_cast<std::uint16_t>(op.op);
    print_expr(op.expr);
    std::cout << ')';
}

void print_grouping(const ast::grouping_t& grouping) {
    std::cout << "(grouping ";
    print_expr(grouping.expr);
    std::cout << ')';
}

void print_factor(const ast::factor_t& factor) {
    std::cout << "(factor ";
    std::visit(overloaded{
        [](const std::shared_ptr<ast::grouping_t>& factor) {
            print_grouping(*factor);
        },
        [](const std::shared_ptr<ast::unop_t>& factor) {
            print_unary_op(*factor);
        },
        [](const ast::constant_t& factor) {
            print_constant(factor);
        }
    }, factor);
    std::cout << ')';
}

void print_times_divide_expr(const ast::times_divide_expression_t& expr) {
    std::cout << "(times_divide_expr ";
    std::cout << '[' << static_cast<std::uint16_t>(expr.op) << ']';
    print_term(expr.lhs);
    print_term(expr.rhs);
    std::cout << ')';
}

void print_term(const ast::term_t& term) {
    std::cout << "(term ";
    std::visit(overloaded{
        [](const std::shared_ptr<ast::times_divide_expression_t>& term) {
            print_times_divide_expr(*term);
        },
        [](const ast::factor_t& factor) {
            print_factor(factor);
        }
    }, term);
    std::cout << ')';
}

void print_plus_minus_expr(const ast::plus_minus_expression_t& expr) {
    std::cout << "(plus_minus_expr ";
    std::cout << '[' << static_cast<std::uint16_t>(expr.op) << ']';
    print_expr(expr.lhs);
    print_expr(expr.rhs);
    std::cout << ')';
}

void print_expr(const ast::expression_t& expr) {
    std::cout << "(expr ";
    std::visit(overloaded{
        [](const std::shared_ptr<ast::plus_minus_expression_t>& expr) {
            print_plus_minus_expr(*expr);
        },
        [](const ast::term_t& term) {
            print_term(term);
        }
    }, expr);
    std::cout << ')';
}

void print_return_stmt(const ast::return_statement_t& return_stmt) {
    //std::cout << "(return ";
    print_expr(return_stmt.return_stmt);
    //std::cout << ')';
}

void print_func_decl(const ast::function_declaration_t& func_decl) {
    //std::cout << "(func_decl ";
    //std::cout << func_decl.name;
    print_return_stmt(func_decl.statement);
    //std::cout << ')';
}

void print_ast(const ast::program_t& program) {
    //std::cout << "(program ";
    print_func_decl(program.function_declaration);
    std::cout << ")\n";
}
