#include "parser.hpp"


static bool is_return_statement(const std::variant<ast::statement_t, ast::declaration_t>& stmt) {
    return std::visit(overloaded{
        [](const ast::statement_t& stmt) {
            return std::visit(overloaded{
                [](const ast::return_statement_t&) {
                    return true;
                },
                [](const auto&) {
                    return false;
                }
            }, stmt);
        },
        [](const ast::declaration_t&) {
            return false;
        }
    }, stmt);
}
// declared in frontend/ast/ast.hpp and used in both this file and backend/x86_64/traverse_ast.hpp
static ast::type_category_t get_type_category_from_token_type(token_type_t token_type) {
    switch(token_type) {
        case token_type_t::INT_CONSTANT:
        case token_type_t::INT_KEYWORD:
            return ast::type_category_t::INT;
        case token_type_t::DOUBLE_CONSTANT:
        case token_type_t::DOUBLE_KEYWORD:
            return ast::type_category_t::DOUBLE;
        case token_type_t::CHAR_CONSTANT:
        case token_type_t::CHAR_KEYWORD: // char is just an 8 bit integer
            return ast::type_category_t::INT;
    }
    throw std::runtime_error("Invalid/Unsupported type: [" + std::to_string(static_cast<std::uint32_t>(token_type)) + std::string("]"));
}
static std::size_t get_size_from_type(std::string_view token_text) {
    if(token_text == "char") {
        return sizeof(char);
    } else if(token_text == "int") {
        return sizeof(int);
    } else if(token_text == "double") {
        return sizeof(double);
    } else {
        throw std::runtime_error("Unsupported type: [" + std::string(token_text) + std::string("]"));
    }
}
static std::size_t get_alignment_from_type(std::string_view token_text) {
    if(token_text == "char") {
        return sizeof(char);
    } else if(token_text == "int") {
        return sizeof(int);
    } else if(token_text == "double") {
        return sizeof(double);
    } else {
        throw std::runtime_error("Unsupported type: [" + std::string(token_text) + std::string("]"));
    }
}
ast::type_name_t create_type_name_from_token(const token_t& token) {
    return ast::type_name_t { get_type_category_from_token_type(token.token_type), std::string(token.token_text), get_size_from_type(token.token_text), get_alignment_from_type(token.token_text) };
}
bool has_return_statement(const ast::compound_statement_t& compound_stmt) {
    for(const auto& stmt : compound_stmt.stmts) {
        if(is_return_statement(stmt)) {
            return true;
        }
    }
    return false;
}

ast::var_name_t parse_var_name(parser_t& parser) {
    auto identifier_token = parser.advance_token();
    if(!is_var_name(identifier_token)) {
        throw std::runtime_error("Invalid identifier: [" + std::to_string(static_cast<std::uint32_t>(parser.peek_token().token_type)) + std::string("]"));
    }

    return ast::var_name_t { identifier_token.token_text };
}
template<typename T>
ast::constant_t parse_constant(parser_t& parser) {
    auto next = parser.advance_token();
    if(!is_constant(next)) {
        throw std::runtime_error("Invalid constant: [" + std::to_string(static_cast<std::uint32_t>(next.token_type)) + std::string("]"));
    }

    T result{};
    utils::str_to_int(next.token_text, result);
    return ast::constant_t { result };
}
ast::expression_t parse_int_constant(parser_t& parser) {
    return ast::expression_t { parse_constant<int>(parser), ast::type_name_t{ast::type_category_t::INT, "int", sizeof(std::uint64_t), sizeof(std::uint64_t)} };
}
ast::expression_t parse_double_constant(parser_t& parser) {
    return ast::expression_t { parse_constant<double>(parser), ast::type_name_t{ast::type_category_t::DOUBLE, "double", sizeof(double), sizeof(double)} };
}
ast::expression_t parse_char_constant(parser_t& parser) {
    return ast::expression_t { ast::constant_t { parser.advance_token().token_text[1] }, ast::type_name_t{ast::type_category_t::INT, "char", sizeof(char), sizeof(char)} };
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
    throw std::runtime_error("Invalid prefix token.");
}
ast::precedence_t prefix_binding_power(const ast::unary_operator_token_t token) {
    switch(token) {
        case ast::unary_operator_token_t::PLUS_PLUS:
        case ast::unary_operator_token_t::MINUS_MINUS:
        case ast::unary_operator_token_t::PLUS:
        case ast::unary_operator_token_t::MINUS:
        case ast::unary_operator_token_t::LOGICAL_NOT:
        case ast::unary_operator_token_t::BITWISE_NOT:
            return 27;
    }
    throw std::runtime_error("Invalid prefix token.");
}
std::shared_ptr<ast::unary_expression_t> make_prefix_op(const ast::unary_operator_token_t op, ast::expression_t&& rhs) {
    return std::make_shared<ast::unary_expression_t>(ast::unary_expression_t{ast::unary_operator_fixity_t::PREFIX, op, std::move(rhs)});
}
ast::expression_t parse_prefix_expression(parser_t& parser) {
    switch(parser.peek_token().token_type) {
        case token_type_t::IDENTIFIER:
            return {parse_var_name(parser), std::nullopt};
        case token_type_t::INT_CONSTANT:
            return parse_int_constant(parser);
        case token_type_t::DOUBLE_CONSTANT:
            return parse_double_constant(parser);
        case token_type_t::CHAR_CONSTANT:
            return parse_char_constant(parser);
        case token_type_t::LEFT_PAREN:
            return {parse_grouping(parser), std::nullopt}; // I could set the type here since it is trivial, but it is slightly cleaner to just do it later in the validation/typing pass
        default:
            if(is_prefix_op(parser.peek_token())) {
                auto op = parse_prefix_op(parser.advance_token());
                auto r_bp = prefix_binding_power(op);
                auto rhs = parse_expression(parser, r_bp);
                return {make_prefix_op(op, std::move(rhs)), std::nullopt};
            }
            throw std::runtime_error("Invalid prefix expression.");
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
    throw std::runtime_error("Invalid postfix token.");
}
ast::precedence_t postfix_binding_power(const ast::unary_operator_token_t token) {
    switch(token) {
        case ast::unary_operator_token_t::PLUS_PLUS:
        case ast::unary_operator_token_t::MINUS_MINUS:
            return 28;
    }
    throw std::runtime_error("Invalid postfix token.");
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
    throw std::runtime_error("Invalid infix token.");
}
std::pair<ast::precedence_t, ast::precedence_t> infix_binding_power(const ast::binary_operator_token_t token) {
    switch(token) {
        case ast::binary_operator_token_t::MULTIPLY:
        case ast::binary_operator_token_t::DIVIDE:
        case ast::binary_operator_token_t::MODULO:
            return {26, 25};
        case ast::binary_operator_token_t::PLUS:
        case ast::binary_operator_token_t::MINUS:
            return {24, 23};
        case ast::binary_operator_token_t::LEFT_BITSHIFT:
        case ast::binary_operator_token_t::RIGHT_BITSHIFT:
            return {22, 21};
        case ast::binary_operator_token_t::LESS_THAN:
        case ast::binary_operator_token_t::LESS_THAN_EQUAL:
        case ast::binary_operator_token_t::GREATER_THAN:
        case ast::binary_operator_token_t::GREATER_THAN_EQUAL:
            return {20, 19};
        case ast::binary_operator_token_t::EQUAL:
        case ast::binary_operator_token_t::NOT_EQUAL:
            return {18, 17};
        case ast::binary_operator_token_t::BITWISE_AND:
            return {16, 15};
        case ast::binary_operator_token_t::BITWISE_XOR:
            return {14, 13};
        case ast::binary_operator_token_t::BITWISE_OR:
            return {12, 11};
        case ast::binary_operator_token_t::LOGICAL_AND:
            return {10, 9};
        case ast::binary_operator_token_t::LOGICAL_OR:
            return {8, 7};
        case ast::binary_operator_token_t::ASSIGNMENT:
            return {3, 4}; // right-to-left
        case ast::binary_operator_token_t::COMMA:
            return {2, 1}; // left-to-right
    }
    throw std::runtime_error("Invalid infix token.");
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
    throw std::runtime_error("Invalid compound assignment token.");
}
std::pair<ast::precedence_t, ast::precedence_t> ternary_binding_power() {
    return {5, 6}; // right-to-left
}
std::string get_identifier_from_expression(const ast::expression_t& expr) {
    return std::visit(overloaded{
        [](const ast::var_name_t& var) -> std::string {
            return std::string(var);
        },
        [](const auto&) -> std::string {
            throw std::runtime_error("Expression is not an identifier.");
            return std::string("");
        }
    }, expr.expr);
}
ast::expression_t parse_expression(parser_t& parser, const ast::precedence_t precedence) {
    auto lhs = parse_prefix_expression(parser);

    for(;;) {
        if(parser.peek_token().token_type == token_type_t::EOF_TOK) {
            break;
        }

        if(parser.peek_token().token_type == token_type_t::LEFT_PAREN) {
            parser.advance_token();
            std::vector<ast::expression_t> args;
            while(parser.peek_token().token_type != token_type_t::RIGHT_PAREN) {
                args.push_back(parse_expression(parser, 3)); // accept all expressions as arguments except for comma operator, so pass precedence of assignment operator lhs
                if(parser.peek_token().token_type != token_type_t::COMMA) {
                    break;
                }
                parser.advance_token();
            }
            parser.expect_token(token_type_t::RIGHT_PAREN, "Expected `)` in function call.");
            lhs = {std::make_shared<ast::function_call_t>(ast::function_call_t{get_identifier_from_expression(lhs), std::move(args)}), std::nullopt};
            continue;
        }

        if(is_postfix_op(parser.peek_token())) {
            auto op = parse_postfix_op(parser.peek_token());
            auto post_bp = postfix_binding_power(op);
            if(post_bp < precedence) {
                break;
            }
            parser.advance_token();
            lhs = {make_postfix_op(op, std::move(lhs)), std::nullopt};
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
            lhs = {make_infix_op(op, std::move(lhs), std::move(rhs)), std::nullopt};
            continue;
        }

        if(is_compound_assignment_op(parser.peek_token())) {
            auto op = get_op_from_compound_assignment_op(parser.peek_token());
            parser.advance_token();
            lhs = {make_infix_op(ast::binary_operator_token_t::ASSIGNMENT, {validate_lvalue_expression_exp(lhs), std::nullopt}, {make_infix_op(op, {validate_lvalue_expression_exp(lhs), std::nullopt}, parse_expression(parser, precedence)), std::nullopt}), std::nullopt};
            continue;
        }

        if(parser.peek_token().token_type == token_type_t::QUESTION_MARK) {
            // TODO: double check that the associativity and precedence are correct for ternary expressions
            auto [r_bp, l_bp] = ternary_binding_power();
            if(l_bp < precedence) {
                break;
            }
            parser.advance_token();
            auto if_true = parse_expression(parser, precedence);
            parser.expect_token(token_type_t::COLON, "Expected `:` in ternary expression.");
            auto if_false = parse_expression(parser, 28); // 28 is the highest level of precedence, we use this so it greedily parses `a < b ? a = 1 : a = 2` as `(a < b ? a = 1 : a) = 2` instead of `(a < b ? a = 1 : a = 2)`.
            lhs = {std::make_shared<ast::ternary_expression_t>(ast::ternary_expression_t{std::move(lhs), std::move(if_true), std::move(if_false)}), std::nullopt};
            continue;
        }

        break;
    }

    return lhs;
}
ast::expression_t parse_expression(parser_t& parser) {
    return parse_expression(parser, 0u);
}

ast::return_statement_t parse_return_statement(parser_t& parser) {
    parser.expect_token(token_type_t::RETURN_KEYWORD, "Expected `return` keyword in statement.");

    auto expression = parse_expression(parser);

    parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in statement.");

    return ast::return_statement_t{std::move(expression)};
}
ast::expression_statement_t parse_expression_statement(parser_t& parser) {
    if(parser.peek_token().token_type == token_type_t::SEMICOLON) {
        parser.advance_token();
        return ast::expression_statement_t{std::nullopt}; // null statement, i.e. `;`
    }

    auto expression = parse_expression(parser);

    parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in statement.");

    return ast::expression_statement_t{std::move(expression)};
}
ast::if_statement_t parse_if_statement(parser_t& parser) {
    parser.expect_token(token_type_t::IF_KEYWORD, "Expected `if` keyword in statement.");

    parser.expect_token(token_type_t::LEFT_PAREN, "Expected `(` in statement.");

    auto if_exp = parse_expression(parser);

    parser.expect_token(token_type_t::RIGHT_PAREN, "Expected `)` in statement.");

    ast::statement_t if_body;
    if(parser.peek_token().token_type == token_type_t::LEFT_CURLY) {
        if_body = std::make_shared<ast::compound_statement_t>(parse_compound_statement(parser));
    } else {
        if_body = parse_statement(parser);
    }

    if(parser.peek_token().token_type != token_type_t::ELSE_KEYWORD) {
        return ast::if_statement_t{std::move(if_exp), std::move(if_body), std::nullopt};
    }

    parser.advance_token(); // consume `else` keyword

    if(parser.peek_token().token_type == token_type_t::LEFT_CURLY) {
        return ast::if_statement_t{std::move(if_exp), std::move(if_body), std::make_shared<ast::compound_statement_t>(parse_compound_statement(parser))};
    } else {
        return ast::if_statement_t{std::move(if_exp), std::move(if_body), parse_statement(parser)};
    }
}
ast::statement_t parse_statement(parser_t& parser) {
    if(parser.is_eof()) {
        throw std::runtime_error("Unexpected end of file.");
    }

    if(parser.peek_token().token_type == token_type_t::RETURN_KEYWORD) {
        return parse_return_statement(parser);
    } else if(parser.peek_token().token_type == token_type_t::IF_KEYWORD) {
        return std::make_shared<ast::if_statement_t>(parse_if_statement(parser));
    } else if(parser.peek_token().token_type == token_type_t::LEFT_CURLY) {
        return std::make_shared<ast::compound_statement_t>(parse_compound_statement(parser));
    }
    return parse_expression_statement(parser);
}

ast::declaration_t parse_declaration(parser_t& parser) {
    const auto type_token = parser.advance_token();
    if(type_token.token_type != token_type_t::INT_KEYWORD && type_token.token_type != token_type_t::DOUBLE_KEYWORD && type_token.token_type != token_type_t::CHAR_KEYWORD) {
        throw std::runtime_error("expected `int`, `double`, or `char` keyword");
    }

    auto identifier_token = parser.advance_token();
    if(identifier_token.token_type != token_type_t::IDENTIFIER) {
        throw std::runtime_error("Expected identifier.");
    }

    if(parser.peek_token().token_type != token_type_t::EQUALS) {
        auto ret = ast::declaration_t{create_type_name_from_token(type_token), ast::var_name_t(identifier_token.token_text), std::nullopt};

        parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in statement.");

        return ret;
    }

    parser.advance_token(); // consume `=` token

    auto ret = ast::declaration_t{create_type_name_from_token(type_token), ast::var_name_t(identifier_token.token_text), parse_expression(parser)};

    parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in statement.");

    return ret;
}
ast::compound_statement_t parse_compound_statement(parser_t& parser) {
    parser.expect_token(token_type_t::LEFT_CURLY, "Expected `{` in statement.");

    ast::compound_statement_t ret{};

    while(parser.peek_token().token_type != token_type_t::RIGHT_CURLY) {
        if(parser.is_eof()) {
            throw std::runtime_error("Unexpected end of file. Unterminated compound statement.");
        }

        if(parser.peek_token().token_type == token_type_t::INT_KEYWORD || parser.peek_token().token_type == token_type_t::DOUBLE_KEYWORD || parser.peek_token().token_type == token_type_t::CHAR_KEYWORD) {
            ret.stmts.push_back(parse_declaration(parser));
        } else {
            ret.stmts.push_back(parse_statement(parser));
        }
    }
    parser.advance_token(); // consume `}` token

    return ret;
}
static std::vector<std::pair<ast::type_name_t, std::optional<ast::var_name_t>>> parse_function_definition_parameter_list(parser_t& parser) {
    std::vector<std::pair<ast::type_name_t, std::optional<ast::var_name_t>>> param_list;
    for(;;) {
        std::vector<token_t> current_param;
        while(parser.peek_token().token_type != token_type_t::COMMA && parser.peek_token().token_type != token_type_t::RIGHT_PAREN) {
            if(parser.is_eof()) {
                throw std::runtime_error("Unexpected end of file.");
            }
            current_param.push_back(parser.advance_token());
        }

        if(current_param.size() == 0) {
            break; // empty param list. e.g. `int main();`
        } else if(current_param.size() == 1) {
            // TODO: Only integer types are supported in parameter lists currently
            if(current_param[0].token_type != token_type_t::INT_KEYWORD) {
                throw std::runtime_error("Expected identifier name (type name) in function declaration.");
            }
            param_list.push_back({create_type_name_from_token(current_param[0]), std::nullopt});
        } else if(current_param.size() == 2) {
            // TODO: Only integer types are supported in parameter lists currently
            if(current_param[0].token_type != token_type_t::INT_KEYWORD) {
                throw std::runtime_error("Expected identifier name (type name) in function declaration.");
            }
            if(current_param[1].token_type != token_type_t::IDENTIFIER) {
                throw std::runtime_error("Expected identifier name (variable name) in function declaration.");
            }
            param_list.push_back({create_type_name_from_token(current_param[0]), std::make_optional(ast::var_name_t{current_param[1].token_text})});
        } else {
            throw std::runtime_error("Unexpected token in function definition param list.");
        }

        if(parser.peek_token().token_type == token_type_t::COMMA) {
            parser.advance_token(); // consume `,`
        } else if(parser.peek_token().token_type == token_type_t::RIGHT_PAREN) {
            break;
        } else {
            throw std::logic_error("Unexpected token in function definition param list."); // should be impossible to trigger
        }
    }
    return param_list;
}
static std::vector<ast::type_name_t> parse_function_declaration_parameter_list(std::vector<std::pair<ast::type_name_t, std::optional<ast::var_name_t>>> list_with_names) {
    std::vector<ast::type_name_t> ret_type_list;
    for(const auto& param : list_with_names) {
        ret_type_list.push_back(std::move(param.first));
    }
    return ret_type_list;
}
std::variant<ast::function_declaration_t, ast::function_definition_t, ast::global_variable_declaration_t> parse_top_level_declaration(parser_t& parser) {
    const auto type_token = parser.advance_token();
    if(type_token.token_type != token_type_t::INT_KEYWORD) {
        throw std::runtime_error("Expected `int` keyword in function declaration.");
    }

    const auto name_token = parser.advance_token();
    if(name_token.token_type != token_type_t::IDENTIFIER) {
        throw std::runtime_error("Expected identifier name in function declaration.");
    }

    if(parser.peek_token().token_type == token_type_t::LEFT_PAREN) {
        parser.advance_token();
        auto param_list = parse_function_definition_parameter_list(parser);
        parser.expect_token(token_type_t::RIGHT_PAREN, "Expected `)` in function declaration.");

        if(parser.peek_token().token_type == token_type_t::SEMICOLON) {
            parser.advance_token();

            return ast::function_declaration_t{ create_type_name_from_token(type_token), ast::func_name_t(name_token.token_text), parse_function_declaration_parameter_list(param_list) };
        }

        ast::compound_statement_t statements = parse_compound_statement(parser);

        if(name_token.token_text == "main") {
            constexpr int DEFAULT_RETURN_VALUE = 0;
            // use `has_return_statement` instead of `is_return_statement` because we don't need to emit a return statement if there already is one,
            //  even if there is unreachable code after the already existing return statement.
            if(statements.stmts.size() == 0 || !has_return_statement(statements)) {
                statements.stmts.push_back(ast::return_statement_t { ast::expression_t{ ast::constant_t{DEFAULT_RETURN_VALUE}, ast::type_name_t{ast::type_category_t::INT, "int", sizeof(std::uint64_t), sizeof(std::uint64_t)} } } );
            }
        }

        return ast::function_definition_t{ create_type_name_from_token(type_token), ast::func_name_t(name_token.token_text), std::move(param_list), std::move(statements) };
    } else {
        if(parser.peek_token().token_type != token_type_t::EQUALS) {
            auto ret = ast::global_variable_declaration_t{create_type_name_from_token(type_token), ast::var_name_t(name_token.token_text), std::nullopt};
        
            parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in global variable declaration.");

            return ret;
        }

        parser.advance_token(); // consume `=` token

        auto ret = ast::global_variable_declaration_t{create_type_name_from_token(type_token), ast::var_name_t(name_token.token_text), parse_expression(parser)};

        parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in global variable declaration.");

        return ret;
    }
}

ast::program_t parse(parser_t& parser) {
    std::vector<std::variant<ast::function_declaration_t, ast::function_definition_t, ast::global_variable_declaration_t>> top_level_declarations;
    while(parser.peek_token().token_type != token_type_t::EOF_TOK) {
        top_level_declarations.push_back(parse_top_level_declaration(parser));
    }
    return ast::program_t { std::move(top_level_declarations) };
}
