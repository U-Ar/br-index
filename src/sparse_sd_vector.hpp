/*
 * sparse_sd_vector: a wrapper on sd_vector of the sdsl library, with support for rank/select1
 */

#ifndef INCLUDED_SPARSE_SD_VECTOR_HPP
#define INCLUDED_SPARSE_SD_VECTOR_HPP

#include "definitions.hpp"

#ifndef ulint
typedef uint64_t ulint;
#endif

#ifndef uint
typedef uint32_t uint;
#endif 

namespace bri {

class sparse_sd_vector{
public:

    /*
     * empty contructor. all bits are 0
     */
    sparse_sd_vector(){}

    /*
     * constructor. build using std::vector<bool>
     */
    sparse_sd_vector(std::vector<bool> &b)
    {
        if (b.size() == 0) return;

        u = b.size();

        sdsl::bit_vector bv(b.size());
        for (size_t i = 0; i < b.size(); ++i) bv[i] = b[i];

        sdv = sdsl::sd_vector<>(bv);
        rank1 = sdsl::sd_vector<>::rank_1_type(&sdv);
        select1 = sdsl::sd_vector<>::select_1_type(&sdv);

    }

    /*
     * constructor. build using bit_vector
     */
    sparse_sd_vector(sdsl::bit_vector& bv)
    {
        sdv = sdsl::sd_vector<>(bv);
        rank1 = sdsl::sd_vector<>::rank_1_type(&sdv);
        select1 = sdsl::sd_vector<>::select_1_type(&sdv);
    }

    /*
     * substitution operator.
     */
    sparse_sd_vector& operator=(sparse_sd_vector& other)
    {
        u = other.size();
        sdv = sdsl::sd_vector<>(other.raw_vector());
        rank1 = sdsl::sd_vector<>::rank_1_type(&sdv);
        select1 = sdsl::sd_vector<>::select_1_type(&sdv);
    }

    sdsl::sd_vector<>& raw_vector()
    {
        return sdv;
    }

    /*
     * argument: position i 
     * returns: bit in position i
     * ACCESS ONLY
     */
    bool operator[](size_t i) 
    {
        assert(i < size());
        return sdv[i];
    }

    bool at(size_t i)
    {
        return operator[](i);
    }

    /*
     * argument: position i 
     * returns: number of 1-bits in sdv[0...i-1]
     */
    ulint rank(size_t i)
    {
        assert(i <= size());

        return rank1(i);
    }

    /*
	 * argument: position 0<=i<=n
	 * returns: predecessor of i (position i excluded)
	 */
    size_t predecessor(size_t i)
    {
        assert(rank(i)>0);

        return select(rank(i) - 1);
    }
    
    ulint gap_at(size_t i)
    {
        assert(i<number_of_1());

        if (i == 0) return select(0) + 1;

        return select(i) - select(i-1);
    }

    /*
	 * argument: ulint i >= 0
	 * returns: position of the i-th 1-bit
	 */
    size_t select(ulint i)
    {
        assert(i<number_of_1());
        
        return select1(i+1);
    }

    /*
     * returns: size of the bitvector
     */
    ulint size() { return u; }

    /*
     * returns: number of 1s in the bitvector
     */
    ulint number_of_1() { return rank1(size()); }

    /*
     * argument: ostream
     * returns: number of bytes written to ostream
     */
    ulint serialize(std::ostream& out)
    {
        ulint w_bytes = 0;

        out.write((char*)&u, sizeof(u));
        w_bytes += sizeof(u);

        if (u == 0) return w_bytes;

        w_bytes += sdv.serialize(out);
        return w_bytes;
    }

    /*
     * load bitvector from istream
     * argument: istream
     */
    void load(std::istream& in)
    {
        in.read((char*)&u, sizeof(u));

        if (u == 0) return;

        sdv.load(in);
        rank1 = sdsl::sd_vector<>::rank_1_type(&sdv);
        select1 = sdsl::sd_vector<>::select_1_type(&sdv);
    }

private:

    //length of bitvector
    ulint u = 0;

    sdsl::sd_vector<> sdv;
    sdsl::sd_vector<>::rank_1_type rank1;
    sdsl::sd_vector<>::select_1_type select1;

};

};

#endif /* INCLUDED_SPARSE_SD_VECTOR_HPP */