#include "compile_operators.hpp"


// trashes `rcx`
// TODO: stop this from trashing `rcx`
void store_constant(assembly_output_t& assembly_output, const ast::constant_t& constant) {
    assembly_output.output += "movq $" + std::to_string(constant.value) + ", %rcx\n";
    assembly_output.output += "pushq %rcx\n";
}
void store_register(assembly_output_t& assembly_output, const std::string& register_name) {
    assembly_output.output += "pushq %" + register_name + "\n";
}
void store_variable(assembly_output_t& assembly_output, const ast::var_name_t variable_name, const std::string& register_name) {
    const auto offset = assembly_output.variable_lookup.find_from_lowest_scope(variable_name);
    if(!offset.has_value()) {
        throw std::runtime_error("Variable " + variable_name + " not declared in current scope.");
    }

    assembly_output.output += "movq %" + register_name + ", -" + std::to_string(*offset) + "(%rbp)\n";

    // assignments are expressions in C, so we push the new value of the variable onto the stack
    assembly_output.output += "pushq %" + register_name + "\n";
}
void pop_constant(assembly_output_t& assembly_output, const std::string& register_name) {
    assembly_output.output += "popq %" + register_name + "\n";
}
void pop_variable(assembly_output_t& assembly_output, const ast::var_name_t variable_name, const std::string& register_name) {
    const auto offset = assembly_output.variable_lookup.find_from_lowest_scope(variable_name);
    if(!offset.has_value()) {
        throw std::runtime_error("Variable " + variable_name + " not declared in current scope.");
    }

    assembly_output.output += "movq -" + std::to_string(*offset) + "(%rbp), %" + register_name + "\n";
}

void allocate_stack_space_for_variable(assembly_output_t& assembly_output) {
    assembly_output.output += "sub $8, %rsp\n"; // TODO: we only support 64 bit integers currently
}

void generate_negation(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rax");
    assembly_output.output += "neg %rax\n";
    store_register(assembly_output, "rax");
}
void generate_bitwise_not(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rax");
    assembly_output.output += "not %rax\n";
    store_register(assembly_output, "rax");
}
void generate_logic_not(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rax");
    assembly_output.output += "cmpq $0, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "sete %al\n";
    store_register(assembly_output, "rax");
}
void generate_multiplication(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");
    assembly_output.output += "imulq %rcx, %rax\n";
    store_register(assembly_output, "rax");
}
void generate_division(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");
    assembly_output.output += "cqo\n"; // trash `rdx` by sign extending into `rdx`. Needed for `idiv` dividend.
    assembly_output.output += "idivq %rcx\n";
    store_register(assembly_output, "rax");
}
void generate_addition(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");
    assembly_output.output += "addq %rcx, %rax\n";
    store_register(assembly_output, "rax");
}
void generate_subtraction(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");
    assembly_output.output += "subq %rcx, %rax\n";
    store_register(assembly_output, "rax");
}
void generate_less_than(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");
    assembly_output.output += "cmpq %rcx, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setl %al\n";
    store_register(assembly_output, "rax");
}
void generate_greater_than(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");
    assembly_output.output += "cmpq %rcx, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setg %al\n";
    store_register(assembly_output, "rax");
}
void generate_less_than_equal(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");
    assembly_output.output += "cmpq %rcx, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setle %al\n";
    store_register(assembly_output, "rax");
}
void generate_greater_than_equal(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");
    assembly_output.output += "cmpq %rcx, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setge %al\n";
    store_register(assembly_output, "rax");
}
void generate_equality(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");
    assembly_output.output += "cmpq %rcx, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "sete %al\n";
    store_register(assembly_output, "rax");
}
void generate_not_equals(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");
    assembly_output.output += "cmpq %rcx, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setne %al\n";
    store_register(assembly_output, "rax");
}

void generate_function_epilogue(assembly_output_t& assembly_output) {
    assembly_output.output += "movq %rbp, %rsp\n";
    assembly_output.output += "popq %rbp\n";
    assembly_output.output += "ret\n";
}
void generate_function_prologue(assembly_output_t& assembly_output) {
    assembly_output.output += "pushq %rbp\n";
    assembly_output.output += "movq %rsp, %rbp\n";
}
