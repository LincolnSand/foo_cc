#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <cstdint>
#include <iostream>
#include <optional>
#include <unordered_map>

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
struct function_call_t;
// `std::shared_ptr` is to get around not having full recursive type definitions in C++
// TODO: maybe use `std::unique_ptr` instead of `std::shared_ptr`
using expression_t = std::variant<std::shared_ptr<grouping_t>, std::shared_ptr<unary_expression_t>, std::shared_ptr<binary_expression_t>, std::shared_ptr<ternary_expression_t>, std::shared_ptr<function_call_t>, constant_t, var_name_t>;

struct grouping_t { // grouped with `( <expr> )`
    expression_t expr;
};
struct function_call_t {
    std::string function_name;
    std::vector<expression_t> params;
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
using type_name_t = token_t;
struct declaration_t {
    type_name_t type_name;
    var_name_t var_name;
    std::optional<expression_t> value;
};
struct expression_statement_t {
    std::optional<expression_t> expr;
};

struct if_statement_t;
struct compound_statement_t;
using statement_t = std::variant<return_statement_t, expression_statement_t, std::shared_ptr<if_statement_t>, std::shared_ptr<compound_statement_t>>;

struct compound_statement_t {
    std::vector<std::variant<statement_t, declaration_t>> stmts;
};
struct if_statement_t {
    expression_t if_exp;
    statement_t if_body;
    std::optional<statement_t> else_body;
};

using func_name_t = std::string;
struct function_declaration_t {
    type_name_t return_type;
    func_name_t function_name;
    std::vector<type_name_t> params;
};
struct function_definition_t {
    type_name_t return_type;
    func_name_t function_name;
    std::vector<std::pair<type_name_t, std::optional<var_name_t>>> params;
    compound_statement_t statements;
};

struct program_t {
    // symbol validation and all other non-syntax checking (e.g. number of params, multiple definition, etc.) will not be checked during parsing and are caught and validated during a later compiler pass.
    std::vector<function_declaration_t> function_declarations;
    std::vector<function_definition_t> function_definitions;
    std::vector<declaration_t> declarations;
};
}
