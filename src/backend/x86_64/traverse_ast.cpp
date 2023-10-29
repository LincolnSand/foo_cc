#include "traverse_ast.hpp"


void generate_grouping(assembly_output_t& assembly_output, const ast::grouping_t& grouping) {
    generate_expression(assembly_output, grouping.expr);
}
void generate_convert(assembly_output_t& assembly_output, const ast::convert_t& convert) {

}
void generate_unary_expression(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp) {
    if(unary_exp.fixity == ast::unary_operator_fixity_t::PREFIX) {
        switch(unary_exp.op) {
            case ast::unary_operator_token_t::PLUS_PLUS:
                generate_prefix_plus_plus(assembly_output, unary_exp);
                return;
            case ast::unary_operator_token_t::MINUS_MINUS:
                generate_prefix_minus_minus(assembly_output, unary_exp);
                return;
            case ast::unary_operator_token_t::PLUS:
                // since number literals default to positive anyways, the unary plus operator is purely a decorative prefix and is otherwise ignored in codegen
                generate_expression(assembly_output, unary_exp.exp);
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
        throw std::runtime_error("Invalid prefix unary operator.");
    } else if(unary_exp.fixity == ast::unary_operator_fixity_t::POSTFIX) {
        switch(unary_exp.op) {
            case ast::unary_operator_token_t::PLUS_PLUS:
                generate_postfix_plus_plus(assembly_output, unary_exp);
                return;
            case ast::unary_operator_token_t::MINUS_MINUS:
                generate_postfix_minus_minus(assembly_output, unary_exp);
                return;
        }
        throw std::runtime_error("Invalid unary postfix operator.");
    } else {
        throw std::runtime_error("Invalid unary operator fixity.");
    }
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
            generate_binary_operation(assembly_output, binary_exp, &generate_modulo);
            return;
        case ast::binary_operator_token_t::PLUS:
            generate_binary_operation(assembly_output, binary_exp, &generate_addition);
            return;
        case ast::binary_operator_token_t::MINUS:
            generate_binary_operation(assembly_output, binary_exp, &generate_subtraction);
            return;
        case ast::binary_operator_token_t::LEFT_BITSHIFT:
            generate_binary_operation(assembly_output, binary_exp, &generate_left_bitshift);
            return;
        case ast::binary_operator_token_t::RIGHT_BITSHIFT:
            generate_binary_operation(assembly_output, binary_exp, &generate_right_bitshift);
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
            generate_binary_operation(assembly_output, binary_exp, &generate_bitwise_and);
            return;
        case ast::binary_operator_token_t::BITWISE_XOR:
            generate_binary_operation(assembly_output, binary_exp, &generate_bitwise_xor);
            return;
        case ast::binary_operator_token_t::BITWISE_OR:
            generate_binary_operation(assembly_output, binary_exp, &generate_bitwise_or);
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
    throw std::runtime_error("Invalid binary operator.");
}
void generate_ternary_expression(assembly_output_t& assembly_output, const ast::ternary_expression_t& ternary_exp) {

}
void push_params(assembly_output_t& assembly_output, const std::vector<ast::expression_t>& params) {

}
void pop_params(assembly_output_t& assembly_output, const ast::function_call_t& function_call) {

}
void generate_function_call(assembly_output_t& assembly_output, const ast::function_call_t& function_call_exp) {

}
void generate_variable_access(assembly_output_t& assembly_output, const ast::variable_access_t& var_name) {

}
void generate_constant(assembly_output_t& assembly_output, const ast::constant_t& constant) {
    std::visit(overloaded{
        [&assembly_output](const auto& value) {
            assembly_output.output += "pushq $" + std::to_string(value) + "\n";
        },
        [](float) {},
        [](double) {},
        [](long double) {}
    }, constant.value);
}

void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::grouping_t>& grouping) {
            generate_grouping(assembly_output, *grouping);
        },
        [&assembly_output](const std::shared_ptr<ast::convert_t>& convert) {
            generate_convert(assembly_output, *convert);
        },
        [&assembly_output](const std::shared_ptr<ast::unary_expression_t>& unary_exp) {
            generate_unary_expression(assembly_output, *unary_exp);
        },
        [&assembly_output](const std::shared_ptr<ast::binary_expression_t>& binary_exp) {
            generate_binary_expression(assembly_output, *binary_exp);
        },
        [&assembly_output](const std::shared_ptr<ast::ternary_expression_t>& ternary_exp) {
            generate_ternary_expression(assembly_output, *ternary_exp);
        },
        [&assembly_output](const std::shared_ptr<ast::function_call_t>& function_call_exp) {
            generate_function_call(assembly_output, *function_call_exp);
        },
        [&assembly_output](const ast::variable_access_t& var_name) {
            generate_variable_access(assembly_output, var_name);
        },
        [&assembly_output](const ast::constant_t& constant) {
            generate_constant(assembly_output, constant);
        },
    }, expression.expr);
}

void generate_return_statement(assembly_output_t& assembly_output, const ast::return_statement_t& return_stmt) {
    if(is_integral(return_stmt.expr.type.value())) {
        generate_expression(assembly_output, return_stmt.expr);
        assembly_output.output += "popq %rax\n";
    }
}
void generate_if_statement(assembly_output_t& assembly_output, const ast::if_statement_t& if_stmt) {

}
void generate_statement(assembly_output_t& assembly_output, const ast::statement_t& stmt) {
    std::visit(overloaded{
        [&assembly_output](const ast::return_statement_t& stmt) {
            generate_return_statement(assembly_output, stmt);
        },
        [&assembly_output](const ast::expression_statement_t& stmt) {
            if(stmt.expr.has_value()) {
                generate_expression(assembly_output, stmt.expr.value());
            }
        },
        [&assembly_output](const std::shared_ptr<ast::if_statement_t>& stmt) {
            generate_if_statement(assembly_output, *stmt);
        },
        [&assembly_output](const std::shared_ptr<ast::compound_statement_t>& stmt) {
            generate_compound_statement(assembly_output, *stmt);
        }
    }, stmt);
}

void generate_declaration(assembly_output_t& assembly_output, const ast::declaration_t& decl) {

}
void generate_compound_statement(assembly_output_t& assembly_output, const ast::compound_statement_t& compound_stmt, const bool is_function) {
    if(!is_function) {
        assembly_output.variable_lookup.create_new_scope(); // new scope is created by caller for function definitions since the parameter variable names need to be declared in the function scope
    }

    for(const auto& stmt : compound_stmt.stmts) {
        std::visit(overloaded{
            [&assembly_output](const ast::statement_t& stmt) {
                generate_statement(assembly_output, stmt);
            },
            [&assembly_output](const ast::declaration_t& stmt) {
                generate_declaration(assembly_output, stmt);
            }
        }, stmt);
    }

    const auto rsp_offset = assembly_output.variable_lookup.destroy_current_scope();
    if(!has_return_statement(compound_stmt)) { // functions destroy their block scope in the return statement
        assembly_output.output += "addq $" + std::to_string(rsp_offset) + ", %rsp\n"; // no need to emit instructions after `ret` as they are unreachable
    }
    assembly_output.current_rbp_offset -= rsp_offset;
}
void generate_function_definition(assembly_output_t& assembly_output, const ast::function_definition_t& function_definition) {
    assembly_output.output += ".text\n";
    assembly_output.output += ".globl " + function_definition.function_name + "\n";
    assembly_output.output += function_definition.function_name + ":\n";
    generate_function_prologue(assembly_output);
    generate_compound_statement(assembly_output, function_definition.statements);
    generate_function_epilogue(assembly_output);
}
void generate_global_variable_definition(assembly_output_t& assembly_output, const ast::global_variable_declaration_t& global_var_def) {
    // For now, we will only allocate and use .data, but we will use .rodata and .bss in the future
    const auto required_alignment = global_var_def.type_name.alignment.value();
    const auto allocation_size = global_var_def.type_name.size.value();
    if(!global_var_def.value.has_value() || is_constant_with_value_zero(global_var_def.value.value())) {
        assembly_output.output += ".bss\n";
        assembly_output.output += ".align " + std::to_string(required_alignment) + "\n";
        assembly_output.output += ".globl " + global_var_def.var_name + "\n";
        assembly_output.output += global_var_def.var_name + ":\n";
        assembly_output.output += ".zero " + std::to_string(allocation_size) + "\n";
    } else {
        assembly_output.output += ".data\n";
        assembly_output.output += ".align " + std::to_string(required_alignment) + "\n";
        assembly_output.output += ".globl " + global_var_def.var_name + "\n";
        assembly_output.output += global_var_def.var_name + ":\n";
        auto remaining_amount_to_allocate = allocation_size;
        const type_punned_constant_t type_punned_constant = get_type_punned_constant(global_var_def.value.value());
        std::uint64_t current_byte_index = 0u;
        // TODO: Maybe pull out these branches into a separate templated function to get rid of code duplication.
        while(remaining_amount_to_allocate != 0) {
            if(remaining_amount_to_allocate >= sizeof(std::uint64_t)) {
                std::uint64_t value{};
                std::memcpy(&value, type_punned_constant.bytes.get() + current_byte_index, sizeof(value));
                assembly_output.output += ".quad " + std::to_string(value) + "\n";
                remaining_amount_to_allocate -= sizeof(value);
                current_byte_index += sizeof(value);
            } else if(remaining_amount_to_allocate >= sizeof(std::uint32_t)) {
                std::uint32_t value{};
                std::memcpy(&value, type_punned_constant.bytes.get() + current_byte_index, sizeof(value));
                assembly_output.output += ".long " + std::to_string(value) + "\n";
                remaining_amount_to_allocate -= sizeof(value);
                current_byte_index += sizeof(value);
            } else if(remaining_amount_to_allocate >= sizeof(std::uint16_t)) {
                std::uint16_t value{};
                std::memcpy(&value, type_punned_constant.bytes.get() + current_byte_index, sizeof(value));
                assembly_output.output += ".word " + std::to_string(value) + "\n";
                remaining_amount_to_allocate -= sizeof(value);
                current_byte_index += sizeof(value);
            } else {
                std::uint8_t value{};
                std::memcpy(&value, type_punned_constant.bytes.get() + current_byte_index, sizeof(value));
                assembly_output.output += ".byte " + std::to_string(value) + "\n";
                remaining_amount_to_allocate -= sizeof(value);
                current_byte_index += sizeof(value);
            }
        }
    }
}

void generate_program(assembly_output_t& assembly_output, const ast::validated_program_t& program) {
    for(const auto& top_level_decl : program.top_level_declarations) {
        std::visit(overloaded{
            [&assembly_output](const ast::function_definition_t& function_def) {
                generate_function_definition(assembly_output, function_def);
            },
            [&assembly_output](const ast::global_variable_declaration_t& global_var_def) {
                generate_global_variable_definition(assembly_output, global_var_def);
            }
        }, top_level_decl);
    }
}
std::string generate_asm(const ast::validated_program_t& program) {
    assembly_output_t assembly_output;
    generate_program(assembly_output, program);
    return assembly_output.output;
}
