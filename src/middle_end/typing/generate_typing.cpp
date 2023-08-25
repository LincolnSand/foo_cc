#include <middle_end/validation/validate_ast.hpp>


static ast::type_name_t get_type_of_function(const validation_t& validation, const ast::func_name_t& function_name) {
    if(utils::contains(validation.function_declarations_lookup, function_name)) {
        return validation.function_declarations_lookup.at(function_name).return_type;
    }
    else if(utils::contains(validation.function_definitions_lookup, function_name)) {
        return validation.function_definitions_lookup.at(function_name).return_type;
    }
    throw std::logic_error("Function [" + function_name + "] is not declared.");
}
static ast::type_name_t get_type_of_variable(const validation_t& validation, const ast::var_name_t& variable_name) {
    if(validation.variable_lookup.contains_in_accessible_scopes(variable_name)) {
        return validation.variable_lookup.find_in_accessible_scopes(variable_name);
    }
    if(utils::contains(validation.global_variable_declarations, variable_name)) {
        return validation.global_variable_declarations.at(variable_name).type_name;
    }
    if(utils::contains(validation.global_variable_definitions, variable_name)) {
        return validation.global_variable_definitions.at(variable_name).type_name;
    }
    throw std::logic_error("Variable [" + variable_name + "] is not declared.");
}

void add_type_to_grouping(const ast::grouping_t& expr, std::optional<ast::type_name_t>& exp_type) {
    exp_type = expr.expr.type.value();
}
void add_type_to_unary_expression(const ast::unary_expression_t& expr, std::optional<ast::type_name_t>& exp_type) {
    // TODO: check that the unary operator is supported for the type
    exp_type = expr.exp.type.value();
}
void add_type_to_binary_expression(const ast::binary_expression_t& expr, std::optional<ast::type_name_t>& exp_type) {
    // TODO: check that the binary operator is supported for the type
    // TODO: check that these two types are convertible
    validate_type_name(expr.left.type.value(), expr.right.type.value(), "Mismatched types in binary expression.");
    exp_type = expr.left.type.value();
}
void add_type_to_ternary(const ast::ternary_expression_t& expr, std::optional<ast::type_name_t>& exp_type) {
    // TODO: check that the ternary operator `condition` is convertible to `bool`
    // TODO: check if the ternary operator `if_true` and `if_false` are convertible types.
    validate_type_name(expr.if_true.type.value(), expr.if_false.type.value(), "Mismatched types in ternary expression body.");
    exp_type = expr.if_true.type.value();
}
void add_type_to_function_call(const validation_t& validation, const ast::function_call_t& expr, std::optional<ast::type_name_t>& exp_type) {
    exp_type = get_type_of_function(validation, expr.function_name);
}
void add_type_to_variable(const validation_t& validation, const ast::var_name_t& expr, std::optional<ast::type_name_t>& exp_type) {
    exp_type = get_type_of_variable(validation, expr);
}
