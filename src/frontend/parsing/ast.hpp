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
using var_name_t = std::string;
struct constant_t {
    int value;
};

struct grouping_t;
struct unary_expression_t;
struct binary_expression_t;
struct ternary_expression_t;
// `std::shared_ptr` is to get around not having full recursive type definitions in C++
using expression_t = std::variant<std::shared_ptr<grouping_t>, std::shared_ptr<unary_expression_t>, std::shared_ptr<binary_expression_t>, std::shared_ptr<ternary_expression_t>, constant_t, var_name_t>;

struct grouping_t { // grouped with `( <expr> )`
    expression_t expr;
};

using precedence_t = std::uint16_t;
enum class unary_operator_fixity_t : std::uint16_t {
    PREFIX = 0,
    POSTFIX,
};
enum class unary_operator_token_t : std::uint16_t {
    PLUS_PLUS = 0, MINUS_MINUS,
    PLUS, MINUS,
    LOGICAL_NOT, BITWISE_NOT,    
};
enum class binary_operator_token_t : std::uint16_t {
    MULTIPLY = 0, DIVIDE, MODULO,
    PLUS, MINUS,
    LEFT_BITSHIFT, RIGHT_BITSHIFT,
    LESS_THAN, LESS_THAN_EQUAL, GREATER_THAN, GREATER_THAN_EQUAL,
    EQUAL, NOT_EQUAL,
    BITWISE_AND, BITWISE_XOR, BITWISE_OR,
    LOGICAL_AND, LOGICAL_OR,
    ASSIGNMENT, // compound assignment operators are transformed by the parser into assignment expressions using the corresponding binary operator as the rhs.
    COMMA,
};

struct unary_expression_t {
    unary_operator_fixity_t fixity;
    unary_operator_token_t op;
    expression_t exp;
};
struct binary_expression_t {
    binary_operator_token_t op;
    expression_t left;
    expression_t right;
};
struct ternary_expression_t {
    expression_t condition;
    expression_t if_true;
    expression_t if_false;
};

struct return_statement_t {
    expression_t expr;
};
struct declaration_t {
    var_name_t var_name;
    std::optional<expression_t> value;
};

struct if_statement_t;
struct compound_statement_t;
using statement_t = std::variant<return_statement_t, expression_t, std::shared_ptr<if_statement_t>, std::shared_ptr<compound_statement_t>>;

struct compound_statement_t {
    std::vector<std::variant<statement_t, declaration_t>> stmts;
};
struct if_statement_t {
    expression_t if_exp;
    statement_t if_body;
    std::optional<statement_t> else_body;
};

struct function_declaration_t {
    std::string func_name;
    compound_statement_t statements;
};

struct program_t {
    std::vector<function_declaration_t> function_declarations;
    std::vector<declaration_t> declarations;
};
}


void print_expression(const ast::expression_t& expr);

void print_return_statement(const ast::return_statement_t& return_stmt);
void print_if_statement(const ast::if_statement_t& if_statement);
void print_statement(const ast::statement_t& stmt);

void print_declaration(const ast::declaration_t& declaration);
void print_compound_statement(const ast::compound_statement_t& declaration_statement);
void print_function_decl(const ast::function_declaration_t& func_decl);

void print_ast(const ast::program_t& program);
