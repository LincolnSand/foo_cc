#pragma once


#include <stdexcept>
#include <utility>
#include <memory>

#include <frontend/lexing/lexer.hpp>
#include <frontend/ast/ast.hpp>
#include <utils/data_structures/random_access_stack.hpp>
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
        if(is_eof()) {
            return token_t{token_type_t::EOF_TOK, "", 0}; // TODO: Put in an actual line number instead of just `0`
        }
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
        if(is_eof()) {
            return token_t{token_type_t::EOF_TOK, "", 0}; // TODO: Put in an actual line number instead of just `0`
        }
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


bool is_constant(token_t token);
inline bool is_var_name(token_t token) {
    return token.token_type == token_type_t::IDENTIFIER;
}
inline bool is_struct_keyword(token_t token) {
    return token.token_type == token_type_t::STRUCT_KEYWORD;
}
inline bool is_typedef_keyword(token_t token) {
    return token.token_type == token_type_t::TYPEDEF_KEYWORD;
}

ast::var_name_t validate_lvalue_expression_exp(const ast::expression_t& expr);
