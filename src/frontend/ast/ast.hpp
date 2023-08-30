#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <array>
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

constexpr std::size_t NUMBER_OF_TYPE_CATEGORIES = 5;
enum class type_category_t {
    INT, UNSIGNED_INT, DOUBLE,
    STRUCT,
    TYPEDEF,
};
using type_name_t = std::string;
struct type_t {
    type_category_t type_category;
    type_name_t type_name;

    // if `token_type` is `type_category_t::TYPEDEF`, this stores the name and category of the type being aliased. It is `std::nullopt` otherwise.
    std::optional<type_category_t> aliased_type_category;
    std::optional<type_name_t> aliased_type;

    std::optional<std::size_t> size; // is `std::nullopt` if we only have a forward declaration (size and alignment are filled in once the struct is fully defined)
    std::optional<std::size_t> alignment;

    std::unordered_map<std::string, std::size_t> field_offsets; // get the index in `fields` from field name
    std::vector<type_t> fields;
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
using type_table_t = std::array<std::unordered_map<type_name_t, type_t>, NUMBER_OF_TYPE_CATEGORIES>;
struct program_t {
    // Maintain separate symbol tables for type category so we can have both a `struct` and `typedef` with the same name without having to append "struct " to the key in the case of the former.
    // It also just generally will make your life easier for disambiguating identifiers later.
    type_table_t type_table;

    // symbol validation and all other non-syntax checking (e.g. number of params, multiple definition, etc.) will not be checked during parsing and are caught and validated during a later compiler pass.
    std::vector<std::variant<function_declaration_t, function_definition_t, global_variable_declaration_t>> top_level_declarations;
};

struct validated_global_variable_definition_t {
    type_t type_name;
    var_name_t var_name;
    constant_t value; // is set to `0` if the variable is declared, but not defined in `program_t`
};
struct validated_program_t {
    type_table_t type_table;

    std::vector<std::variant<function_definition_t, validated_global_variable_definition_t>> top_level_declarations; // guaranteed to be deduplicated
};
}



inline ast::expression_t make_convert_t(ast::expression_t&& expr, ast::type_t type) {
    return ast::expression_t{ std::make_shared<ast::convert_t>(ast::convert_t{std::move(expr)}), type};
}

// TODO: maybe I should make these functions actually add the types to the type table in addition to just constructing/returning them
// Does NOT add the type to the type_table; it is merely a convenience factory function that returns a constructed `type_t`.
inline ast::type_t make_primitive_type_t(ast::type_category_t type_category, ast::type_name_t type_name, std::size_t size, std::size_t alignment) {
    return ast::type_t{type_category, std::move(type_name), std::nullopt, std::nullopt, size, alignment, {}, {}};
}
inline ast::type_t make_typedef_type_t(const ast::type_table_t& type_table, ast::type_name_t type_name, ast::type_category_t aliased_type_category, ast::type_name_t aliased_type) {
    auto aliased_type_iter = type_table.at(static_cast<std::uint32_t>(aliased_type_category)).find(aliased_type);
    if(aliased_type_iter == std::end(type_table.at(static_cast<std::uint32_t>(aliased_type_category)))) {
        throw std::logic_error("Type being aliased does not exist.");
    }
    // Fyi, if you are type aliasing a forward declaration (or a type alias of a forward declaration) that is later defined, the size, alignment, and fields members will be updated on the first query after it has been defined.
    return ast::type_t{ast::type_category_t::TYPEDEF, std::move(type_name), aliased_type_category, aliased_type, aliased_type_iter->second.size, aliased_type_iter->second.alignment, aliased_type_iter->second.field_offsets, aliased_type_iter->second.fields};
}
inline ast::type_t make_struct_forward_decl_type_t(ast::type_name_t type_name) {
    return ast::type_t{ast::type_category_t::STRUCT, std::move(type_name), std::nullopt, std::nullopt, std::nullopt, std::nullopt, {}, {}};
}
inline ast::type_t make_struct_definition_type_t(const ast::type_table_t& type_table, ast::type_name_t type_name, std::vector<ast::type_t> field_types, std::vector<std::string> field_names) {
    if(field_types.size() != field_names.size()) {
        throw std::logic_error("Mismatch of number of field names and types.");
    }
    std::unordered_map<std::string, std::size_t> field_offsets;
    std::size_t struct_size{};
    std::size_t struct_alignment{};
    std::size_t prior_field_size{};
    for(std::size_t i = 0; i < std::size(field_types); ++i) {
        field_offsets.insert({std::move(field_names.at(i)), i});
        const ast::type_t& member_type = field_types.at(i);
        if(!member_type.size.has_value() || !member_type.size.has_value()) {
            // TODO: Implement checking and handling if `member_type` is a type alias of a struct forward declaration where the struct has been defined since the type alias was created.
            throw std::runtime_error("You cannot instantiate a struct forward declaration.");
        }
        struct_size += member_type.size.value();
        // TODO: maybe insert dummy padding members to make life easier later during assembly codegen
        struct_size += (prior_field_size % member_type.alignment.value()); // round up to abide by alignment requirements of fields
        prior_field_size = member_type.size.value();
        struct_alignment = std::max(struct_alignment, member_type.alignment.value());
    }
    if(struct_size == 0) {
        struct_size = 1;
    }
    if(struct_alignment == 0) {
        struct_alignment = 1;
    }
    return ast::type_t{ast::type_category_t::STRUCT, std::move(type_name), std::nullopt, std::nullopt, struct_size, struct_alignment, std::move(field_offsets), std::move(field_types)};
}


// defined in middle_end/typing/generate_typing.cpp:
bool compare_type_names(const ast::type_t& lhs, const ast::type_t& rhs);

// defined in frontend/parsing/parser.cpp:
ast::type_t create_type_name_from_token(const token_t& token);
bool has_return_statement(const ast::compound_statement_t& compound_stmt);

