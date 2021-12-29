#include "iutest.hpp"
#include <vector>
#include <fstream>
#include <string>

#include "../src/br_index.hpp"

using namespace bri;

IUTEST(BrIndexTest, BasicConstruction)
{
    std::string s("aaaaaaaaaaaaaaaaaaaa");
    br_index<> idx(s);
    std::cout << "construction OK" << std::endl;
    IUTEST_ASSERT_EQ(20,idx.text_size());
    IUTEST_ASSERT_EQ(21,idx.text_size());
}