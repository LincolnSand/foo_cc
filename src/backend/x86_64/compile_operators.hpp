#pragma once

#include <variant>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <stack>

#include <frontend/ast/ast.hpp>
#include <frontend/parsing/parser_utils.hpp>
#include <utils/data_structures/random_access_stack.hpp>


namespace backend {
struct parameters_info_t {
    // TODO: Note `long double` will NOT be supported in the near future because of how complex its calling convention is
    std::uint32_t number_of_INTEGER_class_parameters = 0u;
    std::uint32_t number_of_SEE_class_parameters = 0u;
    std::uint32_t number_of_MEMORY_class_parameters = 0u;
};
}

struct assembly_output_t  {
    std::string output;

    // NOTE: Because of the .align gas directive, we don't need to align global variables

    std::uint64_t current_label_number = 0u; // appended to the end of labels so we don't get duplicate labels for things like boolean short circuiting and if statements

    utils::data_structures::backend_variable_lookup_t variable_lookup;

    // For now, we will just align all variables to 8 bytes (64 bits) for stack allocations
    std::uint64_t current_rbp_offset; // appended to `-` symbol since the stack grows downwards on x86_64

    backend::parameters_info_t parameters_info;

    ast::type_table_t type_table;

    assembly_output_t() = default;
    assembly_output_t(std::string output) : output(std::move(output)) {}
};

// Put this here despite it being defined in `traverse_ast.cpp` and already having its prototype in `traverse_ast.hpp` since some of the functions (prefix/postfix `++`/`--`) in this file have to call it.
void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression);

void store_constant(assembly_output_t& assembly_output, const ast::constant_t& constant);
void store_register(assembly_output_t& assembly_output, const std::string& register_name);
void store_variable(assembly_output_t& assembly_output, const ast::variable_access_t& variable_name, const std::string& register_name);
void pop_register(assembly_output_t& assembly_output, const std::string& register_name);
void pop_variable(assembly_output_t& assembly_output, const ast::variable_access_t& variable_name, const std::string& register_name);

void allocate_stack_space_for_variable(assembly_output_t& assembly_output);

void generate_negation(assembly_output_t& assembly_output);
void generate_bitwise_not(assembly_output_t& assembly_output);
void generate_logical_not(assembly_output_t& assembly_output);

void generate_multiplication(assembly_output_t& assembly_output);
void generate_division(assembly_output_t& assembly_output);
void generate_modulo(assembly_output_t& assembly_output);
void generate_addition(assembly_output_t& assembly_output);
void generate_subtraction(assembly_output_t& assembly_output);
void generate_left_bitshift(assembly_output_t& assembly_output);
void generate_right_bitshift(assembly_output_t& assembly_output);
void generate_less_than(assembly_output_t& assembly_output);
void generate_greater_than(assembly_output_t& assembly_output);
void generate_less_than_equal(assembly_output_t& assembly_output);
void generate_greater_than_equal(assembly_output_t& assembly_output);
void generate_equality(assembly_output_t& assembly_output);
void generate_not_equals(assembly_output_t& assembly_output);
void generate_bitwise_and(assembly_output_t& assembly_output);
void generate_bitwise_xor(assembly_output_t& assembly_output);
void generate_bitwise_or(assembly_output_t& assembly_output);
void generate_comma(assembly_output_t& assembly_output);

void generate_prefix_plus_plus(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp);
void generate_prefix_minus_minus(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp);
void generate_postfix_plus_plus(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp);
void generate_postfix_minus_minus(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp);

void generate_function_prologue(assembly_output_t& assembly_output);
void generate_function_epilogue(assembly_output_t& assembly_output);
