#include "parser.hpp"


ast::var_name_t parse_var_name(parser_t& parser) {
    auto identifier_token = parser.advance_token();
    if(!is_var_name(identifier_token)) {
        throw std::runtime_error("invalid identifier: [" + std::to_string(static_cast<std::uint32_t>(parser.peek_token().token_type)) + std::string("]"));
    }

    return ast::var_name_t { identifier_token.token_text };
}
ast::constant_t parse_constant(parser_t& parser) {
    auto next = parser.advance_token();
    if(!is_constant(next)) {
        throw std::runtime_error("invalid constant: [" + std::to_string(static_cast<std::uint32_t>(next.token_type)) + std::string("]"));
    }

    int result{};
    utils::str_to_int(next.token_text, result);
    return ast::constant_t { result };
}
std::shared_ptr<ast::highest_precedence_unary_expression_t> parse_highest_precedence_unary_expression(parser_t& parser) {
    auto op = convert_to_highest_precedence_unary_token(parser.advance_token());
    auto factor = parse_factor(parser);
    return make_highest_precedence_unary_expression(op, factor);
}
std::shared_ptr<ast::grouping_t> parse_grouping(parser_t& parser) {
    parser.advance_token();
    auto exp = parse_expression(parser);
    if(parser.peek_token().token_type != token_type_t::RIGHT_PAREN) {
        throw std::runtime_error("expected `)`");
    }
    parser.advance_token();
    return make_grouping(exp);
}
ast::factor_t parse_factor(parser_t& parser) {
    auto next = parser.peek_token();
    if(next.token_type == token_type_t::LEFT_PAREN) {
        return parse_grouping(parser);
    } else if(is_highest_precedence_unary_operator(next)) {
        return parse_highest_precedence_unary_expression(parser);
    } else if(is_constant(next)) {
        return parse_constant(parser);
    } else if(is_var_name(next)) {
        return parse_var_name(parser);
    } else {
        throw std::runtime_error("invalid factor");
    }
}
ast::unary_expression_t parse_unary_expression(parser_t& parser) {
    if(is_unary_op(parser.peek_token())) {
        auto op = convert_to_unary_token(parser.advance_token());
        return make_unary_expression(op, parse_unary_expression(parser));
    }
    return ast::unary_expression_t{parse_factor(parser)};
}
ast::times_divide_expression_t parse_times_divide_expression(parser_t& parser) {
    ast::times_divide_expression_t first_param = parse_unary_expression(parser);
    while(parser.peek_token().token_type == token_type_t::ASTERISK || parser.peek_token().token_type == token_type_t::SLASH) {
        auto op = convert_to_times_divide_token(parser.advance_token());
        first_param = make_times_divide_binary_expression(first_param, op, parse_unary_expression(parser));
    }
    return first_param;
}
ast::plus_minus_expression_t parse_plus_minus_expression(parser_t& parser) {
    ast::plus_minus_expression_t first_param = parse_times_divide_expression(parser);
    while(parser.peek_token().token_type == token_type_t::PLUS || parser.peek_token().token_type == token_type_t::DASH) {
        auto op = convert_to_plus_minus_token(parser.advance_token());
        first_param = make_plus_minus_binary_expression(first_param, op, parse_times_divide_expression(parser));
    }
    return first_param;
}
ast::bitshift_expression_t parse_bitshift_expression(parser_t& parser) {
    ast::bitshift_expression_t first_param = parse_plus_minus_expression(parser);
    while(parser.peek_token().token_type == token_type_t::BITWISE_LEFT_SHIFT || parser.peek_token().token_type == token_type_t::BITWISE_RIGHT_SHIFT) {
        auto op = convert_to_bitshift_token(parser.advance_token());
        first_param = make_bitshift_binary_expression(first_param, op, parse_plus_minus_expression(parser));
    }
    return first_param;
}
ast::relational_expression_t parse_relational_expression(parser_t& parser) {
    ast::relational_expression_t first_param = parse_bitshift_expression(parser);
    while(parser.peek_token().token_type == token_type_t::LESS_THAN || parser.peek_token().token_type == token_type_t::GREATER_THAN || parser.peek_token().token_type == token_type_t::LESS_THAN_EQUAL || parser.peek_token().token_type == token_type_t::GREATER_THAN_EQUAL) {
        auto op = convert_to_relational_token(parser.advance_token());
        first_param = make_relational_binary_expression(first_param, op, parse_bitshift_expression(parser));
    }
    return first_param;
}
ast::equality_expression_t parse_equality_expression(parser_t& parser) {
    ast::equality_expression_t first_param = parse_relational_expression(parser);
    while(parser.peek_token().token_type == token_type_t::EQUAL_EQUAL || parser.peek_token().token_type == token_type_t::NOT_EQUAL) {
        auto op = convert_to_equality_token(parser.advance_token());
        first_param = make_equality_binary_expression(first_param, op, parse_relational_expression(parser));
    }
    return first_param;
}
ast::bitwise_and_expression_t parse_bitwise_and_expression(parser_t& parser) {
    ast::bitwise_and_expression_t first_param = parse_equality_expression(parser);
    while(parser.peek_token().token_type == token_type_t::BITWISE_AND) {
        parser.advance_token(); // consume `&` token
        first_param = make_bitwise_and_binary_expression(first_param, parse_equality_expression(parser));
    }
    return first_param;
}
ast::bitwise_xor_expression_t parse_bitwise_xor_expression(parser_t& parser) {
    ast::bitwise_xor_expression_t first_param = parse_bitwise_and_expression(parser);
    while(parser.peek_token().token_type == token_type_t::BITWISE_XOR) {
        parser.advance_token(); // consume `^` token
        first_param = make_bitwise_xor_binary_expression(first_param, parse_bitwise_and_expression(parser));
    }
    return first_param;
}
ast::bitwise_or_expression_t parse_bitwise_or_expression(parser_t& parser) {
    ast::bitwise_or_expression_t first_param = parse_bitwise_xor_expression(parser);
    while(parser.peek_token().token_type == token_type_t::BITWISE_OR) {
        parser.advance_token(); // consume `|` token
        first_param = make_bitwise_or_binary_expression(first_param, parse_bitwise_xor_expression(parser));
    }
    return first_param;
}
ast::logical_and_expression_t parse_logical_and_expression(parser_t& parser) {
    ast::logical_and_expression_t first_param = parse_bitwise_or_expression(parser);
    while(parser.peek_token().token_type == token_type_t::LOGIC_AND) {
        parser.advance_token(); // consume `&&` token
        first_param = make_logical_and_binary_expression(first_param, parse_bitwise_or_expression(parser));
    }
    return first_param;
}
ast::logical_or_expression_t parse_logical_or_expression(parser_t& parser) {
    ast::logical_or_expression_t first_param = parse_logical_and_expression(parser);
    while(parser.peek_token().token_type == token_type_t::LOGIC_OR) {
        parser.advance_token(); // consume `||` token
        first_param = make_logical_or_binary_expression(first_param, parse_logical_and_expression(parser));
    }
    return first_param;
}
ast::assignment_expression_t parse_assignment_expression(parser_t& parser) {
    ast::assignment_expression_t first_param = parse_logical_or_expression(parser);
    if(parser.peek_token().token_type == token_type_t::EQUALS) {
        parser.advance_token(); // consume `=` token
        return make_assignment_expression(first_param, parse_logical_or_expression(parser)); // `make_assignment_expression()` validates that `first_param` is a valid lvalue
    }
    return first_param;
}
ast::comma_operator_expression_t parse_comma_expression(parser_t& parser) {
    ast::comma_operator_expression_t first_param = parse_assignment_expression(parser);
    while(parser.peek_token().token_type == token_type_t::COMMA) {
        parser.advance_token(); // consume `,` token
        first_param = make_comma_operator_expression(first_param, parse_assignment_expression(parser));
    }
    return first_param;
}
ast::expression_t parse_expression(parser_t& parser) {
    return parse_comma_expression(parser);
}
ast::declaration_t parse_declaration(parser_t& parser) {
    if(parser.advance_token().token_type != token_type_t::INT_KEYWORD) {
        throw std::runtime_error("expected `int` keyword");
    }

    auto identifier_token = parser.advance_token();
    if(identifier_token.token_type != token_type_t::IDENTIFIER) {
        throw std::runtime_error("expected identifier");
    }

    if(parser.peek_token().token_type != token_type_t::EQUALS) {
        return ast::declaration_t{ast::var_name_t(identifier_token.token_text), std::nullopt};
    }

    parser.advance_token(); // consume `=` token

    return ast::declaration_t{ast::var_name_t(identifier_token.token_text), parse_expression(parser)};
}
ast::return_statement_t parse_return_statement(parser_t& parser) {
    const auto keyword_token = parser.advance_token();
    if(keyword_token.token_type != token_type_t::RETURN_KEYWORD) {
        throw std::runtime_error("expected `return` keyword in statement");
    }

    auto expression = parse_expression(parser);

    return ast::return_statement_t{std::move(expression)};
}
static ast::statement_t parse_statement_helper(parser_t& parser) {
    if(parser.peek_token().token_type == token_type_t::RETURN_KEYWORD) {
        return parse_return_statement(parser);
    } else if(parser.peek_token().token_type == token_type_t::INT_KEYWORD) {
        return parse_declaration(parser);
    }
    return parse_expression(parser);
}
ast::statement_t parse_statement(parser_t& parser) {
    if(parser.is_eof()) {
        throw std::runtime_error("unexpected end of file");
    }

    ast::statement_t statement = parse_statement_helper(parser);

    const auto semicolon_token = parser.advance_token();
    if(semicolon_token.token_type != token_type_t::SEMICOLON) {
        throw std::runtime_error("expected `;` in statement");
    }

    return statement;
}
// TODO: generate `return 0;` if no return statement in `main`
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

    std::vector<ast::statement_t> statements;

    while(!parser.is_eof() && parser.peek_token().token_type != token_type_t::RIGHT_CURLY) {
        statements.push_back(parse_statement(parser));
    }

    // TODO: check if the function identifier is `main` first for when we add support for other functions
    constexpr std::size_t RETURN_INDEX = 0;
    if(statements.size() == 0 || statements.at(statements.size() - 1).index() != RETURN_INDEX) {
        statements.push_back(ast::return_statement_t { make_constant_expr(0) } );
    }

    curly_token = parser.advance_token();
    if(curly_token.token_type != token_type_t::RIGHT_CURLY) {
        throw std::runtime_error("expected `}` in function declaration");
    }

    return ast::function_declaration_t{ std::string(name_token.token_text), std::move(statements) };
}
ast::program_t parse(parser_t& parser) {
    return ast::program_t { parse_function_decl(parser) };
}
