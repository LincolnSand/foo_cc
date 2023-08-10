#include "traverse_ast.hpp"


void generate_grouping(assembly_output_t& assembly_output, const ast::grouping_t& grouping) {
    generate_expression(assembly_output, grouping.expr);
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
    generate_expression(assembly_output, ternary_exp.condition);
    pop_register(assembly_output, "rax");
    const std::string else_label = "_else_" + std::to_string(assembly_output.current_label_number++);
    const std::string end_label_name = "_end_" + std::to_string(assembly_output.current_label_number++);
    assembly_output.output += "cmpq $0, %rax\n";
    assembly_output.output += "je " + else_label + "\n";
    generate_expression(assembly_output, ternary_exp.if_true);
    assembly_output.output += "jmp " + end_label_name + "\n";
    assembly_output.output += else_label + ":\n";
    generate_expression(assembly_output, ternary_exp.if_false);
    assembly_output.output += end_label_name + ":\n";
}
void push_params(assembly_output_t& assembly_output, const std::vector<ast::expression_t>& params) {
    for(const auto& param : params) {
        generate_expression(assembly_output, param);
    }

    if(params.size() >= 1) {
        pop_register(assembly_output, "rdi");
    }
    if(params.size() >= 2) {
        pop_register(assembly_output, "rsi");
    }
    if(params.size() >= 3) {
        pop_register(assembly_output, "rdx");
    }
    if(params.size() >= 4) {
        pop_register(assembly_output, "rcx");
    }
    if(params.size() >= 5) {
        pop_register(assembly_output, "r8");
    }
    if(params.size() >= 6) {
        pop_register(assembly_output, "r9");
    }
    if(params.size() >= 7); // values are already on the stack in reverse order, so no need to do anything here

    // callee preserves the registers `rbx`, `rsp`, `rbp`, `r12`, `r13`, `r14`, and `r15`.
}
void pop_params(assembly_output_t& assembly_output, const ast::function_call_t& function_call) {
    if(function_call.params.size() > 6) { // parameter isn't in register
        assembly_output.output += "addq $" + std::to_string(function_call.params.size()-6) + ", %rsp\n"; // deallocate caller allocated parameters
    }
}
void generate_function_call(assembly_output_t& assembly_output, const ast::function_call_t& function_call_exp) {
    push_params(assembly_output, function_call_exp.params);
    bool needs_alignment = false;
    if((assembly_output.current_rbp_offset % 16) == sizeof(std::uint64_t)) {
        assembly_output.output += "subq $" + std::to_string(sizeof(std::uint64_t)) + ", %rsp\n";
        assembly_output.current_rbp_offset += sizeof(std::uint64_t);
        needs_alignment = true;
    } else if(assembly_output.current_rbp_offset % 16 != 0) {
        throw std::logic_error("Invalid stack alignment.");
    }
    assembly_output.output += "call " + function_call_exp.function_name + "\n";
    if(needs_alignment) {
        assembly_output.output += "addq $" + std::to_string(sizeof(std::uint64_t)) + ", %rsp\n";
        assembly_output.current_rbp_offset -= sizeof(std::uint64_t);
    }
    pop_params(assembly_output, function_call_exp); // does not trash or affect the current register values after function call
    store_register(assembly_output, "rax"); // push the return value of the function call
}

void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::grouping_t>& grouping) {
            generate_grouping(assembly_output, *grouping);
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
        [&assembly_output](const ast::constant_t& constant) {
            store_constant(assembly_output, constant);
        },
        [&assembly_output](const ast::var_name_t& var_name) {
            pop_variable(assembly_output, var_name, "rax");
            store_register(assembly_output, "rax");
        }
    }, expression);
}

void generate_return_statement(assembly_output_t& assembly_output, const ast::return_statement_t& return_stmt) {
    generate_expression(assembly_output, return_stmt.expr);

    pop_register(assembly_output, "rax");

    const auto rsp_offset = assembly_output.variable_lookup.get_total_stack_size();
    assembly_output.output += "addq $" + std::to_string(rsp_offset) + ", %rsp\n";
    assembly_output.current_rbp_offset -= rsp_offset;

    generate_function_epilogue(assembly_output);

    assembly_output.current_rbp_offset += rsp_offset;
}
void generate_if_statement(assembly_output_t& assembly_output, const ast::if_statement_t& if_stmt) {
    generate_expression(assembly_output, if_stmt.if_exp);
    pop_register(assembly_output, "rax");
    const std::string else_label = "_else_" + std::to_string(assembly_output.current_label_number++);
    const std::string end_label_name = "_end_" + std::to_string(assembly_output.current_label_number++);
    assembly_output.output += "cmpq $0, %rax\n";
    assembly_output.output += "je " + else_label + "\n";
    generate_statement(assembly_output, if_stmt.if_body);
    assembly_output.output += "jmp " + end_label_name + "\n";
    assembly_output.output += else_label + ":\n";
    if(if_stmt.else_body.has_value()) {
        generate_statement(assembly_output, *if_stmt.else_body);
    }
    assembly_output.output += end_label_name + ":\n";
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
    // start off at `sizeof(std::uint64_t)` instead of `0` since we read from low to high memory address and we negate the offset from rbp in the emited code, so the first byte is `-8` and the range is [-8, 0) instead of [0, -8).
    assembly_output.variable_lookup.add_new_variable_in_current_scope(decl.var_name, (assembly_output.current_rbp_offset += sizeof(std::uint64_t))); // TODO: we currently only support 64 bit integer type

    allocate_stack_space_for_variable(assembly_output);

    if(decl.value.has_value()) {
        generate_expression(assembly_output, *decl.value);

        pop_register(assembly_output, "rax");
        store_variable(assembly_output, decl.var_name, "rax");
    }
}
bool has_return_statement(const ast::compound_statement_t& compound_stmt) {
    for(const auto& stmt : compound_stmt.stmts) {
        const bool is_return = std::visit(overloaded{
            [](const ast::statement_t& stmt) {
                return std::visit(overloaded{
                    [](const ast::return_statement_t&) {
                        return true;
                    },
                    [](const auto&) {
                        return false;
                    }
                }, stmt);
            },
            [](const ast::declaration_t&) {
                return false;
            }
        }, stmt);
        if(is_return) {
            return true;
        }
    }
    return false;
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
    assembly_output.output += ".globl ";
    assembly_output.output += function_definition.function_name;
    assembly_output.output += "\n";
    assembly_output.output += function_definition.function_name;
    assembly_output.output += ":\n";

    generate_function_prologue(assembly_output);

    assembly_output.variable_lookup.create_new_scope();
    for(auto i = 0; i < function_definition.params.size(); ++i) {
        if(function_definition.params.at(i).second.has_value()) {
            if(i == (function_definition.params.size()-1)) {
                store_register(assembly_output, "rdi");
                assembly_output.current_rbp_offset += sizeof(std::uint64_t);
            } else if(i == (function_definition.params.size()-2)) {
                store_register(assembly_output, "rsi");
                assembly_output.current_rbp_offset += sizeof(std::uint64_t);
            } else if(i == (function_definition.params.size()-3)) {
                store_register(assembly_output, "rdx");
                assembly_output.current_rbp_offset += sizeof(std::uint64_t);
            } else if(i == (function_definition.params.size()-4)) {
                store_register(assembly_output, "rcx");
                assembly_output.current_rbp_offset += sizeof(std::uint64_t);
            } else if(i == (function_definition.params.size()-5)) {
                store_register(assembly_output, "r8");
                assembly_output.current_rbp_offset += sizeof(std::uint64_t);
            } else if(i == (function_definition.params.size()-6)) {
                store_register(assembly_output, "r9");
                assembly_output.current_rbp_offset += sizeof(std::uint64_t);
            } else {
                assembly_output.output += "movq " + std::to_string(i*sizeof(std::uint64_t)) + "(%rbp), -" + std::to_string(assembly_output.current_rbp_offset += sizeof(std::uint64_t)) + "(%rbp)\n";
            }
            assembly_output.variable_lookup.add_new_variable_in_current_scope(function_definition.params.at(i).second.value(), assembly_output.current_rbp_offset);
        } // ignore anonymous parameters as they cannot be referenced by the callee
    }
    generate_compound_statement(assembly_output, function_definition.statements, true);
}

void generate_program(assembly_output_t& assembly_output, const ast::program_t& program) {
    for(const auto& decl : program.declarations) {
        generate_declaration(assembly_output, decl);
    }
    // function declarations do not get emited into the assembly. They exist for the purposes of the validation pass and for lookup when emiting function calls.
    for(const auto& function_definition : program.function_definitions) {
        generate_function_definition(assembly_output, function_definition);
    }
}
std::string generate_asm(const ast::program_t& program) {
    assembly_output_t assembly_output;
    generate_program(assembly_output, program);
    return assembly_output.output;
}
