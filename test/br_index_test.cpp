#include "iutest.hpp"
#include <vector>
#include <fstream>
#include <string>

#include "../src/br_index.hpp"

using namespace bri;

template<class T>
void print_vec(std::vector<T>& vec)
{
    for (size_t i = 0; i < vec.size(); ++i)
        std::cout << vec[i] << " ";
    std::cout << std::endl;
}

void print_range(range_t rg)
{
    std::cout << "1st val: " << rg.first <<
    " 2nd val: " << rg.second << std::endl;
}

IUTEST(BrIndexTest, BasicExtension)
{
    std::string s("aaaaaaaaaaaaaaaaaaaa");
    br_index<> idx(s);
    IUTEST_ASSERT_EQ(20,idx.text_size());
    IUTEST_ASSERT_EQ(21,idx.bwt_size());
    range_t range = idx.full_range();
    IUTEST_ASSERT_EQ(20,range.second);
    IUTEST_ASSERT_EQ(0,range.first);

    idx.reset_pattern();
    range = idx.left_extension('a');
    IUTEST_ASSERT_EQ(20,range.second);
    IUTEST_ASSERT_EQ(1,range.first);
    std::vector<ulint> res = idx.locate();
    IUTEST_ASSERT_EQ(20,res.size());
    for (ulint i = 0; i < res.size(); ++i)
    {
        IUTEST_ASSERT_EQ(19-i,res[i]);
    }

    idx.reset_pattern();
    range = idx.right_extension('a');
    IUTEST_ASSERT_EQ(20,range.second);
    IUTEST_ASSERT_EQ(1,range.first);
    res = idx.locate();
    IUTEST_ASSERT_EQ(20,res.size());
    for (ulint i = 0; i < res.size(); ++i)
    {
        IUTEST_ASSERT_EQ(19-i,res[i]);
    }
}

IUTEST(BrIndexTest, PeriodicTextLocate)
{
    std::string s("abcdabcdabcdabcdhello");
    br_index<> idx(s);
    idx.reset_pattern();
    idx.right_extension('a');
    idx.right_extension('b');
    idx.right_extension('c');
    range_t range = idx.right_extension('d');
    IUTEST_ASSERT_EQ(4,range.second-range.first+1);
    auto vec = idx.locate();
    std::cout << "right ex only" << std::endl;
    print_vec<>(vec);
    IUTEST_ASSERT_EQ(0,vec[0]);
    IUTEST_ASSERT_EQ(4,vec[1]);
    IUTEST_ASSERT_EQ(8,vec[2]);
    IUTEST_ASSERT_EQ(12,vec[3]);


    idx.reset_pattern();
    idx.left_extension('d');
    idx.left_extension('c');
    idx.left_extension('b');
    range = idx.left_extension('a');
    IUTEST_ASSERT_EQ(4,range.second-range.first+1);
    vec = idx.locate();
    std::cout << "left ex only" << std::endl;
    print_vec<>(vec);
    IUTEST_ASSERT_EQ(0,vec[0]);
    IUTEST_ASSERT_EQ(4,vec[1]);
    IUTEST_ASSERT_EQ(8,vec[2]);
    IUTEST_ASSERT_EQ(12,vec[3]);


    idx.reset_pattern();
    range = idx.left_extension('b');
    range = idx.right_extension('c');
    range = idx.left_extension('a');
    range = idx.right_extension('d');
    IUTEST_EXPECT_EQ(4,range.second-range.first+1);
    vec = idx.locate();
    std::cout << "right and left mixed" << std::endl;
    print_vec<>(vec);
    IUTEST_EXPECT_EQ(0,vec[0]);
    IUTEST_EXPECT_EQ(4,vec[1]);
    IUTEST_EXPECT_EQ(8,vec[2]);
    IUTEST_EXPECT_EQ(12,vec[3]);


    idx.reset_pattern();
    range = idx.right_extension('c');
    range = idx.left_extension('b');
    range = idx.right_extension('d');
    range = idx.left_extension('a');
    IUTEST_EXPECT_EQ(4,idx.pattern_length());
    IUTEST_EXPECT_EQ(4,range.second-range.first+1);
    vec = idx.locate();
    std::cout << "right and left mixed2" << std::endl;
    print_vec<>(vec);
    IUTEST_EXPECT_EQ(0,vec[0]);
    IUTEST_EXPECT_EQ(4,vec[1]);
    IUTEST_EXPECT_EQ(8,vec[2]);
    IUTEST_EXPECT_EQ(12,vec[3]);

}