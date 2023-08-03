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
// `std::shared_ptr` is to get around not having full recursive type definitions in C++
using expression_t = std::variant<std::shared_ptr<grouping_t>, std::shared_ptr<binary_expression_t>, std::shared_ptr<unary_expression_t>, constant_t, var_name_t>;

struct grouping_t { // grouped with `( <expr> )`
    expression_t expr;
};

enum class associativity_t : std::uint16_t {
    LEFT_TO_RIGHT = 0,
    RIGHT_TO_LEFT,
};
using precedence_t = std::uint16_t; // C has 15 precedence levels
//constexpr precedence_t MAX_PRECEDENCE = 14; // [0 - 14]
enum class unary_operator_fixity_t : std::uint16_t {
    PREFIX = 0,
    POSTFIX,
};
enum class binary_operator_fixity_t : std::uint16_t {
    PREFIX = 0,
    POSTFIX,
    INFIX,
    MIDFIX, // e.g. ternary
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
    TERNARY,
    ASSIGNMENT, // compound assignment operators are transformed by the parser into assignment expressions using the corresponding binary operator as the rhs.
    COMMA,
};

struct unary_expression_t {
    //precedence_t precedence_level;
    unary_operator_fixity_t fixity;
    unary_operator_token_t op;
    expression_t exp;
};
struct binary_expression_t {
    //associativity_t associativity;
    //precedence_t precedence_level;
    //binary_operator_fixity_t fixity;
    binary_operator_token_t op;
    expression_t left;
    expression_t right;
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


void print_expression(const ast::expression_t& expr);

void print_declaration(const ast::declaration_t& declaration);
void print_return_stmt(const ast::return_statement_t& return_stmt);
void print_stmt(const ast::statement_t& stmt);
void print_func_decl(const ast::function_declaration_t& func_decl);
void print_ast(const ast::program_t& program);
