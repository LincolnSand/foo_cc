#include "parser.hpp"


void validate_type_name(const ast::type_t& expected, const ast::type_t& actual, const std::string& error_message) {
    if(expected.type_category != actual.type_category) { // optimization to avoid having to do string comparisons for built-in types
        if(expected.type_name != actual.type_name) {
            throw std::runtime_error(error_message);
        }
    }
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
    parser.expect_token(token_type_t::LEFT_PAREN, "Expected '(' in grouping expression.");
    auto exp = parse_and_validate_expression(parser, 0u);
    if(parser.peek_token().token_type != token_type_t::RIGHT_PAREN) {
        throw std::runtime_error("expected `)`");
    }
    parser.expect_token(token_type_t::RIGHT_PAREN, "Expected ')' in grouping expression.");
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
ast::var_name_t parse_and_validate_variable(parser_t& parser, const token_t name_token) {
    auto name = ast::var_name_t { name_token.token_text };
    if(!parser.symbol_info.variable_lookup.contains_in_accessible_scopes(name) && !utils::contains(parser.symbol_info.global_variable_declarations, name) && !utils::contains(parser.symbol_info.global_variable_definitions, name)) {
        throw std::runtime_error("Variable [" + name + "] is not declared in currently accessible scopes.");
    }
    return name;
}
std::shared_ptr<ast::function_call_t> parse_and_validate_function_call(parser_t& parser, const token_t name_token) {
    parser.expect_token(token_type_t::LEFT_PAREN, "Expected `(` in function call.");

    std::vector<ast::expression_t> args;
    while(parser.peek_token().token_type != token_type_t::RIGHT_PAREN) {
        args.push_back(parse_and_validate_expression(parser, 3)); // accept all expressions as arguments except for comma operator, so pass precedence of assignment operator lhs

        if(parser.peek_token().token_type != token_type_t::COMMA) {
            break;
        }
        parser.advance_token();
    }

    parser.expect_token(token_type_t::RIGHT_PAREN, "Expected `)` in function call.");

    auto function_call = ast::function_call_t{ast::func_name_t(name_token.token_text), std::move(args)};

    if(utils::contains(parser.symbol_info.function_declarations_lookup, function_call.function_name)) {
        const auto declaration = parser.symbol_info.function_declarations_lookup.at(function_call.function_name);
        if(declaration.params.size() != function_call.params.size()) {
            throw std::runtime_error("Function [" + function_call.function_name + "] param count mismatch.");
        }
        // TODO: type check parameters to function call
    } else if(utils::contains(parser.symbol_info.function_definitions_lookup, function_call.function_name)) {
        const auto definition = parser.symbol_info.function_definitions_lookup.at(function_call.function_name);
        if(definition.params.size() != function_call.params.size()) {
            throw std::runtime_error("Function [" + function_call.function_name + "] param count mismatch.");
        }
        // TODO: type check parameters to function call
    } else {
        throw std::runtime_error("Function [" + function_call.function_name + "] not declared or defined.");
    }

    return std::make_shared<ast::function_call_t>(std::move(function_call));
}
std::variant<ast::var_name_t, std::shared_ptr<ast::function_call_t>> parse_and_validate_variable_or_function_call(parser_t& parser) {
    auto name_token = parser.advance_token();
    if(name_token.token_type != token_type_t::IDENTIFIER) {
        throw std::runtime_error("Invalid identifier: [" + std::to_string(static_cast<std::uint32_t>(name_token.token_type)) + std::string("]"));
    }

    if(parser.peek_token().token_type == token_type_t::LEFT_PAREN) {
        return parse_and_validate_function_call(parser, name_token);
    } else {
        return parse_and_validate_variable(parser, name_token);
    }
}
ast::expression_t parse_prefix_expression(parser_t& parser) {
    std::cout << "Non-Error Prefix Expression: " << static_cast<std::uint32_t>(parser.peek_token().token_type) << ": " << parser.peek_token().token_text << std::endl;
    switch(parser.peek_token().token_type) {
        case token_type_t::IDENTIFIER:
            return {parse_and_validate_variable_or_function_call(parser), std::nullopt};
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
                auto rhs = parse_and_validate_expression(parser, r_bp);
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
ast::expression_t parse_and_validate_expression(parser_t& parser, const ast::precedence_t precedence) {
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
            auto rhs = parse_and_validate_expression(parser, r_bp);
            lhs = {make_infix_op(op, std::move(lhs), std::move(rhs)), std::nullopt};
            continue;
        }

        if(is_compound_assignment_op(parser.peek_token())) {
            auto op = get_op_from_compound_assignment_op(parser.peek_token());
            parser.advance_token();
            lhs = {make_infix_op(ast::binary_operator_token_t::ASSIGNMENT, {validate_lvalue_expression_exp(lhs), std::nullopt}, {make_infix_op(op, {validate_lvalue_expression_exp(lhs), std::nullopt}, parse_and_validate_expression(parser, precedence)), std::nullopt}), std::nullopt};
            continue;
        }

        if(parser.peek_token().token_type == token_type_t::QUESTION_MARK) {
            // TODO: double check that the associativity and precedence are correct for ternary expressions
            auto [r_bp, l_bp] = ternary_binding_power();
            if(l_bp < precedence) {
                break;
            }
            parser.advance_token();
            auto if_true = parse_and_validate_expression(parser, precedence);
            parser.expect_token(token_type_t::COLON, "Expected `:` in ternary expression.");
            auto if_false = parse_and_validate_expression(parser, 28); // 28 is the highest level of precedence, we use this so it greedily parses `a < b ? a = 1 : a = 2` as `(a < b ? a = 1 : a) = 2` instead of `(a < b ? a = 1 : a = 2)`.
            lhs = {std::make_shared<ast::ternary_expression_t>(ast::ternary_expression_t{std::move(lhs), std::move(if_true), std::move(if_false)}), std::nullopt};
            continue;
        }

        break;
    }

    return lhs;
}
ast::expression_t parse_and_validate_expression(parser_t& parser) {
    return parse_and_validate_expression(parser, 0u);
}

ast::return_statement_t parse_return_statement(parser_t& parser) {
    parser.expect_token(token_type_t::RETURN_KEYWORD, "Expected `return` keyword in statement.");

    auto expression = parse_and_validate_expression(parser);

    parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in statement.");

    return ast::return_statement_t{std::move(expression)};
}
ast::expression_statement_t parse_and_validate_expression_statement(parser_t& parser) {
    if(parser.peek_token().token_type == token_type_t::SEMICOLON) {
        parser.advance_token();
        return ast::expression_statement_t{std::nullopt}; // null statement, i.e. `;`
    }

    auto expression = parse_and_validate_expression(parser);

    parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in statement.");

    return ast::expression_statement_t{std::move(expression)};
}
ast::if_statement_t parse_and_validate_if_statement(parser_t& parser) {
    parser.expect_token(token_type_t::IF_KEYWORD, "Expected `if` keyword in statement.");

    parser.expect_token(token_type_t::LEFT_PAREN, "Expected `(` in statement.");

    auto if_exp = parse_and_validate_expression(parser);

    parser.expect_token(token_type_t::RIGHT_PAREN, "Expected `)` in statement.");

    ast::statement_t if_body;
    if(parser.peek_token().token_type == token_type_t::LEFT_CURLY) {
        if_body = std::make_shared<ast::compound_statement_t>(parse_and_validate_compound_statement(parser));
    } else {
        if_body = parse_and_validate_statement(parser);
    }

    if(parser.peek_token().token_type != token_type_t::ELSE_KEYWORD) {
        return ast::if_statement_t{std::move(if_exp), std::move(if_body), std::nullopt};
    }

    parser.advance_token(); // consume `else` keyword

    if(parser.peek_token().token_type == token_type_t::LEFT_CURLY) {
        return ast::if_statement_t{std::move(if_exp), std::move(if_body), std::make_shared<ast::compound_statement_t>(parse_and_validate_compound_statement(parser))};
    } else {
        return ast::if_statement_t{std::move(if_exp), std::move(if_body), parse_and_validate_statement(parser)};
    }
}
ast::statement_t parse_and_validate_statement(parser_t& parser) {
    if(parser.is_eof()) {
        throw std::runtime_error("Unexpected end of file.");
    }

    const auto next_token_type = parser.peek_token().token_type;
    if(next_token_type == token_type_t::RETURN_KEYWORD) {
        return parse_and_validate_return_statement(parser);
    } else if(next_token_type == token_type_t::IF_KEYWORD) {
        return std::make_shared<ast::if_statement_t>(parse_and_validate_if_statement(parser));
    } else if(next_token_type == token_type_t::LEFT_CURLY) {
        return std::make_shared<ast::compound_statement_t>(parse_and_validate_compound_statement(parser));
    }
    return parse_and_validate_expression_statement(parser);
}
ast::declaration_t parse_and_validate_declaration(parser_t& parser) {
    const auto type_token = parser.advance_token();
    if(!is_a_type(type_token)) {
        std::cout << static_cast<std::uint32_t>(type_token.token_type) << ": " << type_token.token_text << std::endl;
        throw std::runtime_error("Expected type name in declaration.");
    }

    auto identifier_token = parser.advance_token();
    if(identifier_token.token_type != token_type_t::IDENTIFIER) {
        throw std::runtime_error("Expected identifier.");
    }

    auto var_name = ast::var_name_t(identifier_token.token_text);

    if(parser.symbol_info.variable_lookup.contains_in_lowest_scope(var_name)) {
        throw std::runtime_error("Variable " + var_name + " already declared in current scope.");
    }

    auto type = create_type_name_from_token(type_token);

    if(parser.peek_token().token_type != token_type_t::EQUALS) {
        auto ret = ast::declaration_t{type, var_name, std::nullopt};

        parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in statement.");

        parser.symbol_info.variable_lookup.add_new_variable_in_current_scope(var_name, type);

        return ret;
    }

    parser.advance_token(); // consume `=` token

    auto ret = ast::declaration_t{type, var_name, parse_and_validate_expression(parser)};

    parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in statement.");

    parser.symbol_info.variable_lookup.add_new_variable_in_current_scope(var_name, type);

    return ret;
}
ast::compound_statement_t parse_and_validate_compound_statement(parser_t& parser, bool is_function_block) {
    if(!is_function_block) {
        parser.symbol_info.variable_lookup.create_new_scope();
    }

    parser.expect_token(token_type_t::LEFT_CURLY, "Expected `{` in statement.");

    ast::compound_statement_t ret{};

    while(parser.peek_token().token_type != token_type_t::RIGHT_CURLY) {
        if(parser.is_eof()) {
            throw std::runtime_error("Unexpected end of file. Unterminated compound statement.");
        }

        if(is_a_type(parser.peek_token())) {
            ret.stmts.push_back(parse_and_validate_declaration(parser));
        } else {
            ret.stmts.push_back(parse_and_validate_statement(parser));
        }
    }
    parser.advance_token(); // consume `}` token

    if(!is_function_block) {
        parser.symbol_info.variable_lookup.destroy_current_scope();
    }

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
ast::function_declaration_t parse_function_declaration(parser_t& parser, const token_t type_token, const token_t name_token, std::vector<std::pair<ast::type_t, std::optional<ast::var_name_t>>>&& param_list) {
    parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in function declaration.");


    auto function_declaration = ast::function_declaration_t{ create_type_name_from_token(type_token), ast::func_name_t(name_token.token_text), parse_function_declaration_parameter_list(param_list) };

    if(utils::contains(parser.symbol_info.global_variable_declarations, function_declaration.function_name) || utils::contains(parser.symbol_info.global_variable_definitions, function_declaration.function_name)) {
        throw "Function [" + function_declaration.function_name + "] is already declared as a global variable.";
    }

    if(utils::contains(parser.symbol_info.function_definitions_lookup, function_declaration.function_name)) {
        const auto existing_function_definition = parser.symbol_info.function_definitions_lookup.at(function_declaration.function_name);
        validate_type_name(function_declaration.return_type, existing_function_definition.return_type, "Function [" + function_declaration.function_name + "] return type mismatch.");
        if(function_declaration.params.size() != existing_function_definition.params.size()) {
            throw std::runtime_error("Function [" + function_declaration.function_name + "] param count mismatch.");
        }
        for(std::uint32_t i = 0u; i < function_declaration.params.size(); ++i) {
            validate_type_name(function_declaration.params[i], existing_function_definition.params[i].first, "Function [" + function_declaration.function_name + "] param type mismatch.");
        }
    }
    if(utils::contains(parser.symbol_info.function_declarations_lookup, function_declaration.function_name)) {
        const auto existing_function_declaration = parser.symbol_info.function_declarations_lookup.at(function_declaration.function_name);
        validate_type_name(function_declaration.return_type, existing_function_declaration.return_type, "Function [" + function_declaration.function_name + "] return type mismatch.");
        if(function_declaration.params.size() != existing_function_declaration.params.size()) {
            throw std::runtime_error("Function [" + function_declaration.function_name + "] param count mismatch.");
        }
        for(std::uint32_t i = 0u; i < function_declaration.params.size(); ++i) {
            validate_type_name(function_declaration.params[i], existing_function_declaration.params[i], "Function [" + function_declaration.function_name + "] param type mismatch.");
        }
    } else {
        parser.symbol_info.function_declarations_lookup.insert({function_declaration.function_name, function_declaration});
    }


    return function_declaration;
}
ast::function_definition_t parse_function_definition(parser_t& parser, const token_t type_token, const token_t name_token, std::vector<std::pair<ast::type_t, std::optional<ast::var_name_t>>>&& param_list) {
    auto type = create_type_name_from_token(type_token);
    auto name = ast::func_name_t(name_token.token_text);

    if(utils::contains(parser.symbol_info.global_variable_declarations, name) || utils::contains(parser.symbol_info.global_variable_definitions, name)) {
        throw "Function [" + name + "] is already declared as a global variable.";
    }

    if(utils::contains(parser.symbol_info.function_definitions_lookup, name)) {
        throw std::runtime_error("Function [" + name + "] already defined.");
    }


    if(utils::contains(parser.symbol_info.function_declarations_lookup, name)) {
        const auto existing_function_declaration = parser.symbol_info.function_declarations_lookup.at(name);
        validate_type_name(type, existing_function_declaration.return_type, "Function [" + name + "] return type mismatch.");
        if(param_list.size() != existing_function_declaration.params.size()) {
            throw std::runtime_error("Function [" + name + "] param count mismatch.");
        }
        for(std::uint32_t i = 0u; i < param_list.size(); ++i) {
            validate_type_name(param_list[i].first, existing_function_declaration.params[i], "Function [" + name + "] param type mismatch.");
        }
    } else {
        // We add it to the function declaration table even though it is not a function definition because then we can do parsing and symbol validation all
        //  at once while still supporting recursion in the body of the function definition with no prior function declaration without hackiness.
        // We don't need to remove the function declaration once we're done because the function declarations do not matter after symbol validation.
        // And a function definition is a stronger statement than a function declaration anyways and can be redeclared infinite times (as long as they all match).
        ast::function_declaration_t function_declaration = ast::function_declaration_t{ type, name, parse_function_declaration_parameter_list(param_list) };
        parser.symbol_info.function_declarations_lookup.insert({name, std::move(function_declaration)});
    }

    parser.symbol_info.variable_lookup.create_new_scope();
    for(const auto& param : param_list) {
        if(param.second.has_value()) {
            parser.symbol_info.variable_lookup.add_new_variable_in_current_scope(param.second.value(), param.first);
        }
    }
    ast::compound_statement_t function_body_statements = parse_and_validate_compound_statement(parser, true);
    parser.symbol_info.variable_lookup.destroy_current_scope();


    if(name_token.token_text == "main" && type_token.token_text == "int") {
        constexpr int DEFAULT_RETURN_VALUE = 0;
        // use `has_return_statement` instead of `is_return_statement` because we don't need to emit a return statement if there already is one,
        //  even if there is unreachable code after the already existing return statement.
        if(function_body_statements.stmts.size() == 0 || !has_return_statement(function_body_statements)) {
            function_body_statements.stmts.push_back(ast::return_statement_t { ast::expression_t{ ast::constant_t{DEFAULT_RETURN_VALUE}, make_primitive_type_t(ast::type_category_t::INT, "int", sizeof(std::int32_t), alignof(std::int32_t)) } } );
        }
    }

    ast::function_definition_t function_definition = ast::function_definition_t{ std::move(type), name, std::move(param_list), std::move(function_body_statements) };

    parser.symbol_info.function_definitions_lookup.insert({std::move(name), std::move(function_definition)});

    return function_definition;
}
std::variant<ast::function_declaration_t, ast::function_definition_t> parse_function_decl_or_def(parser_t& parser, const token_t type_token, const token_t name_token) {
    parser.expect_token(token_type_t::LEFT_PAREN, "Expected '(' in function declaration/definition.");

    auto param_list = parse_function_definition_parameter_list(parser);

    parser.expect_token(token_type_t::RIGHT_PAREN, "Expected `)` in function declaration/definition.");

    const auto next_token = parser.peek_token();
    if(next_token.token_type == token_type_t::SEMICOLON) {
        return parse_function_declaration(parser, type_token, name_token, std::move(param_list));
    } else if(next_token.token_type == token_type_t::LEFT_CURLY) {
        return parse_function_definition(parser, type_token, name_token, std::move(param_list));
    } else {
        throw std::runtime_error("Expected either `;` or `{` in function declaration/definition.");
    }
}
std::variant<ast::function_declaration_t, ast::function_definition_t, ast::global_variable_declaration_t> parse_function_or_global(parser_t& parser) {
    const auto type_token = parser.advance_token();
    if(!is_a_type(type_token)) {
        throw std::logic_error("Expected a type identifier in function or global variable declaration/definition.");
    }

    const auto name_token = parser.advance_token();
    if(name_token.token_type != token_type_t::IDENTIFIER) {
        throw std::logic_error("Expected a identifier name in function or global variable declaration/definition.");
    }

    const auto next_token = parser.peek_token();
    if(next_token.token_type == token_type_t::LEFT_PAREN) {
        return parse_function_decl_or_def(parser, type_token, name_token); // parses either a function declaration or definition
    } else if(next_token.token_type == token_type_t::EQUALS) {
        return parse_global_variable_definition(parser, type_token, name_token);
    } else if(next_token.token_type == token_type_t::SEMICOLON) {
        return parse_global_variable_declaration(parser, type_token, name_token);
    } else {
        throw std::runtime_error("Expected either global variable declaration, global variable definition, or start of function.");
    }
}
ast::type_t parse_and_validate_struct_body(parser_t& parser, const ast::type_name_t& name) {
    parser.expect_token(token_type_t::LEFT_CURLY, "Expected `{` in struct definition.");

    std::vector<ast::type_t> struct_field_types;
    std::vector<std::string> struct_field_names;

    while(parser.peek_token().token_type != token_type_t::RIGHT_CURLY) {
        auto field_type = parse_and_validate_type(parser);

        std::vector<std::string> field_names_in_line;
        auto field_name = parser.advance_token();
        if(field_name.token_type != token_type_t::IDENTIFIER) {
            throw std::logic_error("Expected identifier name in struct definition for field.");
        }
        field_names_in_line.push_back(std::string(field_name.token_text));

        while(parser.peek_token().token_type == token_type_t::COMMA) {
            parser.advance_token();
            field_name = parser.advance_token();
            if(field_name.token_type != token_type_t::IDENTIFIER) {
                throw std::logic_error("Expected identifier name in struct definition for field.");
            }
            field_names_in_line.push_back(std::string(field_name.token_text));
        }

        parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in struct definition.");

        for(auto& field_name : field_names_in_line) {
            struct_field_types.push_back(field_type);
            struct_field_names.push_back(std::move(field_name));
        }
    }

    parser.expect_token(token_type_t::RIGHT_CURLY, "Expected `}` in struct definition.");
    parser.expect_token(token_type_t::SEMICOLON, "Expected `;` in struct definition.");

    return make_struct_definition_type_t(parser.symbol_info.type_table, name, std::move(struct_field_types), std::move(struct_field_names));
}
ast::type_t parse_struct(parser_t& parser) {
    parser.expect_token(token_type_t::STRUCT_KEYWORD, "Expected `struct` keyword in struct declaration/definition.");

    const auto name_token = parser.advance_token();
    if(name_token.token_type != token_type_t::IDENTIFIER) {
        throw std::logic_error("Expected identifier name in struct declaration/definition.");
    }
    auto name = ast::type_name_t(name_token.token_text);

    auto& struct_type_table = parser.symbol_info.type_table.at(static_cast<std::uint32_t>(ast::type_category_t::STRUCT));

    const auto next_token = parser.peek_token();
    if(next_token.token_type == token_type_t::SEMICOLON) {
        parse.advance_token();
        // parse struct declaration
        if(!utils::contains(struct_type_table, name)) {
            auto struct_forward_decl = make_struct_forward_decl_type_t(name);
            struct_type_table.insert({name, struct_forward_decl});
            return struct_forward_decl;
        }
    } else if(next_token.token_type == token_type_t::LEFT_CURLY) {
        // parse struct definition
        if(!utils::contains(struct_type_table, name) || !struct_type_table.at(name).size.has_value()) {
            auto struct_definition = parse_and_validate_struct_body(parser, name);
            // TODO: Double check the C++ docs to make sure this is equivalent to:
            // struct_type_table.at(name) = std::move(struct_definition);
            // for when there is already a struct declaration entry at `name`a
            struct_type_table.insert({name, std::move(struct_definition)}); // should overwrite any struct declaration entry that was previously written to `struct_type_table.at(name)`
        } else {
            throw std::runtime_error("Struct [" + name + "] already defined.");
        }
    } else {
        throw std::runtime_error("Expected either `;` or `{` in struct declaration/definition.");
    }
}
ast::type_t parse_typedef(parser_t& parser) {
    parser.expect_token(token_type_t::TYPEDEF_KEYWORD, "Expected `typedef` keyword in typedef declaration.");

    if(is_struct_keyword(parser.peek_token())) {
        
    } else if(is_keyword_a_type(parser.peek_token().token_type)) { // if the next token is a primitive type

    } else { // type being aliased must be a typedef/non-primitive type name
        
    }
}
std::variant<ast::function_declaration_t, ast::function_definition_t, ast::global_variable_declaration_t, ast::type_t> parse_top_level_declaration(parser_t& parser) {
    const auto first_token = parser.peek_token();
    if(is_a_type(first_token)) {
        return parse_function_or_global(parser); // parses either a global variable declaration/definition, function declaration, or function definition
    }

    if(is_struct_keyword(first_token)) {
        return parse_struct(parser); // parses either a struct declaration or struct definition
    }

    if(is_typedef_keyword(first_token)) {
        return parse_typedef(parser);
    }

    throw std::runtime_error("Unrecognized top level declaration/definition.");
}

ast::validated_program_t parse(parser_t& parser) {
    add_floating_point_types_to_type_table(parser);
    add_integer_types_to_type_table(parser);
    add_unsigned_integer_types_to_type_table(parser);

    while(parser.peek_token().token_type != token_type_t::EOF_TOK) {
        parse_top_level_declaration(parser);
    }

    ast::validated_program_t validated_program{};
    for(const auto& it : parser.symbol_info.function_definitions_lookup) {
        validated_program.top_level_declarations.push_back(it.second);
    }
    for(const auto& it : parser.symbol_info.global_variable_definitions) {
        ast::validated_global_variable_definition_t validated_definition{it.second.type_name, it.first, evaluate_expression(it.second.value.value())};

        validated_program.top_level_declarations.push_back(validated_definition);
    }
    for(const auto& it : parser.symbol_info.global_variable_declarations) {
        if(!utils::contains(parser.symbol_info.global_variable_definitions, it.first)) {
            ast::validated_global_variable_definition_t validated_definition{it.second.type_name, it.first, ast::constant_t{0}};

            validated_program.top_level_declarations.push_back(validated_definition);
        }
    }
    return validated_program;
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




