#include "lexer.hpp"


token_type_t handle_integer_literal_suffix(lexer_t& lexer) {
    if(utils::match_caseless_char(lexer.peek_char(), 'U')) {
        lexer.advance_char();
        if(utils::match_caseless_char(lexer.peek_char(), 'L')) {
            lexer.advance_char();
            if(utils::match_caseless_char(lexer.peek_char(), 'L')) {
                lexer.advance_char();
                return token_type_t::UNSIGNED_LONG_LONG_CONSTANT;
            }
            return token_type_t::UNSIGNED_LONG_CONSTANT;
        }
        return token_type_t::UNSIGNED_INT_CONSTANT;
    }
    if(utils::match_caseless_char(lexer.peek_char(), 'L')) {
        lexer.advance_char();
        if(utils::match_caseless_char(lexer.peek_char(), 'U')) {
            lexer.advance_char();
            return token_type_t::UNSIGNED_LONG_CONSTANT;
        }
        if(utils::match_caseless_char(lexer.peek_char(), 'L')) {
            lexer.advance_char();
            if(utils::match_caseless_char(lexer.peek_char(), 'U')) {
                lexer.advance_char();
                return token_type_t::UNSIGNED_LONG_LONG_CONSTANT;
            }
            return token_type_t::LONG_LONG_CONSTANT;
        }
        return token_type_t::LONG_CONSTANT;
    }
    return token_type_t::INT_CONSTANT;
}
token_type_t handle_floating_literal_suffix(lexer_t& lexer) {
    if(utils::match_caseless_char(lexer.peek_char(), 'F')) {
        lexer.advance_char();
        return token_type_t::FLOAT_CONSTANT;
    }
    if(utils::match_caseless_char(lexer.peek_char(), 'L')) {
        lexer.advance_char();
        return token_type_t::LONG_DOUBLE_CONSTANT;
    }
    return token_type_t::DOUBLE_CONSTANT;
}
token_type_t handle_number(lexer_t& lexer) {
    while(utils::is_digit(lexer.peek_char())) {
        lexer.advance_char();
    }
    if(lexer.peek_char() != '.') {
        return handle_integer_literal_suffix(lexer);
    }
    lexer.advance_char();
    while(utils::is_digit(lexer.peek_char())) {
        lexer.advance_char();
    }
    return handle_floating_literal_suffix(lexer);
}
token_type_t handle_identifier(lexer_t& lexer) {
    while(utils::is_alpha_num(lexer.peek_char())) {
        lexer.advance_char();
    }
    return handle_keywords(lexer);
}
token_type_t handle_char(lexer_t& lexer) {
    // first quote is already consumed by caller
    lexer.advance_char();
    if(lexer.advance_char() != '\'') {
        std::cout << "Missing second quote for char\n";
        return token_type_t::ERROR;
    }
    return token_type_t::CHAR_CONSTANT;
}

static token_type_t match_keyword(const lexer_t& lexer, std::uint32_t start_index, std::string_view expected, const token_type_t expected_type) {
    if(lexer.current_token_str_len() == (start_index + expected.length())
    && std::memcmp(expected.data(), lexer.start + start_index, expected.length()) == 0) {
        return expected_type;
    }
    return token_type_t::IDENTIFIER;
}

token_type_t handle_keywords(lexer_t& lexer) {
    if(lexer.current_token_str_len() == 0) {
        std::cout << "Keyword has string length of 0\n";
        return token_type_t::ERROR;
    }

    switch(lexer.start[0]) {
        case 'c':
            return match_keyword(lexer, 1, "har", token_type_t::CHAR_KEYWORD);
        case 'd':
            return match_keyword(lexer, 1, "ouble", token_type_t::DOUBLE_KEYWORD);
        case 'e':
            return match_keyword(lexer, 1, "lse", token_type_t::ELSE_KEYWORD);
        case 'f':
            return match_keyword(lexer, 1, "loat", token_type_t::FLOAT_KEYWORD);
        case 'i':
            if(lexer.current_token_str_len() > 1) {
                switch(lexer.start[1]) {
                    case 'f':
                        return match_keyword(lexer, 2, "", token_type_t::IF_KEYWORD);
                    case 'n':
                        return match_keyword(lexer, 2, "t", token_type_t::INT_KEYWORD);
                }
            }
            break;
        case 'l':
            return match_keyword(lexer, 1, "ong", token_type_t::LONG_KEYWORD);
        case 'r':
            return match_keyword(lexer, 1, "eturn", token_type_t::RETURN_KEYWORD);
        case 's':
            if(lexer.current_token_str_len() > 1) {
                switch(lexer.start[1]) {
                    case 'h':
                        return match_keyword(lexer, 2, "ort", token_type_t::SHORT_KEYWORD);
                    case 'i':
                        return match_keyword(lexer, 2, "gned", token_type_t::SIGNED_KEYWORD);
                }
            }
            break;
        case 'u':
            return match_keyword(lexer, 1, "nsigned", token_type_t::UNSIGNED_KEYWORD);
    }

    return token_type_t::IDENTIFIER;
}
void handle_whitespace(lexer_t& lexer) {
    for(;;) {
        switch(lexer.peek_char()) {
            case '\n':
                lexer.incr_line_number();
                [[fallthrough]];
            case '\r':
            case '\t':
            case ' ':
                lexer.advance_char();
                break;
            default:
                return;
        }
    }
}

// this function assumes the opening (/*) of the comment has already been consumed. This consumes the comment text itself and the closing of it
static void handle_multiline_comment(lexer_t& lexer) {
    for(;;) {
        while(lexer.peek_char() != '*' && !lexer.is_eof()) {
            if(lexer.peek_char() == '\n') {
                lexer.incr_line_number();
            }
            lexer.advance_char();
        }

        if(lexer.is_eof()) throw std::runtime_error("Unterminated comment");
        lexer.advance_char(); // consume `*`
        if(lexer.is_eof()) throw std::runtime_error("Unterminated comment");
        if(lexer.peek_char() == '/') {
            lexer.advance_char(); // consume `/`
            return;
        }

        // otherwise, we ran into `*`, but not a following `/` and still have text to go, so the comment hasn't ended yet, so keep going
    }
}
// this function assumes the opening (//) of the comment has already been consumed. This consumes the comment text itself and the closing of it
static void handle_single_line_comment(lexer_t& lexer) {
    while(lexer.peek_char() != '\n' && !lexer.is_eof()) {
        lexer.advance_char();
    }

    if(lexer.is_eof()) return;

    // for `\n`:
    lexer.advance_char();
    lexer.incr_line_number();
}
// TODO: clean up this function
bool handle_comment(lexer_t& lexer) {
    for(;;) {
        switch(lexer.peek_char()) {
            case '/':
                if(lexer.peek_char_n(1) == '*') {
                    lexer.advance_char(); // skip over `/` in string
                    lexer.advance_char(); // skip over `*` in string
                    handle_multiline_comment(lexer);
                    return true;
                } else if(lexer.peek_char_n(1) == '/') {
                    lexer.advance_char(); // skip over first `/` in string
                    lexer.advance_char(); // skip over second `/` in string
                    handle_single_line_comment(lexer);
                    return true;
                }
                // no characters consumed, leave comment lexer
                [[fallthrough]];
            default:
                return false;
        }
    }
}

token_t scan_token(lexer_t& lexer) {
    lexer.start = lexer.current; // restart token string for next token

    try {
        do {
            handle_whitespace(lexer);
        } while(handle_comment(lexer));
    }
    catch(std::runtime_error &e) {
        std::cout << "Unterminated comment\n";
        return lexer.make_token(token_type_t::ERROR); // unterminated multiline comment at eof
    }

    lexer.start = lexer.current; // skip over the lexed whitespace and comments


    if(lexer.is_eof()) {
        return lexer.make_token(token_type_t::EOF_TOK);
    }

    const char c = lexer.advance_char();

    if(utils::is_digit(c)) {
        return lexer.make_token(handle_number(lexer));
    }
    if(utils::is_alpha(c)) { // identifier cannot start with number
        return lexer.make_token(handle_identifier(lexer));
    }
    if(c == '\'') {
        return lexer.make_token(handle_char(lexer));
    }

    switch(c) {
        case '(':
            return lexer.make_token(token_type_t::LEFT_PAREN);
        case ')':
            return lexer.make_token(token_type_t::RIGHT_PAREN);
        case '{':
            return lexer.make_token(token_type_t::LEFT_CURLY);
        case '}':
            return lexer.make_token(token_type_t::RIGHT_CURLY);
        case ';':
            return lexer.make_token(token_type_t::SEMICOLON);
        case '-':
            if(lexer.match_char('=')) {
                return lexer.make_token(token_type_t::MINUS_EQUALS);
            } else if(lexer.match_char('-')) {
                return lexer.make_token(token_type_t::DASH_DASH);
            }
            return lexer.make_token(token_type_t::DASH);
        case '~':
            return lexer.make_token(token_type_t::TILDE);
        case '!':
            if(lexer.match_char('=')) {
                return lexer.make_token(token_type_t::NOT_EQUAL);
            }
            return lexer.make_token(token_type_t::BANG);
        case '+':
            if(lexer.match_char('=')) {
                return lexer.make_token(token_type_t::PLUS_EQUALS);
            } else if(lexer.match_char('+')) {
                return lexer.make_token(token_type_t::PLUS_PLUS);
            }
            return lexer.make_token(token_type_t::PLUS);
        case '*':
            if(lexer.match_char('=')) {
                return lexer.make_token(token_type_t::TIMES_EQUALS);
            }
            return lexer.make_token(token_type_t::ASTERISK);
        case '/':
            if(lexer.match_char('=')) {
                return lexer.make_token(token_type_t::DIVIDE_EQUALS);
            }
            return lexer.make_token(token_type_t::SLASH);
        case '&':
            if(lexer.match_char('&')) {
                return lexer.make_token(token_type_t::LOGIC_AND);
            } else if(lexer.match_char('=')) {
                return lexer.make_token(token_type_t::AND_EQUALS);
            }
            return lexer.make_token(token_type_t::BITWISE_AND);
        case '|':
            if(lexer.match_char('|')) {
                return lexer.make_token(token_type_t::LOGIC_OR);
            } else if(lexer.match_char('=')) {
                return lexer.make_token(token_type_t::OR_EQUALS);
            }
            return lexer.make_token(token_type_t::BITWISE_OR);
        case '<':
            if(lexer.match_char('=')) {
                return lexer.make_token(token_type_t::LESS_THAN_EQUAL);
            } else if(lexer.match_char('<')) {
                if(lexer.match_char('=')) {
                    return lexer.make_token(token_type_t::LEFT_SHIFT_EQUALS);
                }
                return lexer.make_token(token_type_t::BITWISE_LEFT_SHIFT);
            }
            return lexer.make_token(token_type_t::LESS_THAN);
        case '>':
            if(lexer.match_char('=')) {
                return lexer.make_token(token_type_t::GREATER_THAN_EQUAL);
            } else if(lexer.match_char('>')) {
                if(lexer.match_char('=')) {
                    return lexer.make_token(token_type_t::RIGHT_SHIFT_EQUALS);
                }
                return lexer.make_token(token_type_t::BITWISE_RIGHT_SHIFT);
            }
            return lexer.make_token(token_type_t::GREATER_THAN);
        case '=':
            if(lexer.match_char('=')) {
                return lexer.make_token(token_type_t::EQUAL_EQUAL);
            }
            return lexer.make_token(token_type_t::EQUALS);
        case '%':
            if(lexer.match_char('=')) {
                return lexer.make_token(token_type_t::MODULO_EQUALS);
            }
            return lexer.make_token(token_type_t::MODULO);
        case '^':
            if(lexer.match_char('=')) {
                return lexer.make_token(token_type_t::XOR_EQUALS);
            }
            return lexer.make_token(token_type_t::BITWISE_XOR);
        case ',':
            return lexer.make_token(token_type_t::COMMA);
        case '?':
            return lexer.make_token(token_type_t::QUESTION_MARK);
        case ':':
            return lexer.make_token(token_type_t::COLON);
    }

    std::cout << "Unrecognized token: " << c << "\n";
    return lexer.make_token(token_type_t::ERROR);
}
// TODO: double check that `merge_tokens` is correct and test it thoroughly
std::vector<token_t> merge_tokens(const std::vector<token_t>& tokens) {
    std::vector<token_t> result;
    std::uint32_t i = 0;
    for(;;) {
        if(i >= tokens.size()) {
            break;
        }
        const auto token = tokens.at(i);
        // TODO: remove a bunch of code duplication and clean this up
        if(token.token_type == token_type_t::UNSIGNED_KEYWORD) {
            if((++i) >= tokens.size()) {
                result.push_back(token_t{token_type_t::UNSIGNED_INT_KEYWORD, "unsigned int", token.line_number});
            }
            const auto token_2 = tokens.at(i);
            if(token_2.token_type == token_type_t::INT_KEYWORD) {
                ++i;
                result.push_back(token_t{token_type_t::UNSIGNED_INT_KEYWORD, "unsigned int", token.line_number});
            } else if(token_2.token_type ==  token_type_t::LONG_KEYWORD) {
                if((++i) >= tokens.size()) {
                    result.push_back(token_t{token_type_t::UNSIGNED_LONG_KEYWORD, "unsigned long", token.line_number});
                }
                const auto token_3 = tokens.at(i);
                if(token_3.token_type == token_type_t::LONG_KEYWORD) {
                    if((++i) >= tokens.size()) {
                        result.push_back(token_t{token_type_t::UNSIGNED_LONG_LONG_KEYWORD, "unsigned long long", token.line_number});
                    }
                    const auto token_4 = tokens.at(i);
                    if(token_4.token_type == token_type_t::INT_KEYWORD) {
                        ++i;
                    }
                    result.push_back(token_t{token_type_t::UNSIGNED_LONG_LONG_KEYWORD, "unsigned long long", token.line_number});
                } else if(token_3.token_type == token_type_t::INT_KEYWORD) {
                    ++i;
                    result.push_back(token_t{token_type_t::UNSIGNED_LONG_KEYWORD, "unsigned long", token.line_number});
                } else {
                    result.push_back(token_t{token_type_t::UNSIGNED_LONG_KEYWORD, "unsigned long", token.line_number});
                }
            } else if(token_2.token_type == token_type_t::CHAR_KEYWORD) {
                ++i;
                result.push_back(token_t{token_type_t::UNSIGNED_CHAR_KEYWORD, "unsigned char", token.line_number});
            } else if(token_2.token_type == token_type_t::SHORT_KEYWORD) {
                if((++i) >= tokens.size()) {
                    result.push_back(token_t{token_type_t::UNSIGNED_SHORT_KEYWORD, "unsigned short", token.line_number});
                }
                const auto token_3 = tokens.at(i);
                if(token_3.token_type == token_type_t::INT_KEYWORD) {
                    ++i;
                }
                result.push_back(token_t{token_type_t::UNSIGNED_SHORT_KEYWORD, "unsigned short", token.line_number});
            } else {
                result.push_back(token_t{token_type_t::UNSIGNED_INT_KEYWORD, "unsigned int", token.line_number});
            }
        } else if(token.token_type == token_type_t::LONG_KEYWORD) {
            if((++i) >= tokens.size()) {
                result.push_back(token_t{token_type_t::LONG_KEYWORD, "long", token.line_number});
            }
            const auto token_2 = tokens.at(i);
            if(token_2.token_type == token_type_t::LONG_KEYWORD) {
                if((++i) >= tokens.size()) {
                    result.push_back(token_t{token_type_t::LONG_LONG_KEYWORD, "long long", token.line_number});
                }
                const auto token_3 = tokens.at(i);
                if(token_3.token_type == token_type_t::INT_KEYWORD) {
                    ++i;
                }
                result.push_back(token_t{token_type_t::LONG_LONG_KEYWORD, "long long", token.line_number});
            } else if(token_2.token_type == token_type_t::INT_KEYWORD) {
                ++i;
                result.push_back(token_t{token_type_t::LONG_KEYWORD, "long", token.line_number});
            } else {
                result.push_back(token_t{token_type_t::LONG_KEYWORD, "long", token.line_number});
            }
        } else if(token.token_type == token_type_t::SHORT_KEYWORD) {
            if((++i) >= tokens.size()) {
                result.push_back(token_t{token_type_t::SHORT_KEYWORD, "short", token.line_number});
            }
            const auto token_2 = tokens.at(i);
            if(token_2.token_type == token_type_t::INT_KEYWORD) {
                ++i;
            }
            result.push_back(token_t{token_type_t::SHORT_KEYWORD, "short", token.line_number});
        } else if(token.token_type == token_type_t::SIGNED_KEYWORD) {
            if((++i) >= tokens.size()) {
                result.push_back(token_t{token_type_t::INT_KEYWORD, "int", token.line_number});
            }
            const auto token_2 = tokens.at(i);
            if(token_2.token_type == token_type_t::CHAR_KEYWORD) {
                ++i;
                result.push_back(token_t{token_type_t::SIGNED_KEYWORD, "signed char", token.line_number});
            } else if(token_2.token_type == token_type_t::SHORT_KEYWORD) {
                if((++i) >= tokens.size()) {
                    result.push_back(token_t{token_type_t::SHORT_KEYWORD, "short", token.line_number});
                }
                const auto token_3 = tokens.at(i);
                if(token_3.token_type == token_type_t::INT_KEYWORD) {
                    ++i;
                }
                result.push_back(token_t{token_type_t::SHORT_KEYWORD, "short", token.line_number});
            } else if(token_2.token_type == token_type_t::INT_KEYWORD) {
                ++i;
                result.push_back(token_t{token_type_t::INT_KEYWORD, "int", token.line_number});
            } else if(token_2.token_type == token_type_t::LONG_KEYWORD) {
                if((++i) >= tokens.size()) {
                    result.push_back(token_t{token_type_t::LONG_KEYWORD, "long", token.line_number});
                }
                const auto token_3 = tokens.at(i);
                if(token_3.token_type == token_type_t::LONG_KEYWORD) {
                    if((++i) >= tokens.size()) {
                        result.push_back(token_t{token_type_t::LONG_LONG_KEYWORD, "long long", token.line_number});
                    }
                    const auto token_4 = tokens.at(i);
                    if(token_4.token_type == token_type_t::INT_KEYWORD) {
                        ++i;
                    }
                    result.push_back(token_t{token_type_t::LONG_LONG_KEYWORD, "long long", token.line_number});
                } else if(token_3.token_type == token_type_t::INT_KEYWORD) {
                    ++i;
                    result.push_back(token_t{token_type_t::LONG_KEYWORD, "long", token.line_number});
                } else {
                    result.push_back(token_t{token_type_t::LONG_KEYWORD, "long", token.line_number});
                }
            } else {
                result.push_back(token_t{token_type_t::INT_KEYWORD, "int", token.line_number});
            }
        } else {
            ++i;
            result.push_back(token);
        }
    }
    return result;
}
std::vector<token_t> scan_all_tokens(lexer_t& lexer) {
    std::vector<token_t> initial_tokens;
    for(;;) {
        const auto token = scan_token(lexer);
        initial_tokens.push_back(token);
        if(token.token_type == token_type_t::EOF_TOK) {
            break;
        }
    }
    return merge_tokens(initial_tokens);
}
