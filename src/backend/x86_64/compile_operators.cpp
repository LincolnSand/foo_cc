#include "compile_operators.hpp"



void store_constant(assembly_output_t& assembly_output, const ast::constant_t& constant) {

}
void store_register(assembly_output_t& assembly_output, const std::string& register_name) {

}
void store_variable(assembly_output_t& assembly_output, const ast::variable_access_t& variable_name, const std::string& register_name) {

}
void pop_register(assembly_output_t& assembly_output, const std::string& register_name) {

}
void pop_variable(assembly_output_t& assembly_output, const ast::variable_access_t& variable_name, const std::string& register_name) {

}

void allocate_stack_space_for_variable(assembly_output_t& assembly_output) {

}

void generate_negation(assembly_output_t& assembly_output) {

}
void generate_bitwise_not(assembly_output_t& assembly_output) {

}
void generate_logical_not(assembly_output_t& assembly_output) {

}

void generate_multiplication(assembly_output_t& assembly_output) {

}
void generate_division(assembly_output_t& assembly_output) {

}
void generate_modulo(assembly_output_t& assembly_output) {

}
void generate_addition(assembly_output_t& assembly_output) {

}
void generate_subtraction(assembly_output_t& assembly_output) {

}
void generate_left_bitshift(assembly_output_t& assembly_output) {

}
void generate_right_bitshift(assembly_output_t& assembly_output) {

}
void generate_less_than(assembly_output_t& assembly_output) {

}
void generate_greater_than(assembly_output_t& assembly_output) {

}
void generate_less_than_equal(assembly_output_t& assembly_output) {

}
void generate_greater_than_equal(assembly_output_t& assembly_output) {

}
void generate_equality(assembly_output_t& assembly_output) {

}
void generate_not_equals(assembly_output_t& assembly_output) {

}
void generate_bitwise_and(assembly_output_t& assembly_output) {

}
void generate_bitwise_xor(assembly_output_t& assembly_output) {

}
void generate_bitwise_or(assembly_output_t& assembly_output) {

}
void generate_comma(assembly_output_t& assembly_output) {

}

void generate_prefix_plus_plus(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp) {
    generate_expression(assembly_output, unary_exp.exp); // first evaluate the interior expression so we handle any side effects

    const auto var_name = validate_lvalue_expression_exp(unary_exp.exp);

    pop_variable(assembly_output, var_name, "rax");
    assembly_output.output += "addq $1, %rax\n";
    store_variable(assembly_output, var_name, "rax");
    store_register(assembly_output, "rax"); // return the variable value after the increment
}
void generate_prefix_minus_minus(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp) {
    generate_expression(assembly_output, unary_exp.exp); // first evaluate the interior expression so we handle any side effects

    const auto var_name = validate_lvalue_expression_exp(unary_exp.exp);

    pop_variable(assembly_output, var_name, "rax");
    assembly_output.output += "subq $1, %rax\n";
    store_variable(assembly_output, var_name, "rax");
    store_register(assembly_output, "rax"); // return the variable value after the decrement
}
void generate_postfix_plus_plus(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp) {
    generate_expression(assembly_output, unary_exp.exp); // first evaluate the interior expression so we handle any side effects

    const auto var_name = validate_lvalue_expression_exp(unary_exp.exp);

    pop_variable(assembly_output, var_name, "rax");
    assembly_output.output += "movq %rax, %rcx\n"; // store the variable value before the increment
    assembly_output.output += "addq $1, %rax\n";
    store_variable(assembly_output, var_name, "rax");
    store_register(assembly_output, "rcx"); // return the variable value before the increment
}
void generate_postfix_minus_minus(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp) {
    generate_expression(assembly_output, unary_exp.exp); // first evaluate the interior expression so we handle any side effects

    const auto var_name = validate_lvalue_expression_exp(unary_exp.exp);

    pop_variable(assembly_output, var_name, "rax");
    assembly_output.output += "movq %rax, %rcx\n"; // store the variable value before the increment
    assembly_output.output += "subq $1, %rax\n";
    store_variable(assembly_output, var_name, "rax");
    store_register(assembly_output, "rcx"); // return the variable value before the increment
}

void generate_function_prologue(assembly_output_t& assembly_output) { // TODO: handle passing other types of parameters than just integer types
    assembly_output.output += "pushq %rbp\n";
    assembly_output.output += "movq %rsp, %rbp\n";

    assembly_output.current_rbp_offset = 0;
}
void generate_function_epilogue(assembly_output_t& assembly_output) { // TODO: handle passing other types of parameters than just integer types
    assembly_output.output += "leave\n";
    assembly_output.output += "ret\n";
}
