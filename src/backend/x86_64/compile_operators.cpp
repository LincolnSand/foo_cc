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
void pop_constant(assembly_output_t& assembly_output, const std::string& register_name) {
    assembly_output.output += "popq %" + register_name + "\n";
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

void generate_logical_and(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");

    assembly_output.output += "cmpq $0, %rax\n";
    std::string clause_2_label_name = "_clause2_" + std::to_string(assembly_output.current_label_number++);
    std::string end_label_name = "_end_" + std::to_string(assembly_output.current_label_number++);
    assembly_output.output += "jne " + clause_2_label_name + "\n";

    // short circuit on false
    assembly_output.output += "movq $1, %rax\n";
    assembly_output.output += "jmp " + end_label_name + "\n";

    assembly_output.output += clause_2_label_name + ":\n";
    assembly_output.output += "cmpq $0, %rcx\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setne %al\n";
    assembly_output.output += end_label_name + ":\n";

    store_register(assembly_output, "rax");
}
void generate_logical_or(assembly_output_t& assembly_output) {
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");

    assembly_output.output += "cmpq $0, %rax\n";
    std::string clause_2_label_name = "_clause2_" + std::to_string(assembly_output.current_label_number++);
    std::string end_label_name = "_end_" + std::to_string(assembly_output.current_label_number++);
    assembly_output.output += "je " + clause_2_label_name + "\n";

    // short circuit on true
    assembly_output.output += "movq $1, %rax\n";
    assembly_output.output += "jmp " + end_label_name + "\n";

    assembly_output.output += clause_2_label_name + ":\n";
    assembly_output.output += "cmpq $0, %rcx\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setne %al\n";
    assembly_output.output += end_label_name + ":\n";

    store_register(assembly_output, "rax");
}
