#include "traverse_ast_helpers.hpp"


void generate_binary_operation(assembly_output_t& assembly_output, const ast::binary_expression_t& binary_exp, void(*func)(assembly_output_t&)) {
    generate_expression(assembly_output, binary_exp.left);
    generate_expression(assembly_output, binary_exp.right);

    func(assembly_output);
}
void generate_logical_and(assembly_output_t& assembly_output, const ast::binary_expression_t& binary_exp) {

}
void generate_logical_or(assembly_output_t& assembly_output, const ast::binary_expression_t& binary_exp) {

}
void generate_assignment_expression(assembly_output_t& assembly_output, const ast::binary_expression_t& assignment) {

}
void generate_unary_operation(assembly_output_t& assembly_output, const ast::unary_expression_t& unary_exp, void(*func)(assembly_output_t&)) {
    generate_expression(assembly_output, unary_exp.exp);

    func(assembly_output);
}
