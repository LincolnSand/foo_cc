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
        },
        [&assembly_output](const ast::var_name_t& var_name) {
            pop_variable(assembly_output, var_name, "rax");
            store_register(assembly_output, "rax");
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

    pop_constant(assembly_output, "rax");

    assembly_output.output += "cmpq $0, %rax\n";
    std::string clause_2_label_name = "_clause2_" + std::to_string(assembly_output.current_label_number++);
    std::string end_label_name = "_end_" + std::to_string(assembly_output.current_label_number++);
    assembly_output.output += "jne " + clause_2_label_name + "\n";

    // short circuit on false
    assembly_output.output += "movq $1, %rax\n";
    assembly_output.output += "jmp " + end_label_name + "\n";

    assembly_output.output += clause_2_label_name + ":\n";

    generate_logical_and_expression(assembly_output, expr.rhs);
    pop_constant(assembly_output, "rax");

    assembly_output.output += "cmpq $0, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setne %al\n";
    assembly_output.output += end_label_name + ":\n";

    store_register(assembly_output, "rax");
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

    pop_constant(assembly_output, "rax");

    assembly_output.output += "cmpq $0, %rax\n";
    std::string clause_2_label_name = "_clause2_" + std::to_string(assembly_output.current_label_number++);
    std::string end_label_name = "_end_" + std::to_string(assembly_output.current_label_number++);
    assembly_output.output += "je " + clause_2_label_name + "\n";

    // short circuit on true
    assembly_output.output += "movq $1, %rax\n";
    assembly_output.output += "jmp " + end_label_name + "\n";

    assembly_output.output += clause_2_label_name + ":\n";

    generate_logical_or_expression(assembly_output, expr.rhs);
    pop_constant(assembly_output, "rax");

    assembly_output.output += "cmpq $0, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setne %al\n";
    assembly_output.output += end_label_name + ":\n";

    store_register(assembly_output, "rax");
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
void generate_assignment(assembly_output_t& assembly_output, const ast::assignment_t& assignment) {
    if(!assembly_output.variable_lookup.contains_in_accessible_scopes(assignment.var_name)) {
        throw std::runtime_error("Variable " + assignment.var_name + " not declared in currently accessible scopes.");
    }

    generate_expression(assembly_output, assignment.expr);

    pop_constant(assembly_output, "rax");
    store_variable(assembly_output, assignment.var_name, "rax");
}
void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::assignment_t> expr) {
            generate_assignment(assembly_output, *expr);
        },
        [&assembly_output](const ast::logical_or_expression_t& expr) {
            generate_logical_or_expression(assembly_output, expr);
        }
    }, expression);
}
void generate_decl(assembly_output_t& assembly_output, const ast::declaration_t& decl) {
    if(assembly_output.variable_lookup.contains_in_lowest_scope(decl.var_name)) {
        throw std::runtime_error("Variable " + decl.var_name + " already declared in current scope.");
    }

    // start off at `8` instead of `0` since we read from low to high memory address and we negate the offset from ebp in the emited code, so the first byte is `-8` and the range is [-8, 0) instead of [0, -8).
    assembly_output.variable_lookup.add_new_variable_in_current_scope(decl.var_name, (assembly_output.current_ebp_offset += 8));

    allocate_stack_space_for_variable(assembly_output);

    if(decl.value.has_value()) {
        generate_expression(assembly_output, *decl.value);

        pop_constant(assembly_output, "rax");
        store_variable(assembly_output, decl.var_name, "rax");
    }
}
void generate_return_stmt(assembly_output_t& assembly_output, const ast::return_statement_t& return_stmt) {
    generate_expression(assembly_output, return_stmt.expr);

    pop_constant(assembly_output, "rax");

    generate_function_epilogue(assembly_output);
}
void generate_stmt(assembly_output_t& assembly_output, const ast::statement_t& stmt) {
    std::visit(overloaded{
        [&assembly_output](const ast::return_statement_t& stmt) {
            generate_return_stmt(assembly_output, stmt);
        },
        [&assembly_output](const ast::declaration_t& stmt) {
            generate_decl(assembly_output, stmt);
        },
        [&assembly_output](const ast::expression_t& stmt) {
            generate_expression(assembly_output, stmt);
        }
    }, stmt);
}
void generate_func_decl(assembly_output_t& assembly_output, const ast::function_declaration_t& function) {
    assembly_output.variable_lookup.create_new_scope();

    assembly_output.output += ".globl ";
    assembly_output.output += function.func_name;
    assembly_output.output += "\n";
    assembly_output.output += function.func_name;
    assembly_output.output += ":\n";

    generate_function_prologue(assembly_output);

    for(const auto& stmt : function.statements) {
        generate_stmt(assembly_output, stmt);
    }

    assembly_output.variable_lookup.destroy_current_scope();
}
void generate_program(assembly_output_t& assembly_output, const ast::program_t& program) {
    generate_func_decl(assembly_output, program.function_declaration);
}
std::string generate_asm(const ast::program_t& program) {
    assembly_output_t assembly_output;
    generate_program(assembly_output, program);
    return assembly_output.output;
}
