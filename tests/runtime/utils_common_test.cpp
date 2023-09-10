#include "gtest/gtest.h"

#include <utils/common.hpp>

namespace {

TEST(is_alpha, is_alpha_underscore) {
    EXPECT_TRUE(utils::is_alpha('_'));
}
TEST(is_alpha, is_alpha_lowercase) {
    EXPECT_TRUE(utils::is_alpha('c'));
}
TEST(is_alpha, is_alpha_uppercase) {
    EXPECT_TRUE(utils::is_alpha('G'));
}
TEST(is_alpha, is_alpha_number) {
    EXPECT_FALSE(utils::is_alpha('0'));
}


TEST(is_digit, is_digit_number) {
    EXPECT_TRUE(utils::is_digit('7'));
}
TEST(is_digit, is_digit_lowercase_letter) {
    EXPECT_FALSE(utils::is_digit('a'));
}
TEST(is_digit, is_digit_uppercase_letter) {
    EXPECT_FALSE(utils::is_digit('K'));
}


TEST(is_alpha_num, is_alpha_num_underscore) {
    EXPECT_TRUE(utils::is_alpha_num('_'));
}
TEST(is_alpha_num, is_alpha_num_lowercase_letter) {
    EXPECT_TRUE(utils::is_alpha_num('z'));
}
TEST(is_alpha_num, is_alpha_num_uppercase_letter) {
    EXPECT_TRUE(utils::is_alpha_num('B'));
}
TEST(is_alpha_num, is_alpha_num_number) {
    EXPECT_TRUE(utils::is_alpha_num('9'));
}
TEST(is_alpha_num, is_alpha_num_false) {
    EXPECT_FALSE(utils::is_alpha_num('-'));
}


TEST(match_caseless_char, match_different_case) {
    EXPECT_TRUE(utils::match_caseless_char('c', 'C'));
}
TEST(match_caseless_char, match_same_case) {
    EXPECT_TRUE(utils::match_caseless_char('a', 'a'));
}
TEST(match_caseless_char, not_match_different_case) {
    EXPECT_FALSE(utils::match_caseless_char('d', 'g'));
}
TEST(match_caseless_char, not_match_same_case) {
    EXPECT_FALSE(utils::match_caseless_char('F', 'z'));
}


TEST(str_to_int, empty_string) {
    int val = 1;
    EXPECT_FALSE(utils::str_to_T("", val));
    EXPECT_EQ(1, val);
}
TEST(str_to_int, suffix_too_big) {
    int val = 1;
    EXPECT_FALSE(utils::str_to_T("1LU", val, 3));
    EXPECT_EQ(1, val);
}
TEST(str_to_int, single_digit_integer) {
    int val = 1;
    EXPECT_TRUE(utils::str_to_T("2", val));
    EXPECT_EQ(2, val);
}
TEST(str_to_int, multi_digit_integer) {
    int val = 1;
    EXPECT_TRUE(utils::str_to_T("237", val));
    EXPECT_EQ(237, val);
}
TEST(str_to_int, single_digit_integer_with_suffix) {
    int val = 1;
    EXPECT_TRUE(utils::str_to_T("2u", val, 1));
    EXPECT_EQ(2, val);
}
TEST(str_to_int, multi_digit_integer_with_suffix) {
    int val = 1;
    EXPECT_TRUE(utils::str_to_T("976UL", val, 2));
    EXPECT_EQ(976, val);
}


TEST(contains, contains_key) {
    std::unordered_map<int, int> unordered_map;
    unordered_map.insert({7, 2});
    EXPECT_TRUE(utils::contains(unordered_map, 7));
}
TEST(contains, does_not_contain_key) {
    std::unordered_map<int, int> unordered_map;
    unordered_map.insert({7, 2});
    EXPECT_FALSE(utils::contains(unordered_map, 2));
}

}
