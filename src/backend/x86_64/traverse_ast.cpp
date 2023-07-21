#include "traverse_ast.hpp"


void generate_unary_op(assembly_output_t& assembly_output, const ast::unary_op_expression_t& op) {
    generate_factor(assembly_output, op.expr);
    switch(op.op) {
        case ast::unary_op_t::NEG:
            generate_negation(assembly_output);
            break;
        case ast::unary_op_t::BITWISE_NOT:
            generate_bitwise_not(assembly_output);
            break;
        case ast::unary_op_t::LOGIC_NOT:
            generate_logic_not(assembly_output);
            break;
    }
}
void generate_grouping(assembly_output_t& assembly_output, const ast::grouping_t& grouping) {
    generate_expression(assembly_output, grouping.expr);
}
void generate_factor(assembly_output_t& assembly_output, const ast::factor_t& factor) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::grouping_t>& factor) {
            generate_grouping(assembly_output, *factor);
        },
        [&assembly_output](const std::shared_ptr<ast::unary_op_expression_t>& unary_exp) {
            generate_unary_op(assembly_output, *unary_exp);
        },
        [&assembly_output](const ast::constant_t& constant) {
            store_constant(assembly_output, constant);
        }
    }, factor);
}
void generate_times_divide_binary_expression(assembly_output_t& assembly_output, const ast::times_divide_binary_expression_t& expr) {
    generate_times_divide_expression(assembly_output, expr.lhs);
    generate_times_divide_expression(assembly_output, expr.rhs);
    switch(expr.op) {
        case ast::times_divide_t::TIMES:
            generate_multiplication(assembly_output);
            break;
        case ast::times_divide_t::DIVIDE:
            generate_division(assembly_output);
            break;
    }
}
void generate_times_divide_expression(assembly_output_t& assembly_output, const ast::times_divide_expression_t& expr) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::times_divide_binary_expression_t>& expr) {
            generate_times_divide_binary_expression(assembly_output, *expr);
        },
        [&assembly_output](const ast::factor_t& expr) {
            generate_factor(assembly_output, expr);
        }
    }, expr);
}
void generate_plus_minus_binary_expression(assembly_output_t& assembly_output, const ast::plus_minus_binary_expression_t& expr) {
    generate_plus_minus_expression(assembly_output, expr.lhs);
    generate_plus_minus_expression(assembly_output, expr.rhs);
    switch(expr.op) {
        case ast::plus_minus_t::PLUS:
            generate_addition(assembly_output);
            break;
        case ast::plus_minus_t::MINUS:
            generate_subtraction(assembly_output);
            break;
    }
}
void generate_plus_minus_expression(assembly_output_t& assembly_output, const ast::plus_minus_expression_t& expr) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::plus_minus_binary_expression_t>& expr) {
            generate_plus_minus_binary_expression(assembly_output, *expr);
        },
        [&assembly_output](const ast::times_divide_expression_t& expr) {
            generate_times_divide_expression(assembly_output, expr);
        }
    }, expr);
}
void generate_relational_binary_expression(assembly_output_t& assembly_output, const ast::relational_binary_expression_t& expr) {
    generate_relational_expression(assembly_output, expr.lhs);
    generate_relational_expression(assembly_output, expr.rhs);
    switch(expr.op) {
        case ast::relational_t::LESS_THAN:
            generate_less_than(assembly_output);
            break;
        case ast::relational_t::GREATER_THAN:
            generate_greater_than(assembly_output);
            break;
        case ast::relational_t::LESS_THAN_EQUAL:
            generate_less_than_equal(assembly_output);
            break;
        case ast::relational_t::GREATER_THAN_EQUAL:
            generate_greater_than_equal(assembly_output);
            break;
    }
}
void generate_relational_expression(assembly_output_t& assembly_output, const ast::relational_expression_t& expr) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::relational_binary_expression_t>& expr) {
            generate_relational_binary_expression(assembly_output, *expr);
        },
        [&assembly_output](const ast::plus_minus_expression_t& expr) {
            generate_plus_minus_expression(assembly_output, expr);
        }
    }, expr);
}
void generate_equality_binary_expression(assembly_output_t& assembly_output, const ast::equality_binary_expression_t& expr) {
    generate_equality_expression(assembly_output, expr.lhs);
    generate_equality_expression(assembly_output, expr.rhs);
    switch(expr.op) {
        case ast::equality_t::EQUAL_EQUAL:
            generate_equality(assembly_output);
            break;
        case ast::equality_t::NOT_EQUAL:
            generate_not_equals(assembly_output);
            break;
    }
}
void generate_equality_expression(assembly_output_t& assembly_output, const ast::equality_expression_t& expr) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::equality_binary_expression_t>& expr) {
            generate_equality_binary_expression(assembly_output, *expr);
        },
        [&assembly_output](const ast::relational_expression_t& expr) {
            generate_relational_expression(assembly_output, expr);
        }
    }, expr);
}
void generate_logical_and_binary_expression(assembly_output_t& assembly_output, const ast::logical_and_binary_expression_t& expr) {
    generate_logical_and_expression(assembly_output, expr.lhs);
    generate_logical_and_expression(assembly_output, expr.rhs);
    generate_logical_and(assembly_output);
}
void generate_logical_and_expression(assembly_output_t& assembly_output, const ast::logical_and_expression_t& expr) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::logical_and_binary_expression_t>& expr) {
            generate_logical_and_binary_expression(assembly_output, *expr);
        },
        [&assembly_output](const ast::equality_expression_t& expr) {
            generate_equality_expression(assembly_output, expr);
        }
    }, expr);
}
void generate_logical_or_binary_expression(assembly_output_t& assembly_output, const ast::logical_or_binary_expression_t& expr) {
    generate_logical_or_expression(assembly_output, expr.lhs);
    generate_logical_or_expression(assembly_output, expr.rhs);
    generate_logical_or(assembly_output);
}
void generate_logical_or_expression(assembly_output_t& assembly_output, const ast::logical_or_expression_t& expr) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::logical_or_binary_expression_t>& expr) {
            generate_logical_or_binary_expression(assembly_output, *expr);
        },
        [&assembly_output](const ast::logical_and_expression_t& expr) {
            generate_logical_and_expression(assembly_output, expr);
        }
    }, expr);
}
void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression) {
    generate_logical_or_expression(assembly_output, expression);
}
void generate_return_stmt(assembly_output_t& assembly_output, const ast::return_statement_t& return_stmt) {
    generate_expression(assembly_output, return_stmt.return_stmt);

    pop_constant(assembly_output, "rax");

    assembly_output.output += "ret\n";
}
void generate_func_decl(assembly_output_t& assembly_output, const ast::function_declaration_t& function) {
    assembly_output.output += ".globl ";
    assembly_output.output += function.name;
    assembly_output.output += "\n";
    assembly_output.output += function.name;
    assembly_output.output += ":\n";

    generate_return_stmt(assembly_output, function.statement);
}
void generate_program(assembly_output_t& assembly_output, const ast::program_t& program) {
    generate_func_decl(assembly_output, program.function_declaration);
}
std::string generate_asm(const ast::program_t& program) {
    assembly_output_t assembly_output;
    generate_program(assembly_output, program);
    return assembly_output.output;
}
