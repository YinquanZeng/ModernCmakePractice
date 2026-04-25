#include "MyDemo/math_api.h"
#include <gtest/gtest.h>

TEST(MathAPITest, AddFunction) {
    EXPECT_EQ(MyDemo::Math::add(2, 3), 5);
    EXPECT_EQ(MyDemo::Math::add(-1, 1), 0);
    EXPECT_EQ(MyDemo::Math::add(0, 0), 0);
}

TEST(MathAPITest, MultiplyFunction) {
    EXPECT_EQ(MyDemo::Math::multiply(3, 4), 12);
    EXPECT_EQ(MyDemo::Math::multiply(-2, 5), -10);
    EXPECT_EQ(MyDemo::Math::multiply(0, 100), 0);
}