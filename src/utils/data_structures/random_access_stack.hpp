#pragma once


#include <cstddef>
#include <stdexcept>
#include <vector>
#include <iterator>
#include <functional>
#include <string>
#include <unordered_map>

#include <frontend/ast/ast.hpp>


// TODO: support iterators
namespace utils::data_structures {
template<typename T>
class random_access_stack_t {
    std::vector<T> data;
    std::size_t stack_size = 0u;

public:
    random_access_stack_t() = default;
    random_access_stack_t(std::vector<T> data) : data(std::move(data)), stack_size(data.size()) {}

    const T& operator[](const std::size_t i) const {
        return data[i];
    }
    T& operator[](const std::size_t i) {
        return data[i];
    }

    const T& at(const std::size_t i) const {
        return data.at(i);
    }
    T& at(const std::size_t i) {
        return data.at(i);
    }

    bool is_empty() const {
        return stack_size == 0u;
    }
    std::size_t size() const {
        return stack_size;
    }
    std::size_t last_index() const {
        if(is_empty()) {
            throw std::logic_error("Stack is empty");
        }
        return stack_size - 1u;
    }

    void push(const T& value) {
        data.push_back(value);
        ++stack_size;
    }
    T pop() {
        if(is_empty()) {
            throw std::logic_error("Stack is empty");
        }
        return data.at(--stack_size);
    }

    const T& peek() const {
        if(is_empty()) {
            throw std::logic_error("Stack is empty");
        }
        return data.at(stack_size - 1);
    }
    T& peek() {
        if(is_empty()) {
            throw std::logic_error("Stack is empty");
        }
        return data.at(stack_size - 1);
    }
};

using rbp_offset_t = std::uint64_t;
struct block_scope_t {
    std::unordered_map<std::string, rbp_offset_t> variables;

    // current offset from what the `rsp` was at the beginning/creation of the current block scope (i.e. how much to increment `rsp` by once this block scope ends).
    std::uint64_t stack_size = 0u;
};
class backend_variable_lookup_t {
    random_access_stack_t<block_scope_t> scopes;

public:
    backend_variable_lookup_t() = default;

    bool contains_in_lowest_scope(const std::string& variable_name) const {
        return scopes.peek().variables.find(variable_name) != scopes.peek().variables.end();
    }
    bool contains_in_accessible_scopes(const std::string& variable_name) {
        for(std::uint32_t i = 0u; i < scopes.size(); ++i) {
            if(scopes.at(i).variables.find(variable_name) != scopes.at(i).variables.end()) {
                return true;
            }
        }
        return false;
    }
    std::optional<std::uint64_t> find_from_lowest_scope(const std::string& variable_name) {
        for(std::uint32_t i = scopes.last_index(); i < scopes.size(); --i) { // iterate backwards so we start in lowest level scope and use `i < variables.size()` so we handle unsigned integer underflow for `i`.
            auto it = scopes.at(i).variables.find(variable_name);
            if(it != scopes.at(i).variables.end()) {
                return it->second; // returns the variable ebp offset for the lowest scope level variable with this name. This means we properly handle variable shadowing.
            }
        }
        return std::nullopt; // variable with this name not found in any accessible scope
    }

    void add_new_variable_in_current_scope(const ast::var_name_t& variable_name, const rbp_offset_t ebp_offset) {
        scopes.peek().variables.insert({variable_name, ebp_offset});
        scopes.peek().stack_size += sizeof(std::uint64_t); // TODO: we currently only support 64 bit integer type
    }

    const std::unordered_map<std::string, rbp_offset_t>& get_current_lowest_scope() const {
        return scopes.peek().variables;
    }
    std::unordered_map<std::string, rbp_offset_t>& get_current_lowest_scope() {
        return scopes.peek().variables;
    }

    void create_new_scope() {
        scopes.push(block_scope_t{});
    }
    std::uint64_t destroy_current_scope() {
        return scopes.pop().stack_size;
    }
};
class validation_variable_lookup_t {
    random_access_stack_t<std::unordered_map<ast::var_name_t, ast::type_name_t>> variables;

public:
    validation_variable_lookup_t() = default;

    bool contains_in_lowest_scope(const std::string& variable_name) const {
        return variables.peek().find(variable_name) != variables.peek().end();
    }
    bool contains_in_accessible_scopes(const std::string& variable_name) {
        for(std::uint32_t i = 0u; i < variables.size(); ++i) {
            if(variables.at(i).find(variable_name) != variables.at(i).end()) {
                return true;
            }
        }
        return false;
    }

    void add_new_variable_in_current_scope(const ast::var_name_t& variable_name, const ast::type_name_t& variable_type) {
        variables.peek().insert({variable_name, variable_type});
    }

    void create_new_scope() {
        variables.push({});
    }
    void destroy_current_scope() {
        variables.pop();
    }
};
}
