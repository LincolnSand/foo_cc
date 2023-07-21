#include "parser.hpp"


std::shared_ptr<ast::grouping_t> parse_grouping(parser_t& parser) {
    parser.advance_token();
    auto exp = parse_expression(parser);
    if(parser.peek_token().token_type != token_type_t::RIGHT_PAREN) {
        throw std::runtime_error("expected `)`");
    }
    parser.advance_token();
    return make_grouping(exp);
}
std::shared_ptr<ast::unary_op_expression_t> parse_unary_expression(parser_t& parser) {
    auto op = convert_to_unary_token(parser.advance_token());
    auto factor = parse_factor(parser);
    return make_unary_expression(op, factor);
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
        return parse_grouping(parser);
    } else if(is_unary_operator(next)) {
        return parse_unary_expression(parser);
    } else if(is_constant(next)) {
        return parse_constant(parser);
    } else {
        throw std::runtime_error("invalid factor");
    }
}
ast::times_divide_expression_t parse_times_divide_expression(parser_t& parser) {
    ast::times_divide_expression_t first_param = parse_factor(parser);
    while(parser.peek_token().token_type == token_type_t::ASTERISK || parser.peek_token().token_type == token_type_t::SLASH) {
        auto op = convert_to_times_divide_token(parser.advance_token());
        ast::times_divide_expression_t second_term = parse_factor(parser);
        first_param = make_times_divide_binary_expression(first_param, op, second_term);
    }
    return first_param;
}
ast::plus_minus_expression_t parse_plus_minus_expression(parser_t& parser) {
    ast::plus_minus_expression_t first_param = parse_times_divide_expression(parser);
    while(parser.peek_token().token_type == token_type_t::PLUS || parser.peek_token().token_type == token_type_t::DASH) {
        auto op = convert_to_plus_minus_token(parser.advance_token());
        ast::plus_minus_expression_t second_term = parse_times_divide_expression(parser);
        first_param = make_plus_minus_binary_expression(first_param, op, second_term);
    }
    return first_param;
}
ast::relational_expression_t parse_relational_expression(parser_t& parser) {
    ast::relational_expression_t first_param = parse_plus_minus_expression(parser);
    while(parser.peek_token().token_type == token_type_t::LESS_THAN || parser.peek_token().token_type == token_type_t::GREATER_THAN || parser.peek_token().token_type == token_type_t::LESS_THAN_EQUAL || parser.peek_token().token_type == token_type_t::GREATER_THAN_EQUAL) {
        auto op = convert_to_relational_token(parser.advance_token());
        ast::relational_expression_t second_term = parse_plus_minus_expression(parser);
        first_param = make_relational_binary_expression(first_param, op, second_term);
    }
    return first_param;
}
ast::equality_expression_t parse_equality_expression(parser_t& parser) {
    ast::equality_expression_t first_param = parse_relational_expression(parser);
    while(parser.peek_token().token_type == token_type_t::EQUAL_EQUAL || parser.peek_token().token_type == token_type_t::NOT_EQUAL) {
        auto op = convert_to_equality_token(parser.advance_token());
        ast::equality_expression_t second_term = parse_relational_expression(parser);
        first_param = make_equality_binary_expression(first_param, op, second_term);
    }
    return first_param;
}
ast::logical_and_expression_t parse_logical_and_expression(parser_t& parser) {
    ast::logical_and_expression_t first_param = parse_equality_expression(parser);
    while(parser.peek_token().token_type == token_type_t::LOGIC_AND) {
        auto op = convert_to_logical_and_token(parser.advance_token());
        ast::logical_and_expression_t second_term = parse_equality_expression(parser);
        first_param = make_logical_and_binary_expression(first_param, op, second_term);
    }
    return first_param;
}
ast::logical_or_expression_t parse_logical_or_expression(parser_t& parser) {
    ast::logical_or_expression_t first_param = parse_logical_and_expression(parser);
    while(parser.peek_token().token_type == token_type_t::LOGIC_OR) {
        auto op = convert_to_logical_or_token(parser.advance_token());
        ast::logical_or_expression_t second_param = parse_logical_and_expression(parser);
        first_param = make_logical_or_binary_expression(first_param, op, second_param);
    }
    return first_param;
}
ast::expression_t parse_expression(parser_t& parser) {
    return parse_logical_or_expression(parser);
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
    return ast::program_t { parse_function_decl(parser) };
}
