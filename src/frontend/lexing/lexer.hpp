#pragma once

#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>
#include <stdexcept>

#include <utils/common.hpp>

// debugging:
#include <iostream>


enum class token_type_t : std::uint32_t {
    // single character lexemes:
    LEFT_PAREN = 0, RIGHT_PAREN,
    LEFT_CURLY, RIGHT_CURLY,
    SEMICOLON,
    DASH, // for both unary negation and binary subtraction
    TILDE,
    BANG,
    PLUS, ASTERISK, SLASH,
    LESS_THAN, GREATER_THAN,
    EQUALS,

    // TODO: currently unimplemented in parser, but implemented in lexer
    MODULO,
    BITWISE_AND, BITWISE_OR, BITWISE_XOR,
    BITWISE_LEFT_SHIFT, BITWISE_RIGHT_SHIFT,
    COMMA,

    // two character lexemes:
    LOGIC_AND, LOGIC_OR,
    EQUAL_EQUAL, NOT_EQUAL,
    LESS_THAN_EQUAL, GREATER_THAN_EQUAL,

    // TODO: currently unimplemented in parser, but implemented in lexer
    PLUS_EQUALS, MINUS_EQUALS, TIMES_EQUALS, DIVIDE_EQUALS,
    MODULO_EQUALS,
    AND_EQUALS, OR_EQUALS, XOR_EQUALS,
    PLUS_PLUS, DASH_DASH,

    // three character lexemes:
    // TODO: currently unimplemented in parser, but implemented in lexer
    LEFT_SHIFT_EQUALS, RIGHT_SHIFT_EQUALS,

    // constants:
    INT_CONSTANT,

    IDENTIFIER,
    // reserved:
    INT_KEYWORD,
    RETURN_KEYWORD,

    // special:
    // have to use `EOF_TOK` since `EOF` is a macro in C/C++
    EOF_TOK,
    ERROR = 255,
};

struct token_t {
    token_type_t token_type;
    std::string_view token_text;
    utils::line_number_t line_number;


    token_t() = delete;

    token_t(token_type_t token_type, std::string_view token_text, utils::line_number_t line_number) :
        token_type(token_type),
        token_text(token_text),
        line_number(line_number)
    {}
};

struct lexer_t {
    const char* start; // start character of current token being lexed
    const char* current; // current character being lexed of the current token being lexed
    utils::line_number_t current_line_number;


    lexer_t() = delete;

    // `text` is the text to be lexed
    lexer_t(const char *const text) :
        start(text),
        current(text),
        current_line_number(1)
    {}

    bool is_eof() const {
        return *current == '\0';
    }
    // `is_eof()` == `is_eof_n(0)`
    bool is_eof_n(const std::uint32_t n) const {
        for(std::uint32_t i = 0; i < n; ++i) {
            if (current[i] == '\0') return true;
        }
        return false;
    }

    char peek_char() const {
        return *current;
    }
    // `peek_char()` == `peek_char_n(0)`
    char peek_char_n(const std::uint32_t lookahead) const {
        if(is_eof_n(lookahead)) return '\0';

        return current[lookahead];
    }

    char advance_char() {
        return *(current++);
    }
    // `advance_char()` == `advance_char_n(1)`
    void advance_char_n(const std::uint32_t n) {
        current += n;
    }

    std::uint32_t current_token_str_len() const {
        return current - start;
    };

    bool match_char(const char expected) {
        if(is_eof()) return false;

        if(peek_char() != expected) return false;

        advance_char();
        return true;
    }

    void incr_line_number() {
        ++current_line_number;
    }


    // factory to use once token string is lexed
    token_t make_token(token_type_t token_type) const {
        return token_t{token_type, {start, current_token_str_len()}, current_line_number};
    }
};


token_type_t handle_number(lexer_t& lexer);
token_type_t handle_identifier(lexer_t& lexer);
token_type_t handle_keywords(lexer_t& lexer);
void handle_whitespace(lexer_t& lexer);
// return `true` if comment is lexed, `false` otherwise
bool handle_comment(lexer_t& lexer);

token_t scan_token(lexer_t& lexer);
std::vector<token_t> scan_all_tokens(lexer_t& lexer);
