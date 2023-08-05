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
std::shared_ptr<ast::grouping_t> parse_grouping(parser_t& parser) {
    parser.advance_token();
    auto exp = parse_expression(parser, 0u);
    if(parser.peek_token().token_type != token_type_t::RIGHT_PAREN) {
        throw std::runtime_error("expected `)`");
    }
    parser.advance_token();
    return make_grouping(std::move(exp));
}


bool is_prefix_op(const token_t& token) {
    switch(token.token_type) {
        case token_type_t::PLUS_PLUS:
        case token_type_t::DASH_DASH:
        case token_type_t::PLUS:
        case token_type_t::DASH:
        case token_type_t::BANG:
        case token_type_t::TILDE:
            return true;
    }
    return false;
}
ast::unary_operator_token_t parse_prefix_op(const token_t& token) {
    switch(token.token_type) {
        case token_type_t::PLUS_PLUS:
            return ast::unary_operator_token_t::PLUS_PLUS;
        case token_type_t::DASH_DASH:
            return ast::unary_operator_token_t::MINUS_MINUS;
        case token_type_t::PLUS:
            return ast::unary_operator_token_t::PLUS;
        case token_type_t::DASH:
            return ast::unary_operator_token_t::MINUS;
        case token_type_t::BANG:
            return ast::unary_operator_token_t::LOGICAL_NOT;
        case token_type_t::TILDE:
            return ast::unary_operator_token_t::BITWISE_NOT;
    }
    throw std::runtime_error("invalid prefix token.");
}
ast::precedence_t prefix_binding_power(const ast::unary_operator_token_t token) {
    switch(token) {
        case ast::unary_operator_token_t::PLUS_PLUS:
        case ast::unary_operator_token_t::MINUS_MINUS:
        case ast::unary_operator_token_t::PLUS:
        case ast::unary_operator_token_t::MINUS:
        case ast::unary_operator_token_t::LOGICAL_NOT:
        case ast::unary_operator_token_t::BITWISE_NOT:
            return 25;
    }
    throw std::runtime_error("invalid prefix token.");
}
std::shared_ptr<ast::unary_expression_t> make_prefix_op(const ast::unary_operator_token_t op, ast::expression_t&& rhs) {
    return std::make_shared<ast::unary_expression_t>(ast::unary_expression_t{ast::unary_operator_fixity_t::PREFIX, op, std::move(rhs)});
}
ast::expression_t parse_prefix_expression(parser_t& parser) {
    switch(parser.peek_token().token_type) {
        case token_type_t::IDENTIFIER:
            return parse_var_name(parser);
        case token_type_t::INT_CONSTANT:
            return parse_constant(parser);
        case token_type_t::LEFT_PAREN:
            return parse_grouping(parser);
        default:
            if(is_prefix_op(parser.peek_token())) {
                auto op = parse_prefix_op(parser.advance_token());
                auto r_bp = prefix_binding_power(op);
                auto rhs = parse_expression(parser, r_bp);
                return make_prefix_op(op, std::move(rhs));
            }
            throw std::runtime_error("invalid prefix expression");
    }
}
bool is_postfix_op(const token_t& token) {
    switch(token.token_type) {
        case token_type_t::PLUS_PLUS:
        case token_type_t::DASH_DASH:
            return true;
    }
    return false;
}
ast::unary_operator_token_t parse_postfix_op(const token_t& token) {
    switch(token.token_type) {
        case token_type_t::PLUS_PLUS:
            return ast::unary_operator_token_t::PLUS_PLUS;
        case token_type_t::DASH_DASH:
            return ast::unary_operator_token_t::MINUS_MINUS;
    }
    throw std::runtime_error("invalid postfix token.");
}
ast::precedence_t postfix_binding_power(const ast::unary_operator_token_t token) {
    switch(token) {
        case ast::unary_operator_token_t::PLUS_PLUS:
        case ast::unary_operator_token_t::MINUS_MINUS:
            return 26;
    }
    throw std::runtime_error("invalid postfix token.");
}
std::shared_ptr<ast::unary_expression_t> make_postfix_op(const ast::unary_operator_token_t op, ast::expression_t&& lhs) {
    return std::make_shared<ast::unary_expression_t>(ast::unary_expression_t{ast::unary_operator_fixity_t::POSTFIX, op, std::move(lhs)});
}
bool is_infix_binary_op(const token_t& token) {
    switch(token.token_type) {
        case token_type_t::ASTERISK:
        case token_type_t::SLASH:
        case token_type_t::MODULO:
        case token_type_t::PLUS:
        case token_type_t::DASH:
        case token_type_t::BITWISE_LEFT_SHIFT:
        case token_type_t::BITWISE_RIGHT_SHIFT:
        case token_type_t::LESS_THAN:
        case token_type_t::LESS_THAN_EQUAL:
        case token_type_t::GREATER_THAN:
        case token_type_t::GREATER_THAN_EQUAL:
        case token_type_t::EQUAL_EQUAL:
        case token_type_t::NOT_EQUAL:
        case token_type_t::BITWISE_AND:
        case token_type_t::BITWISE_XOR:
        case token_type_t::BITWISE_OR:
        case token_type_t::LOGIC_AND:
        case token_type_t::LOGIC_OR:
        case token_type_t::EQUALS:
        case token_type_t::COMMA:
            return true;
    }
    return false;
}
ast::binary_operator_token_t parse_infix_binary_op(const token_t& token) {
    switch(token.token_type) {
        case token_type_t::ASTERISK:
            return ast::binary_operator_token_t::MULTIPLY;
        case token_type_t::SLASH:
            return ast::binary_operator_token_t::DIVIDE;
        case token_type_t::MODULO:
            return ast::binary_operator_token_t::MODULO;
        case token_type_t::PLUS:
            return ast::binary_operator_token_t::PLUS;
        case token_type_t::DASH:
            return ast::binary_operator_token_t::MINUS;
        case token_type_t::BITWISE_LEFT_SHIFT:
            return ast::binary_operator_token_t::LEFT_BITSHIFT;
        case token_type_t::BITWISE_RIGHT_SHIFT:
            return ast::binary_operator_token_t::RIGHT_BITSHIFT;
        case token_type_t::LESS_THAN:
            return ast::binary_operator_token_t::LESS_THAN;
        case token_type_t::LESS_THAN_EQUAL:
            return ast::binary_operator_token_t::LESS_THAN_EQUAL;
        case token_type_t::GREATER_THAN:
            return ast::binary_operator_token_t::GREATER_THAN;
        case token_type_t::GREATER_THAN_EQUAL:
            return ast::binary_operator_token_t::GREATER_THAN_EQUAL;
        case token_type_t::EQUAL_EQUAL:
            return ast::binary_operator_token_t::EQUAL;
        case token_type_t::NOT_EQUAL:
            return ast::binary_operator_token_t::NOT_EQUAL;
        case token_type_t::BITWISE_AND:
            return ast::binary_operator_token_t::BITWISE_AND;
        case token_type_t::BITWISE_XOR:
            return ast::binary_operator_token_t::BITWISE_XOR;
        case token_type_t::BITWISE_OR:
            return ast::binary_operator_token_t::BITWISE_OR;
        case token_type_t::LOGIC_AND:
            return ast::binary_operator_token_t::LOGICAL_AND;
        case token_type_t::LOGIC_OR:
            return ast::binary_operator_token_t::LOGICAL_OR;
        case token_type_t::EQUALS:
            return ast::binary_operator_token_t::ASSIGNMENT;
        case token_type_t::COMMA:
            return ast::binary_operator_token_t::COMMA;
    }
    throw std::runtime_error("invalid infix token.");
}
std::pair<ast::precedence_t, ast::precedence_t> infix_binding_power(const ast::binary_operator_token_t token) {
    switch(token) {
        case ast::binary_operator_token_t::MULTIPLY:
        case ast::binary_operator_token_t::DIVIDE:
        case ast::binary_operator_token_t::MODULO:
            return {24, 23};
        case ast::binary_operator_token_t::PLUS:
        case ast::binary_operator_token_t::MINUS:
            return {22, 21};
        case ast::binary_operator_token_t::LEFT_BITSHIFT:
        case ast::binary_operator_token_t::RIGHT_BITSHIFT:
            return {20, 19};
        case ast::binary_operator_token_t::LESS_THAN:
        case ast::binary_operator_token_t::LESS_THAN_EQUAL:
        case ast::binary_operator_token_t::GREATER_THAN:
        case ast::binary_operator_token_t::GREATER_THAN_EQUAL:
            return {18, 17};
        case ast::binary_operator_token_t::EQUAL:
        case ast::binary_operator_token_t::NOT_EQUAL:
            return {16, 15};
        case ast::binary_operator_token_t::BITWISE_AND:
            return {14, 13};
        case ast::binary_operator_token_t::BITWISE_XOR:
            return {12, 11};
        case ast::binary_operator_token_t::BITWISE_OR:
            return {10, 9};
        case ast::binary_operator_token_t::LOGICAL_AND:
            return {8, 7};
        case ast::binary_operator_token_t::LOGICAL_OR:
            return {6, 5};
        case ast::binary_operator_token_t::ASSIGNMENT:
            return {3, 4}; // right-to-left
        case ast::binary_operator_token_t::COMMA:
            return {2, 1}; // left-to-right
    }
    throw std::runtime_error("invalid infix token.");
}
std::shared_ptr<ast::binary_expression_t> make_infix_op(const ast::binary_operator_token_t op, ast::expression_t&& lhs, ast::expression_t&& rhs) {
    return std::make_shared<ast::binary_expression_t>(ast::binary_expression_t{op, std::move(lhs), std::move(rhs)}); // we validate this is a valid lvalue in the backend during codegen ast traversal
}
bool is_compound_assignment_op(const token_t& token) {
    switch(token.token_type) {
        case token_type_t::PLUS_EQUALS:
        case token_type_t::MINUS_EQUALS:
        case token_type_t::TIMES_EQUALS:
        case token_type_t::DIVIDE_EQUALS:
        case token_type_t::MODULO_EQUALS:
        case token_type_t::AND_EQUALS:
        case token_type_t::OR_EQUALS:
        case token_type_t::XOR_EQUALS:
        case token_type_t::LEFT_SHIFT_EQUALS:
        case token_type_t::RIGHT_SHIFT_EQUALS:
            return true;
    }
    return false;
}
ast::binary_operator_token_t get_op_from_compound_assignment_op(const token_t& token) {
    switch(token.token_type) {
        case token_type_t::PLUS_EQUALS:
            return ast::binary_operator_token_t::PLUS;
        case token_type_t::MINUS_EQUALS:
            return ast::binary_operator_token_t::MINUS;
        case token_type_t::TIMES_EQUALS:
            return ast::binary_operator_token_t::MULTIPLY;
        case token_type_t::DIVIDE_EQUALS:
            return ast::binary_operator_token_t::DIVIDE;
        case token_type_t::MODULO_EQUALS:
            return ast::binary_operator_token_t::MODULO;
        case token_type_t::AND_EQUALS:
            return ast::binary_operator_token_t::BITWISE_AND;
        case token_type_t::OR_EQUALS:
            return ast::binary_operator_token_t::BITWISE_OR;
        case token_type_t::XOR_EQUALS:
            return ast::binary_operator_token_t::BITWISE_XOR;
        case token_type_t::LEFT_SHIFT_EQUALS:
            return ast::binary_operator_token_t::LEFT_BITSHIFT;
        case token_type_t::RIGHT_SHIFT_EQUALS:
            return ast::binary_operator_token_t::RIGHT_BITSHIFT;
    }
    throw std::runtime_error("invalid compound assignment token.");
}
ast::expression_t parse_expression(parser_t& parser, const ast::precedence_t precedence) {
    auto lhs = parse_prefix_expression(parser);

    for(;;) {
        if(parser.peek_token().token_type == token_type_t::EOF_TOK) {
            break;
        }

        if(is_postfix_op(parser.peek_token())) {
            auto op = parse_postfix_op(parser.peek_token());
            auto post_bp = postfix_binding_power(op);
            if(post_bp < precedence) {
                break;
            }
            parser.advance_token();
            lhs = make_postfix_op(op, std::move(lhs));
            continue;
        }

        if(is_infix_binary_op(parser.peek_token())) {
            auto op = parse_infix_binary_op(parser.peek_token());
            auto [r_bp, l_bp] = infix_binding_power(op);
            if(l_bp < precedence) {
                break;
            }
            parser.advance_token();
            auto rhs = parse_expression(parser, r_bp);
            lhs = make_infix_op(op, std::move(lhs), std::move(rhs));
            continue;
        }

        if(is_compound_assignment_op(parser.peek_token())) {
            auto op = get_op_from_compound_assignment_op(parser.peek_token());
            parser.advance_token();
            lhs = make_infix_op(ast::binary_operator_token_t::ASSIGNMENT, validate_lvalue_expression_exp(lhs), make_infix_op(op, validate_lvalue_expression_exp(lhs), parse_expression(parser, precedence)));
            continue;
        }

        break;
    }

    return lhs;
}
ast::expression_t parse_expression(parser_t& parser) {
    return parse_expression(parser, 0u);
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
        statements.push_back(ast::return_statement_t { ast::expression_t{ast::constant_t{0}} } );
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
