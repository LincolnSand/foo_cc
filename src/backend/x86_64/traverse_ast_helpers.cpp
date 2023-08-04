#include "traverse_ast_helpers.hpp"


void generate_binary_operation(assembly_output_t& assembly_output, const ast::binary_expression_t& binary_exp, void(*func)(assembly_output_t&)) {
    generate_expression(assembly_output, binary_exp.left);
    generate_expression(assembly_output, binary_exp.right);

    func(assembly_output);
}
void generate_logical_and(assembly_output_t& assembly_output, const ast::binary_expression_t& binary_exp) {
    generate_expression(assembly_output, binary_exp.left);

    pop_constant(assembly_output, "rax");

    assembly_output.output += "cmpq $0, %rax\n";
    std::string clause_2_label_name = "_clause2_" + std::to_string(assembly_output.current_label_number++);
    std::string end_label_name = "_end_" + std::to_string(assembly_output.current_label_number++);
    assembly_output.output += "jne " + clause_2_label_name + "\n";

    // short circuit on false
    assembly_output.output += "movq $1, %rax\n";
    assembly_output.output += "jmp " + end_label_name + "\n";

    assembly_output.output += clause_2_label_name + ":\n";

    generate_expression(assembly_output, binary_exp.right);
    pop_constant(assembly_output, "rax");

    assembly_output.output += "cmpq $0, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setne %al\n";
    assembly_output.output += end_label_name + ":\n";

    store_register(assembly_output, "rax");
}
void generate_logical_or(assembly_output_t& assembly_output, const ast::binary_expression_t& binary_exp) {
    generate_expression(assembly_output, binary_exp.left);

    pop_constant(assembly_output, "rax");

    assembly_output.output += "cmpq $0, %rax\n";
    std::string clause_2_label_name = "_clause2_" + std::to_string(assembly_output.current_label_number++);
    std::string end_label_name = "_end_" + std::to_string(assembly_output.current_label_number++);
    assembly_output.output += "je " + clause_2_label_name + "\n";

    // short circuit on true
    assembly_output.output += "movq $1, %rax\n";
    assembly_output.output += "jmp " + end_label_name + "\n";

    assembly_output.output += clause_2_label_name + ":\n";

    generate_expression(assembly_output, binary_exp.right);
    pop_constant(assembly_output, "rax");

    assembly_output.output += "cmpq $0, %rax\n";
    assembly_output.output += "movq $0, %rax\n";
    assembly_output.output += "setne %al\n";
    assembly_output.output += end_label_name + ":\n";

    store_register(assembly_output, "rax");
}
void generate_assignment_expression(assembly_output_t& assembly_output, const ast::binary_expression_t& assignment) {
    const auto var_name = validate_lvalue_expression_exp(assignment.left);
    if(!assembly_output.variable_lookup.contains_in_accessible_scopes(var_name)) {
        throw std::runtime_error("Variable " + var_name + " not declared in currently accessible scopes.");
    }

    generate_expression(assembly_output, assignment.right);

    pop_constant(assembly_output, "rax");
    store_variable(assembly_output, var_name, "rax");
    store_register(assembly_output, "rax"); // return variable so it can be used in expressions (e.g. assignment chaining)
}
void generate_unary_operation(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp, void(*func)(assembly_output_t&)) {
    generate_expression(assembly_output, unary_exp.exp);

    func(assembly_output);
}
