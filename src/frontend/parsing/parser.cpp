#include "parser.hpp"


bool is_unop(token_t token) {
    switch(token.token_type) {
        case token_type_t::DASH:
        case token_type_t::TILDE:
        case token_type_t::BANG:
            return true;
    }
    return false;
}

ast::unary_op_t convert_to_unary_token(token_t token) {
    switch(token.token_type) {
        case token_type_t::DASH:
            return ast::unary_op_t::NEG;
        case token_type_t::TILDE:
            return ast::unary_op_t::BITWISE_NOT;
        case token_type_t::BANG:
            return ast::unary_op_t::LOGIC_NOT;
    }
    throw std::runtime_error("invalid unary operator");
}

ast::times_divide_t convert_to_times_divide_token(token_t token) {
    switch(token.token_type) {
        case token_type_t::ASTERISK:
            return ast::times_divide_t::TIMES;
        case token_type_t::SLASH:
            return ast::times_divide_t::DIVIDE;
    }
    throw std::runtime_error("Not either `*` or `/`");
}

ast::plus_minus_t convert_to_add_subtract_token(token_t token) {
    switch(token.token_type) {
        case token_type_t::PLUS:
            return ast::plus_minus_t::PLUS;
        case token_type_t::DASH:
            return ast::plus_minus_t::MINUS;
    }
    throw std::runtime_error("Not either `+` or `-`");
}

std::shared_ptr<ast::times_divide_expression_t> make_times_divide_expression(ast::term_t lhs, ast::times_divide_t op, ast::term_t rhs) {
    return std::make_unique<ast::times_divide_expression_t>(ast::times_divide_expression_t { op, std::move(lhs), std::move(rhs) });
}
std::shared_ptr<ast::plus_minus_expression_t> make_plus_minus_expression(ast::expression_t lhs, ast::plus_minus_t op, ast::expression_t rhs) {
    return std::make_unique<ast::plus_minus_expression_t>(ast::plus_minus_expression_t { op, std::move(lhs), std::move(rhs) });
}

std::pair<ast::term_t*, ast::term_t*> get_non_owning_term_references_from_times_divide_expr(std::shared_ptr<ast::times_divide_expression_t>& expr) {
    return { &expr->lhs, &expr->rhs };
}
std::pair<ast::expression_t*, ast::expression_t*> get_non_owning_expression_references_from_plus_minus_expr(std::shared_ptr<ast::plus_minus_expression_t>& expr) {
    return { &expr->lhs, &expr->rhs };
}

ast::constant_t parse_constant(parser_t& parser) {
    auto next = parser.advance_token();
    if(next.token_type != token_type_t::INT_CONSTANT) {
        throw std::runtime_error("invalid constant: [" + std::to_string(static_cast<std::uint32_t>(next.token_type)) + std::string("]"));
    }

    int result{};
    utils::str_to_int(next.token_text, result);
    return ast::constant_t { result };
}

ast::factor_t parse_factor(parser_t& parser) {
    auto next = parser.peek_token();
    if(next.token_type == token_type_t::LEFT_PAREN) {
        parser.advance_token();
        auto exp = parse_expression(parser);
        if(parser.peek_token().token_type != token_type_t::RIGHT_PAREN) {
            throw std::runtime_error("expected `)`");
        }
        parser.advance_token();
        return make_grouping(exp);
    } else if(is_unop(next)) {
        auto op = convert_to_unary_token(next);
        parser.advance_token();
        auto factor = parse_factor(parser);
        return make_unop(op, factor);
    } else if(next.token_type == token_type_t::INT_CONSTANT) {
        return parse_constant(parser);
    } else {
        throw std::runtime_error("invalid factor");
    }
}

ast::term_t parse_term(parser_t& parser) {
    ast::term_t factor = parse_factor(parser);
    while(parser.peek_token().token_type == token_type_t::ASTERISK || parser.peek_token().token_type == token_type_t::SLASH) {
        auto op = convert_to_times_divide_token(parser.advance_token());
        ast::term_t next_factor = parse_factor(parser);
        factor = make_times_divide_expression(factor, op, next_factor);
    }
    return factor;
}

// right-associative implementation
#if 0
ast::expression_t parse_expression(parser_t& parser) {
    ast::expression_t first_term = parse_term(parser);
    if(parser.peek_token().token_type == token_type_t::PLUS || parser.peek_token().token_type == token_type_t::DASH) {
        auto op = convert_to_add_subtract_token(parser.advance_token());
        ast::expression_t next_term = parse_expression(parser);
        return make_plus_minus_expression(first_term, op, next_term);
    }
    return first_term;
}
#endif

// left-associative implementation, which is correct according to the grammar and standard of C
ast::expression_t parse_expression(parser_t& parser) {
    ast::expression_t term = parse_term(parser);
    while(parser.peek_token().token_type == token_type_t::PLUS || parser.peek_token().token_type == token_type_t::DASH) {
        auto op = convert_to_add_subtract_token(parser.advance_token());
        ast::expression_t next_term = parse_term(parser);
        term = make_plus_minus_expression(term, op, next_term);
    }
    return term;
}


ast::return_statement_t parse_statement(parser_t& parser) {
    const auto keyword_token = parser.advance_token();
    if(keyword_token.token_type != token_type_t::RETURN_KEYWORD) {
        throw std::runtime_error("expected `return` keyword in statement");
    }

    auto expression = parse_expression(parser);

    const auto semicolon_token = parser.advance_token();
    if(semicolon_token.token_type != token_type_t::SEMICOLON) {
        throw std::runtime_error("expected `;` in statement");
    }

    return ast::return_statement_t{std::move(expression)};
}

ast::function_declaration_t parse_function_decl(parser_t& parser) {
    const auto keyword_token = parser.advance_token();
    if(keyword_token.token_type != token_type_t::INT_KEYWORD) {
        throw std::runtime_error("expected `int` keyword in function declaration");
    }

    const auto name_token = parser.advance_token();
    if(name_token.token_type != token_type_t::IDENTIFIER) {
        throw std::runtime_error("expected identifier name in function declaration");
    }

    auto paren_token = parser.advance_token();
    if(paren_token.token_type != token_type_t::LEFT_PAREN) {
        throw std::runtime_error("expected `(` in function declaration");
    }

    paren_token = parser.advance_token();
    if(paren_token.token_type != token_type_t::RIGHT_PAREN) {
        throw std::runtime_error("expected `)` in function declaration");
    }

    auto curly_token = parser.advance_token();
    if(curly_token.token_type != token_type_t::LEFT_CURLY) {
        std::printf("Unexpected token: [%.*s] with type [%d]\n", curly_token.token_text.length(), curly_token.token_text.data(), curly_token.token_type);
        throw std::runtime_error("expected `{` in function declaration");
    }

    auto statement = parse_statement(parser);

    curly_token = parser.advance_token();
    if(curly_token.token_type != token_type_t::RIGHT_CURLY) {
        throw std::runtime_error("expected `}` in function declaration");
    }

    return ast::function_declaration_t{ std::string(name_token.token_text), std::move(statement) };
}

ast::program_t parse(parser_t& parser) {
    return ast::program_t{ parse_function_decl(parser) };
}
