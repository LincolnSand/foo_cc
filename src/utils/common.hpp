#pragma once

#include <cstdint>
#include <charconv>


template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;


namespace utils {
using line_number_t = std::uint32_t;

// includes `_` as true
inline bool is_alpha(const char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
inline bool is_digit(const char c) {
    return c >= '0' && c <= '9';
}
inline bool is_alpha_num(const char c) {
    return is_alpha(c) || is_digit(c);
}

template<typename T>
inline bool str_to_int(std::string_view s, T& value) {
    if(s.empty()) return false;

    auto result = std::from_chars(s.data(), s.data() + s.size(), value);

    if(result.ec == std::errc::invalid_argument) return false;

    if(result.ptr != s.data() + s.size()) return false;

    return true;
}
}
