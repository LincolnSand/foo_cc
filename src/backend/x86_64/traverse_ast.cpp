#include "traverse_ast.hpp"


void generate_grouping(assembly_output_t& assembly_output, const ast::grouping_t& grouping) {
    generate_expression(assembly_output, grouping.expr);
}
void generate_binary_expression(assembly_output_t& assembly_output, const ast::binary_expression_t& binary_exp) {
    switch(binary_exp.op) {
        case ast::binary_operator_token_t::MULTIPLY:
            generate_binary_operation(assembly_output, binary_exp, &generate_multiplication);
            return;
        case ast::binary_operator_token_t::DIVIDE:
            generate_binary_operation(assembly_output, binary_exp, &generate_division);
            return;
        case ast::binary_operator_token_t::MODULO:
            //generate_binary_operation(assembly_output, binary_exp, &generate_modulo);
            return;
        case ast::binary_operator_token_t::PLUS:
            generate_binary_operation(assembly_output, binary_exp, &generate_addition);
            return;
        case ast::binary_operator_token_t::MINUS:
            generate_binary_operation(assembly_output, binary_exp, &generate_subtraction);
            return;
        case ast::binary_operator_token_t::LEFT_BITSHIFT:
            //generate_binary_operation(assembly_output, binary_exp, &generate_left_bitshift);
            return;
        case ast::binary_operator_token_t::RIGHT_BITSHIFT:
            //generate_binary_operation(assembly_output, binary_exp, &generate_right_bitshift);
            return;
        case ast::binary_operator_token_t::LESS_THAN:
            generate_binary_operation(assembly_output, binary_exp, &generate_less_than);
            return;
        case ast::binary_operator_token_t::LESS_THAN_EQUAL:
            generate_binary_operation(assembly_output, binary_exp, &generate_less_than_equal);
            return;
        case ast::binary_operator_token_t::GREATER_THAN:
            generate_binary_operation(assembly_output, binary_exp, &generate_greater_than);
            return;
        case ast::binary_operator_token_t::GREATER_THAN_EQUAL:
            generate_binary_operation(assembly_output, binary_exp, &generate_greater_than_equal);
            return;
        case ast::binary_operator_token_t::EQUAL:
            generate_binary_operation(assembly_output, binary_exp, &generate_equality);
            return;
        case ast::binary_operator_token_t::NOT_EQUAL:
            generate_binary_operation(assembly_output, binary_exp, &generate_not_equals);
            return;
        case ast::binary_operator_token_t::BITWISE_AND:
            //generate_binary_operation(assembly_output, binary_exp, &generate_bitwise_and);
            return;
        case ast::binary_operator_token_t::BITWISE_XOR:
            //generate_binary_operation(assembly_output, binary_exp, &generate_bitwise_xor);
            return;
        case ast::binary_operator_token_t::BITWISE_OR:
            //generate_binary_operation(assembly_output, binary_exp, &generate_bitwise_or);
            return;
        case ast::binary_operator_token_t::LOGICAL_AND:
            generate_logical_and(assembly_output, binary_exp);
            return;
        case ast::binary_operator_token_t::LOGICAL_OR:
            generate_logical_or(assembly_output, binary_exp);
            return;
        case ast::binary_operator_token_t::ASSIGNMENT:
            generate_assignment_expression(assembly_output, binary_exp);
            return;
        case ast::binary_operator_token_t::COMMA:
            generate_binary_operation(assembly_output, binary_exp, &generate_comma);
            return;
    }
    throw std::runtime_error("invalid binary operator.");
}
void generate_unary_expression(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp) {
    if(unary_exp.fixity == ast::unary_operator_fixity_t::PREFIX) {
        switch(unary_exp.op) {
            case ast::unary_operator_token_t::PLUS_PLUS:
                //generate_unary_operation(assembly_output, unary_exp, &generate_prefix_plus_plus);
                return;
            case ast::unary_operator_token_t::MINUS_MINUS:
                //generate_unary_operation(assembly_output, unary_exp, &generate_prefix_minus_minus);
                return;
            case ast::unary_operator_token_t::PLUS:
                //generate_unary_operation(assembly_output, unary_exp, &generate_unary_plus);
                return;
            case ast::unary_operator_token_t::MINUS:
                generate_unary_operation(assembly_output, unary_exp, &generate_negation);
                return;
            case ast::unary_operator_token_t::LOGICAL_NOT:
                generate_unary_operation(assembly_output, unary_exp, &generate_logical_not);
                return;
            case ast::unary_operator_token_t::BITWISE_NOT:
                generate_unary_operation(assembly_output, unary_exp, &generate_bitwise_not);
                return;
        }
        throw std::runtime_error("invalid prefix unary operator.");
    } else if(unary_exp.fixity == ast::unary_operator_fixity_t::POSTFIX) {
        switch(unary_exp.op) {
            case ast::unary_operator_token_t::PLUS_PLUS:
                //generate_unary_operation(assembly_output, unary_exp, &generate_postfix_plus_plus);
                return;
            case ast::unary_operator_token_t::MINUS_MINUS:
                //generate_unary_operation(assembly_output, unary_exp, &generate_postfix_minus_minus);
                return;
        }
        throw std::runtime_error("invalid unary postfix operator.");
    } else {
        throw std::runtime_error("invalid unary operator fixity.");
    }
}
void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::grouping_t>& grouping) {
            generate_grouping(assembly_output, *grouping);
        },
        [&assembly_output](const std::shared_ptr<ast::binary_expression_t>& binary_exp) {
            generate_binary_expression(assembly_output, *binary_exp);
        },
        [&assembly_output](const std::shared_ptr<ast::unary_expression_t>& unary_exp) {
            generate_unary_expression(assembly_output, *unary_exp);
        },
        [&assembly_output](const ast::constant_t& constant) {
            store_constant(assembly_output, constant);
        },
        [&assembly_output](const ast::var_name_t& var_name) {
            pop_variable(assembly_output, var_name, "rax");
            store_register(assembly_output, "rax");
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
