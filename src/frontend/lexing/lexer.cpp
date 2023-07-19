#include "lexer.hpp"


// TODO: we will need to handle floats later, for now we will just handle integer constants
token_type_t handle_number(lexer_t& lexer) {
    while(utils::is_digit(lexer.peek_char())) {
        lexer.advance_char();
    }
    return token_type_t::INT_CONSTANT;
}

token_type_t handle_identifier(lexer_t& lexer) {
    while(utils::is_alpha_num(lexer.peek_char())) {
        lexer.advance_char();
    }
    return handle_keywords(lexer);
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
        case 'i':
            return match_keyword(lexer, 1, "nt", token_type_t::INT_KEYWORD);
        case 'r':
            return match_keyword(lexer, 1, "eturn", token_type_t::RETURN_KEYWORD);
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
        // TODO: disambiguate `-` from `--` and `-=`
        case '-':
            return lexer.make_token(token_type_t::DASH);
        case '~':
            return lexer.make_token(token_type_t::TILDE);
        // TODO: disambiguate `!` from `!=`
        case '!':
            return lexer.make_token(token_type_t::BANG);
        case '+':
            return lexer.make_token(token_type_t::PLUS);
        case '*':
            return lexer.make_token(token_type_t::ASTERISK);
        case '/':
            return lexer.make_token(token_type_t::SLASH);
    }

    std::cout << "Unrecognized token: " << c << "\n";
    return lexer.make_token(token_type_t::ERROR);
}

std::vector<token_t> scan_all_tokens(lexer_t& lexer) {
    std::vector<token_t> tokens;
    for(;;) {
        const auto token = scan_token(lexer);
        tokens.push_back(token);
        if(token.token_type == token_type_t::EOF_TOK) {
            break;
        }
    }
    return tokens;
}

