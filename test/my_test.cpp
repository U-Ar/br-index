#include "gtest/gtest.h"

int factorial(int n) {
    if (n == 0) return 1;
    return n * factorial(n-1);
}


TEST(FactorialTest, HandlesZeroInput) {
    EXPECT_EQ(1, factorial(0));
}

TEST(FactorialTest, HandlesPositiveInput) {
    EXPECT_EQ(6,factorial(3));
}