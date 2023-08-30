#include "parser.hpp"


void validate_type_name(const ast::type_t& expected, const ast::type_t& actual, const std::string& error_message) {
    if(expected.type_category != actual.type_category) { // optimization to avoid having to do string comparisons for built-in types
        if(expected.type_name != actual.type_name) {
            throw std::runtime_error(error_message);
        }
    }
}
void validate_variable(const validation_t& validation, const ast::var_name_t& var_name) {
    if(!validation.variable_lookup.contains_in_accessible_scopes(var_name) && !utils::contains(validation.global_variable_declarations, var_name) && !utils::contains(validation.global_variable_definitions, var_name)) {
        throw std::runtime_error("Variable [" + var_name + "] is not declared in currently accessible scopes.");
    }
}
void validate_variable_declaration(validation_t& validation, ast::declaration_t& declaration) {
    if(validation.variable_lookup.contains_in_lowest_scope(declaration.var_name)) {
        throw std::runtime_error("Variable " + declaration.var_name + " already declared in current scope.");
    }

    validation.variable_lookup.add_new_variable_in_current_scope(declaration.var_name, declaration.type_name);

    if(declaration.value.has_value()) {
        validate_expression(validation, declaration.value.value());
    }
}

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
// declared in frontend/ast/ast.hpp
static ast::type_category_t get_type_category_from_token_type(token_type_t token_type) {
    switch(token_type) {
        case token_type_t::CHAR_CONSTANT:
        case token_type_t::CHAR_KEYWORD:
        case token_type_t::SIGNED_CHAR_KEYWORD:
        case token_type_t::UNSIGNED_CHAR_KEYWORD:
        case token_type_t::SHORT_KEYWORD:
        case token_type_t::UNSIGNED_SHORT_KEYWORD:
        case token_type_t::INT_CONSTANT:
        case token_type_t::INT_KEYWORD:
        case token_type_t::UNSIGNED_INT_CONSTANT:
        case token_type_t::UNSIGNED_INT_KEYWORD:
        case token_type_t::LONG_CONSTANT:
        case token_type_t::LONG_KEYWORD:
        case token_type_t::UNSIGNED_LONG_CONSTANT:
        case token_type_t::UNSIGNED_LONG_KEYWORD:
        case token_type_t::LONG_LONG_CONSTANT:
        case token_type_t::LONG_LONG_KEYWORD:
        case token_type_t::UNSIGNED_LONG_LONG_CONSTANT:
        case token_type_t::UNSIGNED_LONG_LONG_KEYWORD:
            return ast::type_category_t::INT;
        case token_type_t::FLOAT_CONSTANT:
        case token_type_t::FLOAT_KEYWORD:
        case token_type_t::DOUBLE_CONSTANT:
        case token_type_t::DOUBLE_KEYWORD:
        case token_type_t::LONG_DOUBLE_CONSTANT:
        case token_type_t::LONG_DOUBLE_KEYWORD:
            return ast::type_category_t::DOUBLE;
    }
    throw std::runtime_error("Invalid/Unsupported type: [" + std::to_string(static_cast<std::uint32_t>(token_type)) + std::string("]"));
}
static std::string get_type_name_from_token(token_t token) {
    // TODO: Write utility function to remove code duplication in these if statement conditionals
    if(token.token_type == token_type_t::CHAR_CONSTANT) {
        return "char";
    } else if(token.token_type == token_type_t::INT_CONSTANT) {
        return "int";
    } else if(token.token_type == token_type_t::UNSIGNED_INT_CONSTANT) {
        return "unsigned int";
    } else if(token.token_type == token_type_t::LONG_CONSTANT) {
        return "long";
    } else if(token.token_type == token_type_t::UNSIGNED_LONG_CONSTANT) {
        return "unsigned long";
    } else if(token.token_type == token_type_t::LONG_LONG_CONSTANT) {
        return "long long";
    } else if(token.token_type == token_type_t::UNSIGNED_LONG_LONG_CONSTANT) {
        return "unsigned long long";
    } else if(token.token_type == token_type_t::FLOAT_CONSTANT) {
        return "float";
    } else if(token.token_type == token_type_t::DOUBLE_CONSTANT) {
        return "double";
    } else if(token.token_type == token_type_t::LONG_DOUBLE_CONSTANT) {
        return "long double";
    } else {
        return std::string(token.token_text);
    }
}
static std::size_t get_size_from_type(const std::string& type_name) {
    if(type_name == "char" || type_name == "signed char" || type_name == "unsigned char") {
        return sizeof(char);
    } else if(type_name == "short" || type_name == "unsigned short") {
        return sizeof(std::uint16_t);
    } else if(type_name == "int" || type_name == "unsigned int") {
        return sizeof(std::uint32_t);
    } else if(type_name == "long" || type_name == "unsigned long") {
        return sizeof(std::uint64_t);
    } else if(type_name == "long long" || type_name == "unsigned long long") {
        return sizeof(std::uint64_t);
    } else if(type_name == "float") {
        return sizeof(float);
    } else if(type_name == "double") {
        return sizeof(double);
    } else if(type_name == "long double") {
        return sizeof(long double);
    } else {
        throw std::runtime_error("Unsupported type: [" + type_name + std::string("]"));
    }
}
static std::size_t get_alignment_from_type(const std::string& type_name) {
    // `get_alignment_from_type()` and `get_size_from_type()` will diverge once we implement structs and other non-primitive types
    if(type_name == "char" || type_name == "signed char" || type_name == "unsigned char") {
        return alignof(char);
    } else if(type_name == "short" || type_name == "unsigned short") {
        return alignof(std::uint16_t);
    } else if(type_name == "int" || type_name == "unsigned int") {
        return alignof(std::uint32_t);
    } else if(type_name == "long" || type_name == "unsigned long") {
        return alignof(std::uint64_t);
    } else if(type_name == "long long" || type_name == "unsigned long long") {
        return alignof(std::uint64_t);
    } else if(type_name == "float") {
        return alignof(float);
    } else if(type_name == "double") {
        return alignof(double);
    } else if(type_name == "long double") {
        return alignof(long double);
    } else {
        throw std::runtime_error("Unsupported type: [" + type_name + std::string("]"));
    }
}
ast::type_t create_type_name_from_token(const token_t& token) {
    std::string type_name = get_type_name_from_token(token);
    const std::size_t type_size = get_size_from_type(type_name);
    const std::size_t alignment_size = get_alignment_from_type(type_name);
    return make_primitive_type_t(get_type_category_from_token_type(token.token_type), std::move(type_name), type_size, alignment_size);
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
ast::constant_t parse_constant(parser_t& parser, const std::size_t suffix_size = 0u) {
    auto next = parser.advance_token();
    if(!is_constant(next)) {
        throw std::runtime_error("Invalid constant: [" + std::to_string(static_cast<std::uint32_t>(next.token_type)) + std::string("]"));
    }

    T result{};
    utils::str_to_T(next.token_text, result, suffix_size);
    return ast::constant_t { result };
}
ast::expression_t parse_char_constant(parser_t& parser) {
    return ast::expression_t { ast::constant_t { parser.advance_token().token_text[1] }, make_primitive_type_t(ast::type_category_t::INT, "char", sizeof(char), sizeof(char)) };
}
ast::expression_t parse_int_constant(parser_t& parser) {
    return ast::expression_t { parse_constant<int>(parser), make_primitive_type_t(ast::type_category_t::INT, "int", sizeof(std::int32_t), alignof(std::int32_t)) };
}
ast::expression_t parse_unsigned_int_constant(parser_t& parser) {
    return ast::expression_t { parse_constant<unsigned int>(parser, 1), make_primitive_type_t(ast::type_category_t::UNSIGNED_INT, "unsigned int", sizeof(std::int32_t), alignof(std::int32_t)) };
}
ast::expression_t parse_long_constant(parser_t& parser) {
    return ast::expression_t { parse_constant<long>(parser, 1), make_primitive_type_t(ast::type_category_t::INT, "long", sizeof(std::int64_t), alignof(std::int64_t)) };
}
ast::expression_t parse_unsigned_long_constant(parser_t& parser) {
    return ast::expression_t { parse_constant<unsigned long>(parser, 2), make_primitive_type_t(ast::type_category_t::UNSIGNED_INT, "unsigned long", sizeof(std::uint64_t), alignof(std::uint64_t)) };
}
ast::expression_t parse_long_long_constant(parser_t& parser) {
    return ast::expression_t { parse_constant<long long>(parser, 2), make_primitive_type_t(ast::type_category_t::INT, "long long", sizeof(std::int64_t), alignof(std::int64_t)) };
}
ast::expression_t parse_unsigned_long_long_constant(parser_t& parser) {
    return ast::expression_t { parse_constant<unsigned long long>(parser, 3), make_primitive_type_t(ast::type_category_t::UNSIGNED_INT, "unsigned long long", sizeof(std::uint64_t), alignof(std::uint64_t)) };
}
ast::expression_t parse_float_constant(parser_t& parser) {
    return ast::expression_t { parse_constant<float>(parser, 1), make_primitive_type_t(ast::type_category_t::DOUBLE, "float", sizeof(float), alignof(float)) };
}
ast::expression_t parse_double_constant(parser_t& parser) {
    return ast::expression_t { parse_constant<double>(parser), make_primitive_type_t(ast::type_category_t::DOUBLE, "double", sizeof(double), alignof(double)) };
}
ast::expression_t parse_long_double_constant(parser_t& parser) {
    return ast::expression_t { parse_constant<long double>(parser, 1), make_primitive_type_t(ast::type_category_t::DOUBLE, "long double", sizeof(long double), alignof(long double)) };
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
    std::cout << "Non-Error Prefix Expression: " << static_cast<std::uint32_t>(parser.peek_token().token_type) << ": " << parser.peek_token().token_text << std::endl;
    switch(parser.peek_token().token_type) {
        case token_type_t::IDENTIFIER:
            return {parse_var_name(parser), std::nullopt};
        case token_type_t::CHAR_CONSTANT:
            return parse_char_constant(parser);
        case token_type_t::INT_CONSTANT:
            return parse_int_constant(parser);
        case token_type_t::UNSIGNED_INT_CONSTANT:
            return parse_unsigned_int_constant(parser);
        case token_type_t::LONG_CONSTANT:
            return parse_long_constant(parser);
        case token_type_t::UNSIGNED_LONG_CONSTANT:
            return parse_unsigned_long_constant(parser);
        case token_type_t::LONG_LONG_CONSTANT:
            return parse_long_long_constant(parser);
        case token_type_t::UNSIGNED_LONG_LONG_CONSTANT:
            return parse_unsigned_long_long_constant(parser);
        case token_type_t::FLOAT_CONSTANT:
            return parse_float_constant(parser);
        case token_type_t::DOUBLE_CONSTANT:
            return parse_double_constant(parser);
        case token_type_t::LONG_DOUBLE_CONSTANT:
            return parse_long_double_constant(parser);
        case token_type_t::LEFT_PAREN:
            return {parse_grouping(parser), std::nullopt};
        default:
            if(is_prefix_op(parser.peek_token())) {
                auto op = parse_prefix_op(parser.advance_token());
                auto r_bp = prefix_binding_power(op);
                auto rhs = parse_expression(parser, r_bp);
                return {make_prefix_op(op, std::move(rhs)), std::nullopt};
            }
            std::cout << static_cast<std::uint32_t>(parser.peek_token().token_type) << ": " << parser.peek_token().token_text << std::endl;
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
// TODO: maybe pull out and make globally accessible
static bool is_keyword_a_type(token_type_t token_type) {
    switch(token_type) {
        case token_type_t::CHAR_KEYWORD:
        case token_type_t::SIGNED_CHAR_KEYWORD:
        case token_type_t::UNSIGNED_CHAR_KEYWORD:
        case token_type_t::SHORT_KEYWORD:
        case token_type_t::UNSIGNED_SHORT_KEYWORD:
        case token_type_t::INT_KEYWORD:
        case token_type_t::UNSIGNED_INT_KEYWORD:
        case token_type_t::LONG_KEYWORD:
        case token_type_t::UNSIGNED_LONG_KEYWORD:
        case token_type_t::LONG_LONG_KEYWORD:
        case token_type_t::UNSIGNED_LONG_LONG_KEYWORD:
        case token_type_t::FLOAT_KEYWORD:
        case token_type_t::DOUBLE_KEYWORD:
        case token_type_t::LONG_DOUBLE_KEYWORD:
            return true;
    }
    return false;
}
ast::declaration_t parse_declaration(parser_t& parser) {
    const auto type_token = parser.advance_token();
    if(!is_keyword_a_type(type_token.token_type)) {
        std::cout << static_cast<std::uint32_t>(type_token.token_type) << ": " << type_token.token_text << std::endl;
        throw std::runtime_error("Expected type name in declaration.");
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

        if(is_keyword_a_type(parser.peek_token().token_type)) {
            ret.stmts.push_back(parse_declaration(parser));
        } else {
            ret.stmts.push_back(parse_statement(parser));
        }
    }
    parser.advance_token(); // consume `}` token

    return ret;
}
static std::vector<std::pair<ast::type_t, std::optional<ast::var_name_t>>> parse_function_definition_parameter_list(parser_t& parser) {
    std::vector<std::pair<ast::type_t, std::optional<ast::var_name_t>>> param_list;
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
static std::vector<ast::type_t> parse_function_declaration_parameter_list(std::vector<std::pair<ast::type_t, std::optional<ast::var_name_t>>> list_with_names) {
    std::vector<ast::type_t> ret_type_list;
    for(const auto& param : list_with_names) {
        ret_type_list.push_back(std::move(param.first));
    }
    return ret_type_list;
}
ast::type_t parse_type_declaration(parser_t& parser) {
    // TODO: IMPLEMENT ASAP!!!
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
                statements.stmts.push_back(ast::return_statement_t { ast::expression_t{ ast::constant_t{DEFAULT_RETURN_VALUE}, make_primitive_type_t(ast::type_category_t::INT, "int", sizeof(std::int32_t), alignof(std::int32_t)) } } );
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
    return ast::program_t { parser.symbol_info.type_table, std::move(top_level_declarations) };
}












void validate_function_declaration(validation_t& validation, const ast::function_declaration_t& function_declaration) {
    if(utils::contains(validation.global_variable_declarations, function_declaration.function_name) || utils::contains(validation.global_variable_definitions, function_declaration.function_name)) {
        throw "Function [" + function_declaration.function_name + "] is already declared as a global variable.";
    }

    if(utils::contains(validation.function_declarations_lookup, function_declaration.function_name)) {
        const auto existing_function_declaration = validation.function_declarations_lookup.at(function_declaration.function_name);
        validate_type_name(function_declaration.return_type, existing_function_declaration.return_type, "Function [" + function_declaration.function_name + "] return type mismatch.");
        if(function_declaration.params.size() != existing_function_declaration.params.size()) {
            throw std::runtime_error("Function [" + function_declaration.function_name + "] param count mismatch.");
        }
        for(std::uint32_t i = 0u; i < function_declaration.params.size(); ++i) {
            validate_type_name(function_declaration.params[i], existing_function_declaration.params[i], "Function [" + function_declaration.function_name + "] param type mismatch.");
        }
    } else if(utils::contains(validation.function_definitions_lookup, function_declaration.function_name)) {
        const auto existing_function_definition = validation.function_definitions_lookup.at(function_declaration.function_name);
        validate_type_name(function_declaration.return_type, existing_function_definition.return_type, "Function [" + function_declaration.function_name + "] return type mismatch.");
        if(function_declaration.params.size() != existing_function_definition.params.size()) {
            throw std::runtime_error("Function [" + function_declaration.function_name + "] param count mismatch.");
        }
        for(std::uint32_t i = 0u; i < function_declaration.params.size(); ++i) {
            validate_type_name(function_declaration.params[i], existing_function_definition.params[i].first, "Function [" + function_declaration.function_name + "] param type mismatch.");
        }
    } else {
        validation.function_declarations_lookup.insert({function_declaration.function_name, function_declaration});
    }
}
void validate_expression(validation_t& validation, ast::expression_t& expression) {
    std::visit(overloaded{
        [&validation, &expression](const std::shared_ptr<ast::grouping_t>& grouping_exp) {
            validate_expression(validation, grouping_exp->expr);
        },
        [&validation, &expression](const std::shared_ptr<ast::unary_expression_t>& unary_exp) {
            validate_expression(validation, unary_exp->exp);
        },
        [&validation, &expression](const std::shared_ptr<ast::binary_expression_t>& bin_exp) {
            validate_expression(validation, bin_exp->left);
            validate_expression(validation, bin_exp->right);
        },
        [&validation, &expression](const std::shared_ptr<ast::ternary_expression_t>& ternary) {
            validate_expression(validation, ternary->condition);
            validate_expression(validation, ternary->if_true);
            validate_expression(validation, ternary->if_false);
        },
        [&validation, &expression](const std::shared_ptr<ast::function_call_t>& func_call) {
            validate_function_call(validation, *func_call);
            add_type_to_function_call(validation, *func_call, expression.type); // will set `expression.type` to the return type of the accompanying function
        },
        [&validation](const ast::constant_t& expression) {
            // TODO: consider adding types to constant expressions here instead of earlier in the initial parsing pass
        },
        [&validation, &expression](const ast::var_name_t& var_name) {
            validate_variable(validation, var_name);
            add_type_to_variable(validation, var_name, expression.type); // will set `expression.type` to the type of the variable from its declaration
        },
        [](const std::shared_ptr<ast::convert_t>& convert_exp) {
            throw std::runtime_error("User casts not yet supported.");
        }
    }, expression.expr);
}
void validate_statement(validation_t& validation, ast::statement_t& statement) {
    std::visit(overloaded{
        [&validation](ast::return_statement_t& statement) {
            validate_expression(validation, statement.expr);
        },
        [&validation](ast::expression_statement_t& statement) {
            if(statement.expr.has_value()) {
                validate_expression(validation, statement.expr.value());
            }
        },
        [&validation](const std::shared_ptr<ast::if_statement_t>& statement) {
            validate_expression(validation, statement->if_exp);
            validate_statement(validation, statement->if_body);
            if(statement->else_body.has_value()) {
                validate_statement(validation, statement->else_body.value());
            }
        },
        [&validation](const std::shared_ptr<ast::compound_statement_t>& statement) {
            validate_compound_statement(validation, *statement);
        }
    }, statement);
}
void validate_compound_statement(validation_t& validation, ast::compound_statement_t& compound_statement, bool is_function_block) {
    if(!is_function_block) {
        validation.variable_lookup.create_new_scope();
    }
    for(auto& e : compound_statement.stmts) {
        std::visit(overloaded{
            [&validation](ast::statement_t& statement) {
                validate_statement(validation, statement);
            },
            [&validation](ast::declaration_t& declaration) {
                validate_variable_declaration(validation, declaration);
            },
        }, e);
    }
    if(!is_function_block) {
        validation.variable_lookup.destroy_current_scope();
    }
}
void validate_function_definition(validation_t& validation, const ast::function_definition_t& function_definition) {
    if(utils::contains(validation.global_variable_declarations, function_definition.function_name) || utils::contains(validation.global_variable_definitions, function_definition.function_name)) {
        throw "Function [" + function_definition.function_name + "] is already declared as a global variable.";
    }

    if(utils::contains(validation.function_definitions_lookup, function_definition.function_name)) {
        throw std::runtime_error("Function [" + function_definition.function_name + "] already defined.");
    }
    if(utils::contains(validation.function_declarations_lookup, function_definition.function_name)) {
        const auto existing_function_declaration = validation.function_declarations_lookup.at(function_definition.function_name);
        validate_type_name(function_definition.return_type, existing_function_declaration.return_type, "Function [" + function_definition.function_name + "] return type mismatch.");
        if(function_definition.params.size() != existing_function_declaration.params.size()) {
            throw std::runtime_error("Function [" + function_definition.function_name + "] param count mismatch.");
        }
        for(std::uint32_t i = 0u; i < function_definition.params.size(); ++i) {
            validate_type_name(function_definition.params[i].first, existing_function_declaration.params[i], "Function [" + function_definition.function_name + "] param type mismatch.");
        }
    }
    validation.function_definitions_lookup.insert({function_definition.function_name, function_definition});
    validation.variable_lookup.create_new_scope();
    for(const auto param : function_definition.params) {
        if(param.second.has_value()) {
            validation.variable_lookup.add_new_variable_in_current_scope(param.second.value(), param.first);
        }
    }
    auto& validation_function_definition = validation.function_definitions_lookup.at(function_definition.function_name);
    validate_compound_statement(validation, validation_function_definition.statements, true);
    validation.variable_lookup.destroy_current_scope();
}
void validate_function_call(validation_t& validation, const ast::function_call_t& function_call) {
    if(utils::contains(validation.function_declarations_lookup, function_call.function_name)) {
        const auto declaration = validation.function_declarations_lookup.at(function_call.function_name);
        if(declaration.params.size() != function_call.params.size()) {
            throw std::runtime_error("Function [" + function_call.function_name + "] param count mismatch.");
        }
        // TODO: type check parameters to function call
    } else if(utils::contains(validation.function_definitions_lookup, function_call.function_name)) {
        const auto definition = validation.function_definitions_lookup.at(function_call.function_name);
        if(definition.params.size() != function_call.params.size()) {
            throw std::runtime_error("Function [" + function_call.function_name + "] param count mismatch.");
        }
        // TODO: type check parameters to function call
    } else {
        throw std::runtime_error("Function [" + function_call.function_name + "] not declared or defined.");
    }
}
void validate_compile_time_expression(validation_t& validation, const ast::expression_t& expression) {
    std::visit(overloaded{
        [&validation](const std::shared_ptr<ast::grouping_t>& expression) {
            validate_compile_time_expression(validation, expression->expr);
        },
        [&validation](const std::shared_ptr<ast::unary_expression_t>& expression) {
            if(expression->op == ast::unary_operator_token_t::PLUS_PLUS || expression->op == ast::unary_operator_token_t::MINUS_MINUS) {
                throw std::runtime_error("`++` and `--` not supported in compile time expressions.");
            }
            validate_compile_time_expression(validation, expression->exp);
        },
        [&validation](const std::shared_ptr<ast::binary_expression_t>& expression) {
            if(expression->op == ast::binary_operator_token_t::ASSIGNMENT) {
                throw std::runtime_error("Assignment not supported in compile time expressions.");
            }
            validate_compile_time_expression(validation, expression->left);
            validate_compile_time_expression(validation, expression->right);
        },
        [&validation](const std::shared_ptr<ast::ternary_expression_t>& expression) {
            validate_compile_time_expression(validation, expression->condition);
            validate_compile_time_expression(validation, expression->if_true);
            validate_compile_time_expression(validation, expression->if_false);
        },
        [](const std::shared_ptr<ast::function_call_t>& expression) {
            throw std::runtime_error("Function calls not supported in compile time expressions.");
        },
        [](const ast::constant_t& expression) {
            // totally fine, no need to go further as this is a terminal node of the AST
        },
        [](const ast::var_name_t& expression) {
            throw std::runtime_error("Variables not supported in compile time expressions.");
            // TODO: Maybe support referencing other global variables???? Check the C standard to see what is considered valid.
        },
        [&validation](const std::shared_ptr<ast::convert_t>& expression) {
            validate_compile_time_expression(validation, expression->expr);
        }
    }, expression.expr);
}
void validate_global_variable_declaration(validation_t& validation, const ast::global_variable_declaration_t& declaration) {
    if(utils::contains(validation.function_declarations_lookup, declaration.var_name) || utils::contains(validation.function_definitions_lookup, declaration.var_name)) {
        throw std::runtime_error("Global variable [" + declaration.var_name + "] already declared as a function.");
    }

    if(declaration.value.has_value()) {
        if(utils::contains(validation.global_variable_definitions, declaration.var_name)) {
            throw std::runtime_error("Global variable [" + declaration.var_name + "] already defined.");
        }

        if(utils::contains(validation.global_variable_declarations, declaration.var_name)) {
            auto existing_declaration = validation.global_variable_declarations.at(declaration.var_name);

            validate_type_name(existing_declaration.type_name, declaration.type_name, "Mismatched global variable type.");
        }

        validate_compile_time_expression(validation, declaration.value.value());

        validation.global_variable_definitions.insert({declaration.var_name, declaration}); // idempotent operation
    } else { // this part of the C standard diverges from C++ since you are not required to preface declarations with `extern` in C, but you are in C++.
        if(utils::contains(validation.global_variable_declarations, declaration.var_name)) {
            auto existing_declaration = validation.global_variable_declarations.at(declaration.var_name);

            validate_type_name(existing_declaration.type_name, declaration.type_name, "Mismatched global variable type.");
        }
        if(utils::contains(validation.global_variable_definitions, declaration.var_name)) {
            auto existing_definition = validation.global_variable_definitions.at(declaration.var_name);

            validate_type_name(existing_definition.type_name, declaration.type_name, "Mismatched global variable type.");
        }

        validation.global_variable_declarations.insert({declaration.var_name, declaration}); // idempotent operation
    }
}
ast::validated_program_t validate_ast(const ast::program_t& program) {
    validation_t validation;
    for(const auto& top_level_decl : program.top_level_declarations) {
        std::visit(overloaded{
            [&validation](const ast::function_declaration_t& function_decl) {
                validate_function_declaration(validation, function_decl);
            },
            [&validation](const ast::function_definition_t& function_def) {
                validate_function_definition(validation, function_def);
            },
            [&validation](const ast::global_variable_declaration_t& declaration) {
                validate_global_variable_declaration(validation, declaration);
            }
        }, top_level_decl);
    }

    ast::validated_program_t validated_program{};
    for(const auto& it : validation.function_definitions_lookup) {
        validated_program.top_level_declarations.push_back(it.second);
    }
    for(const auto& it : validation.global_variable_definitions) {
        ast::validated_global_variable_definition_t validated_definition{it.second.type_name, it.first, evaluate_expression(it.second.value.value())};

        validated_program.top_level_declarations.push_back(validated_definition);
    }
    for(const auto& it : validation.global_variable_declarations) {
        if(!utils::contains(validation.global_variable_definitions, it.first)) {
            ast::validated_global_variable_definition_t validated_definition{it.second.type_name, it.first, ast::constant_t{0}};

            validated_program.top_level_declarations.push_back(validated_definition);
        }
    }
    return validated_program;
}

