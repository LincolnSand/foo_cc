#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <cstdint>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <cstddef>
#include <cassert>

#include <frontend/lexing/lexer.hpp>
#include <utils/common.hpp>


namespace ast {
using var_name_t = std::string;
struct constant_t {
    // TODO: maybe use <cstdint> type aliases instead so our type sizes aren't host architecture dependent
    std::variant<char, signed char, unsigned char, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, long double> value;
};

enum class type_category_t {
    INT, UNSIGNED_INT, DOUBLE,
};
struct type_t {
    type_category_t token_type; // TODO: rename to `type_category` from `token_type`
    std::string type_name;
    std::size_t size;
    std::size_t alignment;
};

struct grouping_t;
struct convert_t;
struct unary_expression_t;
struct binary_expression_t;
struct ternary_expression_t;
struct function_call_t;
// `std::shared_ptr` is to get around not having full recursive type definitions in C++
// TODO: maybe use `std::unique_ptr` instead of `std::shared_ptr`
struct expression_t {
    std::variant<std::shared_ptr<grouping_t>, std::shared_ptr<convert_t>, std::shared_ptr<unary_expression_t>, std::shared_ptr<binary_expression_t>, std::shared_ptr<ternary_expression_t>, std::shared_ptr<function_call_t>, constant_t, var_name_t> expr;
    std::optional<type_t> type; // usually has `std::nullopt` if not a literal after initial parsing stage. Is filled in during semantic analysis and type checking pass (as we often need symbol tables).
};

struct grouping_t { // grouped with `( <expr> )`
    expression_t expr;
};
struct convert_t { // used for type conversions and casts
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
struct declaration_t {
    type_t type_name;
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
    type_t return_type;
    func_name_t function_name;
    std::vector<type_t> params;
};
struct function_definition_t {
    type_t return_type;
    func_name_t function_name;
    std::vector<std::pair<type_t, std::optional<var_name_t>>> params;
    compound_statement_t statements;
};

using global_variable_declaration_t = declaration_t;
struct program_t {
    // symbol validation and all other non-syntax checking (e.g. number of params, multiple definition, etc.) will not be checked during parsing and are caught and validated during a later compiler pass.
    std::vector<std::variant<function_declaration_t, function_definition_t, global_variable_declaration_t>> top_level_declarations;
};

struct validated_global_variable_definition_t {
    type_t type_name;
    var_name_t var_name;
    constant_t value; // is set to `0` if the variable is declared, but not defined in `program_t`
};
struct validated_program_t {
    std::vector<std::variant<function_definition_t, validated_global_variable_definition_t>> top_level_declarations; // guaranteed to be deduplicated
};
}



inline ast::expression_t make_convert_t(ast::expression_t&& expr, ast::type_t type) {
    return ast::expression_t{ std::make_shared<ast::convert_t>(ast::convert_t{std::move(expr)}), type};
}


// defined in middle_end/typing/generate_typing.cpp:
bool compare_type_names(const ast::type_t& lhs, const ast::type_t& rhs);

// defined in frontend/parsing/parser.cpp:
ast::type_t create_type_name_from_token(const token_t& token);
bool has_return_statement(const ast::compound_statement_t& compound_stmt);

