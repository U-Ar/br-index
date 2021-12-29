#include "iutest.hpp"
#include <vector>
#include <fstream>
#include <string>

#include "../src/br_index.hpp"

using namespace bri;

IUTEST(BrIndexTest, BasicConstruction)
{
    std::string s("aaaaaaaaaaaaaaaaa");
    br_index<> idx(s);
}