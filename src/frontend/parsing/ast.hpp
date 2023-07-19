#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <cstdint>
#include <iostream>

#include <frontend/lexing/lexer.hpp>
#include <utils/common.hpp>


namespace ast {
enum class unary_op_t : std::uint16_t {
    NEG = 0,
    BITWISE_NOT,
    LOGIC_NOT,
};

// binary ops:
enum class plus_minus_t : std::uint16_t {
    PLUS = 0,
    MINUS,
};
enum class times_divide_t : std::uint16_t {
    TIMES = 0,
    DIVIDE,
};


struct constant_t {
    int value;
};

struct unop_t;
struct grouping_t;
struct plus_minus_expression_t;
struct times_divide_expression_t;
// `std::shared_ptr` is to get around not having full recursive type definitions in C++
using factor_t = std::variant<std::shared_ptr<grouping_t>, std::shared_ptr<unop_t>, constant_t>;
using term_t = std::variant<std::shared_ptr<times_divide_expression_t>, factor_t>; 
using expression_t = std::variant<std::shared_ptr<plus_minus_expression_t>, term_t>;

struct plus_minus_expression_t {
    plus_minus_t op;
    expression_t lhs;
    expression_t rhs;
};

struct times_divide_expression_t {
    times_divide_t op;
    term_t lhs;
    term_t rhs;
};

struct grouping_t { // grouped with `( <expr> )`
    expression_t expr;
};

struct unop_t {
    unary_op_t op;
    factor_t expr;
};

struct return_statement_t {
    expression_t return_stmt;
};

struct function_declaration_t {
    std::string name;
    return_statement_t statement;
};

struct program_t {
    function_declaration_t function_declaration;
};
}

inline std::shared_ptr<ast::unop_t> make_unop(const ast::unary_op_t op, ast::factor_t& factor) {
    return std::make_shared<ast::unop_t>(ast::unop_t { op, std::move(factor) });
}
inline std::shared_ptr<ast::grouping_t> make_grouping(ast::expression_t& expression) {
    return std::make_shared<ast::grouping_t>( ast::grouping_t {std::move(expression)} );
}


void print_constant(const ast::constant_t& constant);
void print_unary_op(const ast::unary_op_t& op);
void print_grouping(const ast::grouping_t& grouping);
void print_factor(const ast::factor_t& factor);
void print_times_divide_expr(const ast::times_divide_expression_t& expr);
void print_term(const ast::term_t& term);
void print_plus_minus_expr(const ast::plus_minus_expression_t& expr);
void print_expr(const ast::expression_t& expr);
void print_return_stmt(const ast::return_statement_t& return_stmt);
void print_func_decl(const ast::function_declaration_t& func_decl);
void print_ast(const ast::program_t& program);
