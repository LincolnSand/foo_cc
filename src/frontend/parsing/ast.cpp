#include "ast.hpp"


void print_variable_name(const ast::var_name_t& var_name) {
    std::cout << "(variable_name: ";
    std::cout << var_name;
    std::cout << ')';
}
void print_constant(const ast::constant_t& constant) {
    std::cout << "(constant: " << constant.value << ')';
}
void print_unary_op(const ast::unary_op_expression_t& op) {
    std::cout << "(unary_op: [" << static_cast<std::uint16_t>(op.op) << "]: ";
    print_factor(op.expr);
    std::cout << ')';
}
void print_grouping(const ast::grouping_t& grouping) {
    std::cout << "(grouping: ";
    print_expression(grouping.expr);
    std::cout << ')';
}
void print_factor(const ast::factor_t& factor) {
    std::visit(overloaded{
        [](const std::shared_ptr<ast::grouping_t>& grouping) {
            print_grouping(*grouping);
        },
        [](const std::shared_ptr<ast::unary_op_expression_t>& op) {
            print_unary_op(*op);
        },
        [](const ast::constant_t& constant) {
            print_constant(constant);
        },
        [](const ast::var_name_t& var_name) {
            print_variable_name(var_name);
        }
    }, factor);
}
void print_times_divide_binary_expression(const ast::times_divide_binary_expression_t& expr) {
    std::cout << "(times_divide [" << static_cast<std::uint16_t>(expr.op) << "]: ";
    print_times_divide_expression(expr.lhs);
    print_times_divide_expression(expr.rhs);
    std::cout << ')';
}
void print_times_divide_expression(const ast::times_divide_expression_t& expr) {
    std::visit(overloaded{
        [](const std::shared_ptr<ast::times_divide_binary_expression_t>& expr) {
            print_times_divide_binary_expression(*expr);
        },
        [](const ast::factor_t& factor) {
            print_factor(factor);
        }
    }, expr);
}
void print_plus_minus_binary_expression(const ast::plus_minus_binary_expression_t& expr) {
    std::cout << "(plus_minus [" << static_cast<std::uint16_t>(expr.op) << "]: ";
    print_plus_minus_expression(expr.lhs);
    print_plus_minus_expression(expr.rhs);
    std::cout << ')';
}
void print_plus_minus_expression(const ast::plus_minus_expression_t& expr) {
    std::visit(overloaded{
        [](const std::shared_ptr<ast::plus_minus_binary_expression_t>& expr) {
            print_plus_minus_binary_expression(*expr);
        },
        [](const ast::times_divide_expression_t& expr) {
            print_times_divide_expression(expr);
        }
    }, expr);
}
void print_relational_binary_expression(const ast::relational_binary_expression_t& expr) {
    std::cout << "(relational [" << static_cast<std::uint16_t>(expr.op) << "]: ";
    print_relational_expression(expr.lhs);
    print_relational_expression(expr.rhs);
    std::cout << ')';
}
void print_relational_expression(const ast::relational_expression_t& expr) {
    std::visit(overloaded{
        [](const std::shared_ptr<ast::relational_binary_expression_t>& expr) {
            print_relational_binary_expression(*expr);
        },
        [](const ast::plus_minus_expression_t& expr) {
            print_plus_minus_expression(expr);
        }
    }, expr);
}
void print_equality_binary_expression(const ast::equality_binary_expression_t& expr) {
    std::cout << "(equality [" << static_cast<std::uint16_t>(expr.op) << "]: ";
    print_equality_expression(expr.lhs);
    print_equality_expression(expr.rhs);
    std::cout << ')';
}
void print_equality_expression(const ast::equality_expression_t& expr) {
    std::visit(overloaded{
        [](const std::shared_ptr<ast::equality_binary_expression_t>& expr) {
            print_equality_binary_expression(*expr);
        },
        [](const ast::relational_expression_t& expr) {
            print_relational_expression(expr);
        }
    }, expr);
}
void print_logical_and_binary_expression(const ast::logical_and_binary_expression_t& expr) {
    std::cout << "(logical_and [" << static_cast<std::uint16_t>(expr.op) << "]: ";
    print_logical_and_expression(expr.lhs);
    print_logical_and_expression(expr.rhs);
    std::cout << ')';
}
void print_logical_and_expression(const ast::logical_and_expression_t& expr) {
    std::visit(overloaded{
        [](const std::shared_ptr<ast::logical_and_binary_expression_t>& expr) {
            print_logical_and_binary_expression(*expr);
        },
        [](const ast::equality_expression_t& expr) {
            print_equality_expression(expr);
        }
    }, expr);
}
void print_logical_or_binary_expression(const ast::logical_or_binary_expression_t& expr) {
    std::cout << "(logical_or [" << static_cast<std::uint16_t>(expr.op) << "]: ";
    print_logical_or_expression(expr.lhs);
    print_logical_or_expression(expr.rhs);
    std::cout << ')';
}
void print_logical_or_expression(const ast::logical_or_expression_t& expr) {
    std::visit(overloaded{
        [](const std::shared_ptr<ast::logical_or_binary_expression_t>& expr) {
            print_logical_or_binary_expression(*expr);
        },
        [](const ast::logical_and_expression_t& expr) {
            print_logical_and_expression(expr);
        }
    }, expr);
}
void print_assignment(const ast::assignment_t& assignment) {
    std::cout << "(assignment: ";
    std::cout << assignment.var_name;
    std::cout << " = ";
    print_expression(assignment.expr);
    std::cout << ')';
}
void print_expression(const ast::expression_t& expr) {
    std::visit(overloaded{
        [](const std::shared_ptr<ast::assignment_t>& expr) {
            print_assignment(*expr);
        },
        [](const ast::logical_or_expression_t& expr) {
            print_logical_or_expression(expr);
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
