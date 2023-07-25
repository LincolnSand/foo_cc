#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <cstdint>
#include <iostream>
#include <optional>

#include <frontend/lexing/lexer.hpp>
#include <utils/common.hpp>


namespace ast {
enum class unary_op_t : std::uint16_t {
    NEG = 0,
    BITWISE_NOT,
    LOGIC_NOT,
};

// binary ops:
enum class times_divide_t : std::uint16_t {
    TIMES = 0,
    DIVIDE,
};
enum class plus_minus_t : std::uint16_t {
    PLUS = 0,
    MINUS,
};
enum class relational_t : std::uint16_t {
    LESS_THAN = 0,
    GREATER_THAN,
    LESS_THAN_EQUAL,
    GREATER_THAN_EQUAL,
};
enum class equality_t : std::uint16_t {
    EQUAL_EQUAL = 0,
    NOT_EQUAL,
};
enum class logical_and_t : std::uint16_t {
    AND = 0,
};
enum class logical_or_t : std::uint16_t {
    OR = 0,
};


using var_name_t = std::string;

struct constant_t {
    int value;
};


struct unary_op_expression_t;
struct grouping_t;
struct times_divide_binary_expression_t;
struct plus_minus_binary_expression_t;
struct relational_binary_expression_t;
struct equality_binary_expression_t;
struct logical_and_binary_expression_t;
struct logical_or_binary_expression_t;
struct assignment_t;
// `std::shared_ptr` is to get around not having full recursive type definitions in C++
using factor_t = std::variant<std::shared_ptr<grouping_t>, std::shared_ptr<unary_op_expression_t>, constant_t, var_name_t>;
using times_divide_expression_t = std::variant<std::shared_ptr<times_divide_binary_expression_t>, factor_t>; 
using plus_minus_expression_t = std::variant<std::shared_ptr<plus_minus_binary_expression_t>, times_divide_expression_t>;
using relational_expression_t = std::variant<std::shared_ptr<relational_binary_expression_t>, plus_minus_expression_t>;
using equality_expression_t = std::variant<std::shared_ptr<equality_binary_expression_t>, relational_expression_t>;
using logical_and_expression_t = std::variant<std::shared_ptr<logical_and_binary_expression_t>, equality_expression_t>;
using logical_or_expression_t = std::variant<std::shared_ptr<logical_or_binary_expression_t>, logical_and_expression_t>;
using expression_t = std::variant<std::shared_ptr<assignment_t>, logical_or_expression_t>;


struct unary_op_expression_t {
    unary_op_t op;
    factor_t expr;
};
struct grouping_t { // grouped with `( <expr> )`
    expression_t expr;
};


struct times_divide_binary_expression_t {
    times_divide_t op;
    times_divide_expression_t lhs;
    times_divide_expression_t rhs;
};
struct plus_minus_binary_expression_t {
    plus_minus_t op;
    plus_minus_expression_t lhs;
    plus_minus_expression_t rhs;
};
struct relational_binary_expression_t {
    relational_t op;
    relational_expression_t lhs;
    relational_expression_t rhs;
};
struct equality_binary_expression_t {
    equality_t op;
    equality_expression_t lhs;
    equality_expression_t rhs;
};
struct logical_and_binary_expression_t {
    logical_and_t op;
    logical_and_expression_t lhs;
    logical_and_expression_t rhs;
};
struct logical_or_binary_expression_t {
    logical_or_t op;
    logical_or_expression_t lhs;
    logical_or_expression_t rhs;
};

struct assignment_t {
    var_name_t var_name;
    expression_t expr;
};


struct return_statement_t {
    expression_t expr;
};
struct declaration_t {
    var_name_t var_name;
    std::optional<expression_t> value;
};

using statement_t = std::variant<return_statement_t, declaration_t, expression_t>;

struct function_declaration_t {
    std::string func_name;
    std::vector<statement_t> statements;
};

struct program_t {
    function_declaration_t function_declaration;
};
}


void print_variable_name(const ast::var_name_t& var_name);
void print_constant(const ast::constant_t& constant);
void print_unary_op(const ast::unary_op_expression_t& op);
void print_grouping(const ast::grouping_t& grouping);
void print_factor(const ast::factor_t& factor);
void print_times_divide_binary_expression(const ast::times_divide_binary_expression_t& expr);
void print_times_divide_expression(const ast::times_divide_expression_t& expr);
void print_plus_minus_binary_expression(const ast::plus_minus_binary_expression_t& expr);
void print_plus_minus_expression(const ast::plus_minus_expression_t& expr);
void print_relational_binary_expression(const ast::relational_binary_expression_t& expr);
void print_relational_expression(const ast::relational_expression_t& expr);
void print_equality_binary_expression(const ast::equality_binary_expression_t& expr);
void print_equality_expression(const ast::equality_expression_t& expr);
void print_logical_and_binary_expression(const ast::logical_and_binary_expression_t& expr);
void print_logical_and_expression(const ast::logical_and_expression_t& expr);
void print_logical_or_binary_expression(const ast::logical_or_binary_expression_t& expr);
void print_logical_or_expression(const ast::logical_or_expression_t& expr);
void print_assignment(const ast::assignment_t& assignment);
void print_expression(const ast::expression_t& expr);
void print_declaration(const ast::declaration_t& declaration);
void print_return_stmt(const ast::return_statement_t& return_stmt);
void print_stmt(const ast::statement_t& stmt);
void print_func_decl(const ast::function_declaration_t& func_decl);
void print_ast(const ast::program_t& program);
