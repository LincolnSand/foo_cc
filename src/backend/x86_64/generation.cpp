#include "generation.hpp"


// trashes `rcx`
// TODO: stop this from trashing `rcx`
void store_constant(assembly_output_t& assembly_output, const ast::constant_t& constant) {
    assembly_output += "movq $" + std::to_string(constant.value) + ", %rcx\n";
    assembly_output += "pushq %rcx\n";
}

void store_register(assembly_output_t& assembly_output, const std::string& register_name) {
    assembly_output += "pushq %" + register_name + "\n";
}

void pop_constant(assembly_output_t& assembly_output, const std::string& register_name) {
    assembly_output += "popq %" + register_name + "\n";
}

void generate_negation(assembly_output_t& assembly_output) {
    assembly_output += "neg %rax\n";
}

void generate_bitwise_not(assembly_output_t& assembly_output) {
    assembly_output += "not %rax\n";
}

void generate_logic_not(assembly_output_t& assembly_output) {
    assembly_output += "cmpq $0, %rax\n";
    assembly_output += "movq $0, %rax\n";
    assembly_output += "sete %al\n";
}

void generate_unary_op(assembly_output_t& assembly_output, const ast::unop_t& expression) {
    generate_factor(assembly_output, expression.expr);

    pop_constant(assembly_output, "rax");

    switch(expression.op) {
        case ast::unary_op_t::NEG:
            generate_negation(assembly_output);
            break;
        case ast::unary_op_t::BITWISE_NOT:
            generate_bitwise_not(assembly_output);
            break;
        case ast::unary_op_t::LOGIC_NOT:
            generate_logic_not(assembly_output);
            break;
    }

    store_register(assembly_output, "rax");
}

void generate_grouping(assembly_output_t& assembly_output, const ast::grouping_t& grouping) {
    generate_expression(assembly_output, grouping.expr);
}

void generate_factor(assembly_output_t& assembly_output, const ast::factor_t& factor) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::grouping_t>& factor) {
            generate_grouping(assembly_output, *factor);
        },
        [&assembly_output](const std::shared_ptr<ast::unop_t>& unary_exp) {
            generate_unary_op(assembly_output, *unary_exp);
        },
        [&assembly_output](const ast::constant_t& constant) {
            store_constant(assembly_output, constant);
        }
    }, factor);
}

void generate_multiply(assembly_output_t& assembly_output, const std::string& register_name) {
    assembly_output += "imulq %" + register_name + ", %rax\n";
}

void generate_divide(assembly_output_t& assembly_output, const std::string& register_name) {
    assembly_output += "cqo\n"; // trash `rdx` by sign extending into `rdx`. Needed for `idiv` dividend.
    assembly_output += "idivq %" + register_name + "\n";
}

void generate_times_divide_expr(assembly_output_t& assembly_output, const ast::times_divide_expression_t& expression) {
    generate_term(assembly_output, expression.lhs);
    generate_term(assembly_output, expression.rhs);
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");
    switch(expression.op) {
        case ast::times_divide_t::TIMES:
            generate_multiply(assembly_output, "rcx"); // first parameter is implicitly `rax` and trashes `rdx`
            break;
        case ast::times_divide_t::DIVIDE:
            generate_divide(assembly_output, "rcx"); // divisor is implicitly `rax` and trashes `rdx`
            break;
    }
    store_register(assembly_output, "rax");
}

void generate_add(assembly_output_t& assembly_output, const std::string& register_name_dest, const std::string& register_name_src) {
    assembly_output += "addq %" + register_name_src + ", %" + register_name_dest + "\n"; 
}

void generate_sub(assembly_output_t& assembly_output, const std::string& register_name_dest, const std::string& register_name_src) {
    assembly_output += "subq %" + register_name_src + ", %" + register_name_dest + "\n";
}

void generate_plus_minus_expr(assembly_output_t& assembly_output, const ast::plus_minus_expression_t& expression) {
    generate_expression(assembly_output, expression.lhs);
    generate_expression(assembly_output, expression.rhs);
    pop_constant(assembly_output, "rcx");
    pop_constant(assembly_output, "rax");
    switch(expression.op) {
        case ast::plus_minus_t::PLUS:
            generate_add(assembly_output, "rax", "rcx");
            break;
        case ast::plus_minus_t::MINUS:
            generate_sub(assembly_output, "rax", "rcx");
            break;
    }
    store_register(assembly_output, "rax");
}

void generate_term(assembly_output_t& assembly_output, const ast::term_t& term) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::times_divide_expression_t>& term) {
            generate_times_divide_expr(assembly_output, *term);
        },
        [&assembly_output](const ast::factor_t& factor) {
            generate_factor(assembly_output, factor);
        }
    }, term);
}

void generate_expression(assembly_output_t& assembly_output, const ast::expression_t& expression) {
    std::visit(overloaded{
        [&assembly_output](const std::shared_ptr<ast::plus_minus_expression_t>& expr) {
            generate_plus_minus_expr(assembly_output, *expr);
        },
        [&assembly_output](const ast::term_t& term) {
            generate_term(assembly_output, term);
        }
    }, expression);
}

// TODO: this currently *only* works for `return` statements
void generate_return_statement(assembly_output_t& assembly_output, const ast::return_statement_t& statement) {
    generate_expression(assembly_output, statement.return_stmt);
    pop_constant(assembly_output, "rax");
    assembly_output += "ret\n";
}

void generate_function_decl(assembly_output_t& assembly_output, const ast::function_declaration_t& function_declaration) {
    assembly_output += ".globl ";
    assembly_output += function_declaration.name;
    assembly_output += "\n";
    assembly_output += function_declaration.name;
    assembly_output += ":\n";

    generate_return_statement(assembly_output, function_declaration.statement);
}

assembly_output_t generate_asm(const ast::program_t& program) {
    assembly_output_t output;

    generate_function_decl(output, program.function_declaration);

    return output;
}



