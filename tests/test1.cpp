#include "gtest/gtest.h"
#include <utils/common.hpp>

namespace {

TEST(ExampleTests, Test1) {
    EXPECT_TRUE(utils::match_caseless_char('c', 'C'));
}

}
