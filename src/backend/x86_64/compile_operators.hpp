#pragma once

#include <variant>
#include <string>
#include <stdexcept>
#include <cstdint>

#include <frontend/parsing/ast.hpp>


struct assembly_output_t  {
    std::string output;
    std::uint64_t current_label_number = 0; // appended to the end of labels so we don't get duplicate labels for things like boolean short circuiting

    assembly_output_t() = default;
    assembly_output_t(std::string output) : output(std::move(output)) {}
};

void store_constant(assembly_output_t& assembly_output, const ast::constant_t& constant);
void store_register(assembly_output_t& assembly_output, const std::string& register_name);
void pop_constant(assembly_output_t& assembly_output, const std::string& register_name);

void generate_negation(assembly_output_t& assembly_output);
void generate_bitwise_not(assembly_output_t& assembly_output);
void generate_logic_not(assembly_output_t& assembly_output);
void generate_multiplication(assembly_output_t& assembly_output);
void generate_division(assembly_output_t& assembly_output);
void generate_addition(assembly_output_t& assembly_output);
void generate_subtraction(assembly_output_t& assembly_output);
void generate_less_than(assembly_output_t& assembly_output);
void generate_greater_than(assembly_output_t& assembly_output);
void generate_less_than_equal(assembly_output_t& assembly_output);
void generate_greater_than_equal(assembly_output_t& assembly_output);
void generate_equality(assembly_output_t& assembly_output);
void generate_not_equals(assembly_output_t& assembly_output);
void generate_logical_and(assembly_output_t& assembly_output);
void generate_logical_or(assembly_output_t& assembly_output);
