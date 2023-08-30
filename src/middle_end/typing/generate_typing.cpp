#include <frontend/parsing/parser.hpp>


bool compare_type_names(const ast::type_t& lhs, const ast::type_t& rhs) {
    if(lhs.type_category == rhs.type_category) {
        if(lhs.type_name == rhs.type_name) {
            return true;
        }
    }
    return false;
}

static ast::type_t get_type_of_function(const validation_t& validation, const ast::func_name_t& function_name) {
    if(utils::contains(validation.function_declarations_lookup, function_name)) {
        return validation.function_declarations_lookup.at(function_name).return_type;
    }
    else if(utils::contains(validation.function_definitions_lookup, function_name)) {
        return validation.function_definitions_lookup.at(function_name).return_type;
    }
    throw std::logic_error("Function [" + function_name + "] is not declared.");
}
static ast::type_t get_type_of_variable(const validation_t& validation, const ast::var_name_t& variable_name) {
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

void add_type_to_function_call(const validation_t& validation, const ast::function_call_t& expr, std::optional<ast::type_t>& exp_type) {
    exp_type = get_type_of_function(validation, expr.function_name);
}
void add_type_to_variable(const validation_t& validation, const ast::var_name_t& expr, std::optional<ast::type_t>& exp_type) {
    exp_type = get_type_of_variable(validation, expr);
}
