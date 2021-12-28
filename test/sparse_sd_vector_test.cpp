#include "iutest.hpp"

#include <vector>
#include "../src/sparse_sd_vector.hpp"

IUTEST(SparseSdVectorTest, AllZero) {
    std::vector<bool> vec(10000,0);
    bri::sparse_sd_vector bv(vec);

    for (size_t i = 0; i < bv.size(); ++i)
    {
        IUTEST_ASSERT_EQ(0,bv[i]);
        IUTEST_ASSERT_EQ(0,bv.rank(i));
    }
    IUTEST_ASSERT_EQ(0,bv.number_of_1());
}

IUTEST(SparseSdVectorTest, AllOne) {
    std::vector<bool> vec(10000,1);
    bri::sparse_sd_vector bv(vec);

    for (size_t i = 0; i < bv.size(); ++i)
    {
        IUTEST_ASSERT_EQ(1,bv[i]);
        IUTEST_ASSERT_EQ(i,bv.rank(i));
        IUTEST_ASSERT_EQ(i,bv.select(i));
    }
}