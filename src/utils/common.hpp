#pragma once

#include <cstdint>
#include <charconv>
#include <unordered_map>
#include <string_view>
#include <cctype>


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

inline bool match_caseless_char(const char c1, const char c2) {
    return tolower(c1) == tolower(c2);
}

template<typename T>
inline bool str_to_T(std::string_view s, T& value, const std::size_t suffix_size = 0u) {
    if(s.empty()) return false;
    if(s.size() <= suffix_size) return false;

    auto result = std::from_chars(s.data(), (s.data() + s.size()) - suffix_size, value);

    if(result.ec == std::errc::invalid_argument) return false;

    if(result.ptr != s.data() + s.size()) return false;

    return true;
}

template<typename T, typename U>
inline bool contains(const std::unordered_map<T, U>& map, const T& key) {
    return map.find(key) != map.end();
}
}
