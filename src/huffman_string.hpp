/*
 * huffman_string: a wrapper on wt_huff of the sdsl library, with support for rank/select
 */

#ifndef INCLUDED_HUFFMAN_STRING_HPP
#define INCLUDED_HUFFMAN_STRING_HPP

#include "definitions.hpp"

namespace bri {

class huffman_string {

public:
    huffman_string() {}

    huffman_string(std::string& s)
    {
        s.push_back(0);
        construct_im(wt, s.c_str(), 1);

        assert(wt.size()==s.size()-1);
    }

    uchar operator[](size_t i) 
    {

        assert(i<wt.size());
        return wt[i];

    }

    size_t size()
    {
        return wt.size();
    }

    ulint rank(size_t i, uchar c)
    {
        assert(i<=wt.size());
        return wt.rank(i,c);
    }

    ulint serialize(std::ostream& out)
    {
        return wt.serialize(out);
    }

    void load(std::istream& in)
    {
        wt.load(in);
    }

private:

    wt_huff<> wt;
    
};

};

#endif /* INCLUDED_HUFFMAN_STRING_HPP */