#pragma once


#include <cstddef>
#include <stdexcept>
#include <vector>
#include <iterator>
#include <functional>
#include <string>
#include <unordered_map>


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

class variable_lookup_t {
    random_access_stack_t<std::unordered_map<std::string, std::uint64_t>> variables;

public:
    variable_lookup_t() = default;

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
    std::optional<std::uint64_t> find_from_lowest_scope(const std::string& variable_name) {
        for(std::uint32_t i = variables.last_index(); i < variables.size(); --i) { // iterate backwards so we start in lowest level scope and use `i < variables.size()` so we handle unsigned integer underflow for `i`.
            auto it = variables.at(i).find(variable_name);
            if(it != variables.at(i).end()) {
                return it->second; // returns the variable ebp offset for the lowest scope level variable with this name. This means we properly handle variable shadowing.
            }
        }
        return std::nullopt; // variable with this name not found in any accessible scope
    }

    void add_new_variable_in_current_scope(const std::string& variable_name, const std::uint64_t ebp_offset) {
        variables.peek().insert({variable_name, ebp_offset});
    }

    const std::unordered_map<std::string, std::uint64_t>& get_current_lowest_scope() const {
        return variables.peek();
    }
    std::unordered_map<std::string, std::uint64_t>& get_current_lowest_scope() {
        return variables.peek();
    }

    void create_new_scope() {
        variables.push(std::unordered_map<std::string, std::uint64_t>{});
    }
    void destroy_current_scope() {
        variables.pop();
    }
};
}
