#include "compile_operators.hpp"


void store_constant(assembly_output_t& assembly_output, const ast::constant_t& constant) {
    assembly_output.output += "movq $" + std::to_string(constant.value) + ", %rcx\n";
    assembly_output.output += "pushq %rcx\n";
}
void store_register(assembly_output_t& assembly_output, const std::string& register_name) {
    assembly_output.output += "pushq %" + register_name + "\n";
}
void store_variable(assembly_output_t& assembly_output, const ast::var_name_t& variable_name, const std::string& register_name) {
    const auto variable = assembly_output.variable_lookup.find_from_lowest_scope(variable_name).value();

    assembly_output.output += "movq %" + register_name + ", -" + std::to_string(variable) + "(%rbp)\n";

    // assignments are expressions in C, so we push the new value of the variable onto the stack
    assembly_output.output += "pushq %" + register_name + "\n";
}
void pop_register(assembly_output_t& assembly_output, const std::string& register_name) {
    assembly_output.output += "popq %" + register_name + "\n";
}
void pop_variable(assembly_output_t& assembly_output, const ast::var_name_t& variable_name, const std::string& register_name) {
    const auto variable = assembly_output.variable_lookup.find_from_lowest_scope(variable_name).value();

    assembly_output.output += "movq -" + std::to_string(variable) + "(%rbp), %" + register_name + "\n";
}

void allocate_stack_space_for_variable(assembly_output_t& assembly_output) {
    assembly_output.output += "subq $" + std::to_string(sizeof(std::uint64_t)) + ", %rsp\n"; // TODO: we only support 64 bit integers currently
}

void generate_negation(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rax");
    assembly_output.output += "neg %rax\n";
    store_register(assembly_output, "rax");
}
void generate_bitwise_not(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rax");
    assembly_output.output += "not %rax\n";
    store_register(assembly_output, "rax");
}
void generate_logical_not(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rax");
    assembly_output.output += "cmpq $0, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "sete %al\n";
    store_register(assembly_output, "rax");
}

void generate_multiplication(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "imulq %rcx, %rax\n";
    store_register(assembly_output, "rax");
}
void generate_division(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "cqo\n"; // trash `rdx` by sign extending into `rdx`. Needed for `idiv` dividend.
    assembly_output.output += "idivq %rcx\n";
    store_register(assembly_output, "rax");
}
void generate_modulo(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "cqo\n"; // trash `rdx` by sign extending into `rdx`. Needed for `idiv` dividend.
    assembly_output.output += "idivq %rcx\n";
    store_register(assembly_output, "rcx");
}
void generate_addition(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "addq %rcx, %rax\n";
    store_register(assembly_output, "rax");
}
void generate_subtraction(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "subq %rcx, %rax\n";
    store_register(assembly_output, "rax");
}
void generate_left_bitshift(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "sall %rcx, %rax\n";
    store_register(assembly_output, "rax");
}
void generate_right_bitshift(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "shrw %rcx, %rax\n";
    store_register(assembly_output, "rax");
}
void generate_less_than(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "cmpq %rcx, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setl %al\n";
    store_register(assembly_output, "rax");
}
void generate_greater_than(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "cmpq %rcx, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setg %al\n";
    store_register(assembly_output, "rax");
}
void generate_less_than_equal(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "cmpq %rcx, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setle %al\n";
    store_register(assembly_output, "rax");
}
void generate_greater_than_equal(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "cmpq %rcx, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setge %al\n";
    store_register(assembly_output, "rax");
}
void generate_equality(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "cmpq %rcx, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "sete %al\n";
    store_register(assembly_output, "rax");
}
void generate_not_equals(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "cmpq %rcx, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setne %al\n";
    store_register(assembly_output, "rax");
}
void generate_bitwise_and(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "andq %rcx, %rax\n";
    store_register(assembly_output, "rax");
}
void generate_bitwise_xor(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "xorq %rcx, %rax\n";
    store_register(assembly_output, "rax");
}
void generate_bitwise_or(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rcx");
    pop_register(assembly_output, "rax");
    assembly_output.output += "orq %rcx, %rax\n";
    store_register(assembly_output, "rax");
}
void generate_comma(assembly_output_t& assembly_output) {
    pop_register(assembly_output, "rax");
    pop_register(assembly_output, "rcx");

    store_register(assembly_output, "rax"); // comma operator discards result of left hand result after evaluation of expression
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

void generate_function_prologue(assembly_output_t& assembly_output) {
    assembly_output.output += "pushq %rbp\n";
    assembly_output.output += "movq %rsp, %rbp\n";

    assembly_output.current_rbp_offset = 0;

    assembly_output.output += "pushq %rbx\n";
    assembly_output.output += "pushq %r12\n";
    assembly_output.output += "pushq %r13\n";
    assembly_output.output += "pushq %r14\n";
    assembly_output.output += "pushq %r15\n";

    assembly_output.current_rbp_offset += 5*sizeof(std::uint64_t);
}
void generate_function_epilogue(assembly_output_t& assembly_output) {
    assembly_output.output += "popq %rbx\n";
    assembly_output.output += "popq %r12\n";
    assembly_output.output += "popq %r13\n";
    assembly_output.output += "popq %r14\n";
    assembly_output.output += "popq %r15\n";

    const auto offset = assembly_output.current_rbp_offset;
    if(offset != (5*sizeof(std::uint64_t))) {
        std::cout << "offset: " << offset << '\n';
        throw std::runtime_error("Function epilogue called with extra data on stack.");
    }

    assembly_output.output += "movq %rbp, %rsp\n";
    assembly_output.output += "popq %rbp\n";
    assembly_output.output += "ret\n";
}
