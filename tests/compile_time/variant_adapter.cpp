#include "gtest/gtest.h"

#include <utils/common.hpp>

#include <variant>
#include <type_traits>
#include <string>

namespace {

TEST(variant_adapter, superset_conversion) {
    std::variant<int, float> v = 7.f;
    auto converted_v = utils::variant_adapter<std::variant<int, float, std::string>>(v);
    static_assert(std::is_same_v<std::variant<int, float, std::string>, decltype(converted_v)>);
}
TEST(variant_adapter, same_type_conversion) {
    std::variant<int, float> v = 7.f;
    auto converted_v = utils::variant_adapter<decltype(v)>(v);
    static_assert(std::is_same_v<std::variant<int, float>, decltype(converted_v)>);
}

}
