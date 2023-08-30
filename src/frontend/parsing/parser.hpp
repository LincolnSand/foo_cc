#pragma once


#include <stdexcept>
#include <string_view>
#include <string>
#include <unordered_map>
#include <utility>
#include <memory>
#include <cstddef>

#include <frontend/lexing/lexer.hpp>
#include <frontend/ast/ast.hpp>
#include <utils/data_structures/random_access_stack.hpp>
#include <backend/interpreter/compile_time_evaluator.hpp>
#include "parser_utils.hpp"
#include <utils/common.hpp>


struct validation_t {
    utils::data_structures::validation_variable_lookup_t variable_lookup;

    std::unordered_map<ast::func_name_t, ast::function_declaration_t> function_declarations_lookup;
    std::unordered_map<ast::func_name_t, ast::function_definition_t> function_definitions_lookup;

    std::unordered_map<ast::var_name_t, ast::global_variable_declaration_t> global_variable_declarations;
    std::unordered_map<ast::var_name_t, ast::global_variable_declaration_t> global_variable_definitions;

    ast::type_table_t type_table;
};

struct parser_t {
    std::vector<token_t> tokens;
    std::uint32_t current_token_index = 0; // number of tokens processed so far, starts at `0`, terminates at `tokens.size()`.

    validation_t symbol_info;

    parser_t() = delete;
    parser_t(std::vector<token_t> tokens) : tokens(std::move(tokens)) {}

    bool is_eof() const {
        return current_token_index >= tokens.size();
    }
    // `is_eof()` == `is_eof_n(0)`
    bool is_eof_n(const std::uint32_t lookahead) const {
        return (current_token_index + lookahead) >= tokens.size();
    }

    token_t peek_token() const {
        return tokens.at(current_token_index);
    }
    // `peek_token()` == `peek_token(0)`
    token_t peek_token_n(const std::uint32_t lookahead) const {
        if(is_eof_n(lookahead)) {
            throw std::logic_error("Invalid lookahead. Out of bounds of parser tokens.");
        }
        return tokens.at(current_token_index + lookahead);
    }

    bool is_eof_back_n(const std::uint32_t lookbehind) const {
        const std::int64_t new_index = static_cast<std::int64_t>(current_token_index) - static_cast<std::int64_t>(lookbehind);
        return new_index < 0;
    }
    // `is_eof_back()` == `is_eof_back_n(1)`
    bool is_eof_back() const {
        return is_eof_back_n(1);
    }

    // `peek_back()` == `peek_back_n(1)`
    token_t peek_back_n(const std::uint32_t lookbehind) const {
        const std::int64_t new_index = static_cast<std::int64_t>(current_token_index) - static_cast<std::int64_t>(lookbehind);
        if(is_eof_back_n(lookbehind)) {
            throw std::logic_error("Invalid lookbehind. Out of bounds of parser tokens.");
        }
        return tokens.at(new_index);
    }
    token_t peek_back() const {
        return peek_back_n(1);
    }

    token_t advance_token() {
        return tokens.at(current_token_index++);
    }
    // `advance_token()` == `advance_token_n(1)`
    token_t advance_token_n(const std::uint32_t lookahead) {
        return tokens.at((current_token_index += lookahead) - 1);
    }

    void expect_token(const token_type_t expected, const char *const error_message) {
        const auto actual_token = advance_token();
        if(actual_token.token_type != expected) {
            std::cout << "Found token: " << static_cast<std::uint32_t>(actual_token.token_type) << ": " << actual_token.token_text << std::endl;
            throw std::runtime_error(error_message);
        }
    }
};


void validate_type_name(const ast::type_t& expected, const ast::type_t& actual, const std::string& error_message);

void validate_variable(const validation_t& validation, const ast::var_name_t& var_name);
void validate_variable_declaration(validation_t& validation, ast::declaration_t& declaration);
void validate_function_declaration(validation_t& validation, const ast::function_declaration_t& function_declaration);
void validate_expression(validation_t& validation, ast::expression_t& expression);
void validate_statement(validation_t& validation, ast::statement_t& statement);
void validate_compound_statement(validation_t& validation, ast::compound_statement_t& compound_statement, bool is_function_block = false);
void validate_function_definition(validation_t& validation, const ast::function_definition_t& function_definition);
void validate_function_call(validation_t& validation, const ast::function_call_t& function_call);
void validate_compile_time_expression(validation_t& validation, const ast::expression_t& expression);
void validate_global_variable_declaration(validation_t& validation, const ast::global_variable_declaration_t& declaration);
ast::validated_program_t validate_ast(const ast::program_t& program);


// defined in middle_end/typing/generate_typing.cpp:

// `exp_type` is an outparam
void add_type_to_function_call(const validation_t& validation, const ast::function_call_t& expr, std::optional<ast::type_t>& exp_type);
void add_type_to_variable(const validation_t& validation, const ast::var_name_t& expr, std::optional<ast::type_t>& exp_type);



ast::var_name_t parse_var_name(parser_t& parser);
ast::expression_t parse_int_constant(parser_t& parser);
std::shared_ptr<ast::grouping_t> parse_grouping(parser_t& parser);

ast::expression_t parse_expression(parser_t& parser, ast::precedence_t precedence);
ast::expression_t parse_expression(parser_t& parser);

ast::return_statement_t parse_return_statement(parser_t& parser);
ast::expression_statement_t parse_expression_statement(parser_t& parser);
ast::if_statement_t parse_if_statement(parser_t& parser);
ast::statement_t parse_statement(parser_t& parser);

ast::declaration_t parse_declaration(parser_t& parser);
ast::compound_statement_t parse_compound_statement(parser_t& parser);
std::variant<ast::function_declaration_t, ast::function_definition_t, ast::declaration_t> parse_top_level_declaration(parser_t& parser);

ast::program_t parse(parser_t& parser);
