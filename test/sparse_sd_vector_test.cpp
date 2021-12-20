#include "gtest/gtest.h"

#include "../src/sparse_sd_vector.hpp"

TEST(SparseSdVectorTest, AllZero) {
    std::vector<bool> vec(10000,0);
    bri::sparse_sd_vector bv(vec);

    for (size_t i = 0; i < bv.size(); ++i)
    {
        EXPECT_EQ(0,bv[i]);
    }
}

TEST(SparseSdVectorTest, AllOne) {
    std::vector<bool> vec(10000,1);
    bri::sparse_sd_vector bv(vec);

    for (size_t i = 0; i < bv.size(); ++i)
    {
        EXPECT_EQ(1,bv[i]);
    }
}