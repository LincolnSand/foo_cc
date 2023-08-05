#pragma once


#include <stdexcept>
#include <string_view>
#include <string>
#include <utility>
#include <memory>

#include <frontend/lexing/lexer.hpp>
#include "ast.hpp"
#include "parser_utils.hpp"
#include <utils/common.hpp>

// debugging:
#include <cstdio>


struct parser_t {
    std::vector<token_t> tokens;
    std::uint32_t current_token_index = 0; // number of tokens processed so far, starts at `0`, terminates at `tokens.size()`.

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
};

ast::var_name_t parse_var_name(parser_t& parser);
ast::constant_t parse_constant(parser_t& parser);
std::shared_ptr<ast::grouping_t> parse_grouping(parser_t& parser);

ast::expression_t parse_expression(parser_t& parser, ast::precedence_t precedence);
ast::expression_t parse_expression(parser_t& parser);

ast::return_statement_t parse_return_statement(parser_t& parser);
ast::expression_t parse_expression_statement(parser_t& parser);
ast::if_statement_t parse_if_statement(parser_t& parser);
ast::statement_t parse_statement(parser_t& parser);

ast::declaration_t parse_declaration(parser_t& parser);
ast::compound_statement_t parse_compound_statement(parser_t& parser);
ast::function_declaration_t parse_function_decl(parser_t& parser);

ast::program_t parse(parser_t& parser);
