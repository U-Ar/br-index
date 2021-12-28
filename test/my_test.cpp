#include "iutest.hpp"

int factorial(int n) {
    if (n == 0) return 1;
    return n * factorial(n-1);
}


IUTEST(FactorialTest, HandlesZeroInput) {
    IUTEST_ASSERT_EQ(1, factorial(0));
}

IUTEST(FactorialTest, HandlesPositiveInput) {
    IUTEST_ASSERT_EQ(6,factorial(3));
}