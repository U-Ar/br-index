#ifndef BRI_UTILS_HPP
#define BRI_UTILS_HPP

#include "definitions.hpp"

namespace bri {

uchar bitsize(ulint x)
{
    if (x == 0) return 1;
    return 64 - __builtin_clzll(x);
}

};

#endif