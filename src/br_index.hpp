/*
 * bidirectional r-index
 * 
 * created: 2021/12/22
 * author : U-Ar
 */

#ifndef INCLUDED_BR_INDEX_HPP
#define INCLUDED_BR_INDEX_HPP

#include "definitions.hpp"
#include "rle_string.hpp"
#include "sparse_sd_vector.hpp"

namespace bri {

template<
    class sparse_bitvector_t = sparse_sd_vector,
    class rle_string_t = rle_string_sd 
>
class br_index {

public:

    using triple = std::tuple<range_t, ulint, ulint>;

    br_index() {}

    br_index(std::string& input, bool sais = true)
    {

        this->sais = sais;

        if (contains_reserved_chars(input))
        {

            std::cout << "Error: input string contains one of the reserved characters 0x, 0x1" << std::endl;
            exit(1);
        
        }

        std::cout << "Text length = " << input.size() << std::endl << std::endl;

        std::cout << "(1/3) Building BWT, BWT^R and computing SA samples";
        if (sais) std::cout << " (SA-SAIS) ... " << std::flush;
        else std::cout << " (DIVSUFSORT) ... " << std::flush;

        // build RLBWT

        auto bwt_and_samples = sufsort(input);

        std::reverse(input.begin(),input.end());
        auto bwt_and_samplesR = sufsort(input);

        std::string& bwt_s = std::get<0>(bwt_and_samples);
        std::vector<std::pair<ulint,ulint> >& samples_first_vec = std::get<1>(bwt_and_samples);
        std::vector<ulint>& samples_last_vec = std::get<2>(bwt_and_samples);

        std::string& bwt_sR = std::get<0>(bwt_and_samplesR);
        std::vector<std::pair<ulint,ulint> >& samples_first_vecR = std::get<1>(bwt_and_samplesR);
        std::vector<ulint>& samples_last_vecR = std::get<2>(bwt_and_samplesR);

        std::cout << "done.\n(2/3) Run length encoding BWT ... " << std::flush;



        bwt = rle_string_t(bwt_s);
        bwtR = rle_string_t(bwt_sR);

        // build F column
        F = std::vector<ulint>(256,0);

        for (uchar c : bwt_s) 
            F[c]++;

        for (ulint i = 255; i > 0; --i) 
            F[i] = F[i-1];

        F[0] = 0;

        for(ulint i = 1; i < 256; ++i) 
            F[i] += F[i-1];

		for(ulint i = 0; i < bwt_s.size(); ++i)
			if(bwt_s[i]==TERMINATOR)
				terminator_position = i;
        
        for(ulint i = 0; i < bwt_sR.size(); ++i)
			if(bwt_sR[i]==TERMINATOR)
				terminator_positionR = i;

        assert(input.size() + 1 == bwt.size());

        std::cout << "done." << std::endl << std::endl;



        r = bwt.number_of_runs();
        rR = bwtR.number_of_runs();

        assert(samples_first_vec.size() == r);
        assert(samples_last_vec.size() == r);

        assert(samples_first_vecR.size() == rR);
        assert(samples_last_vecR.size() == rR);

        std::cout << "Number of BWT equal-letter runs: r = " << r << std::endl;
		std::cout << "Rate n/r = " << double(bwt.size())/r << std::endl;
		std::cout << "log2(r) = " << std::log2(double(r)) << std::endl;
		std::cout << "log2(n/r) = " << std::log2(double(bwt.size())/r) << std::endl << std::endl;

        std::cout << "Number of BWT^R equal-letter runs: rR = " << rR << std::endl << std::endl;

        // sort samples of first positions in runs according to text position
        std::sort(samples_first_vec.begin(), samples_first_vec.end());

        // build Elias-Fano predecessor
        {
            auto pred_bv = std::vector<bool>(bwt_s.size(),false);
            for (auto p: samples_first_vec)
            {
                assert(p.first < pred_bv.size());
                pred_bv[p.first] = true;
            }
            pred = sparse_bitvector_t(pred_bv);
        }
        {
            auto pred_bvR = std::vector<bool>(bwt_sR.size(),false);
            for (auto p: samples_first_vecR)
            {
                assert(p.first < pred_bvR.size());
                pred_bvR[p.first] = true;
            }
            predR = sparse_bitvector_t(pred_bvR);
        }

        assert(pred.rank(pred.size()) == r);
        assert(pred[pred.size()-1]);
        assert(predR.rank(predR.size()) == rR);
        assert(predR[predR.size()-1]);

        samples_last = int_vector<>()










        


        std::cout << " done. " << std::endl << std::endl;

        // TODO

    }

    /*
     * get full BWT range
     */
    range_t full_range()
    {
        return {0,bwt_size()-1};
    }

    /*
     * rn: BWT range of a string P
     * c:  character
     * returns: BWT range of cP
     */
    range_t LF(range_t rn, uchar c)
    {

        if ((c == 255 && F[c] == bwt.size()) || F[c] >= F[c+1]) return {1,0};

        ulint c_before = bwt.rank(rn.first, c);

        ulint c_inside = bwt.rank(rn.second+1,c) - c_before;

        if (c_inside == 0) return {1,0};

        ulint lb = F[c] + c_before;

        return {lb, lb + c_inside - 1};
    
    }

    /*
     * rn: BWT^R range of a string P
     * c:  character
     * returns: BWT^R range of cP
     */
    range_t LFR(range_t rn, uchar c)
    {

        if ((c == 255 && F[c] == bwt.size()) || F[c] >= F[c+1]) return {1,0};

        ulint c_before = bwtR.rank(rn.first, c);

        ulint c_inside = bwtR.rank(rn.second+1,c) - c_before;

        if (c_inside == 0) return {1,0};

        ulint lb = F[c] + c_before;

        return {lb, lb + c_inside - 1};

    }

    /*
     * Phi function
     * get SA[i] from SA[i+1]
     */
    ulint Phi(ulint i)
    {
        //TODO
    }
    /*
     * Phi inverse
     * get SA[i] from SA[i-1]
     */
    ulint PhiI(ulint i)
    {
        //TODO
    }

    ulint LF(ulint i)
    {
        auto c = bwt[i];
        return F[c] + bwt.rank(i,c);
    }

    ulint LFR(ulint i)
    {
        auto c = bwtR[i];
        return F[c] + bwtR.rank(i,c);
    }

    /*
     * inverse of LF (known as Psi)
     */
    ulint FL(ulint i)
    {

        // i-th character in first BWT column F
        auto c = F_at(i);

        // j: occurrences of c before i
        ulint j = i - F[c];

        return bwt.select(j,(uchar)c);

    }

    ulint FLR(ulint i)
    {

        // i-th character in first BWT column F
        auto c = F_at(i);

        // j: occurrences of c before i
        ulint j = i - F[c];

        return bwtR.select(j,(uchar)c);
        
    }

    /*
     * character of position i in column F
     */
    uchar F_at(ulint i)
    {

        ulint c = (std::upper_bound(F.begin(),F.end(),i) - F.begin()) - 1;
        assert(c < 256);
        assert(i >= F[c]);

        return (uchar)c;

    }

    /*
     * return BWT range of character c
     */
    range_t get_char_range(uchar c)
    {

        if ((c == 255 && F[c] == bwt_size()) || F[c] >= F[c+1]) return {1,0};

        ulint lb = F[c];
        ulint rb = bwt_size() - 1;

        if (c < 255) rb = F[c+1] - 1;

        return {lb,rb};

    }

    /*
     * return current BWT/BWT^R range 
     */
    range_t get_current_range(bool reversed = false)
    {

        if (!reversed) return {l,r};
        return {lR,rR};

    }

    /*
     * count occurrences of current pattern P
     */
    ulint count()
    {
        return (r + 1) - l;
    }

    /*
     * locate occurrences of current pattern P
     * return them as std::vector
     * (space comsuming if result is big)
     */
    std::vector<ulint> locate()
    {
        //TODO
    }

    /*
     * get BWT[i] or BWT^R[i]
     */
    uchar bwt_at(ulint i, bool reversed = false)
    {
        if (!reversed) return bwt[i];
        return bwtR[i];
    }

    /*
     * get number of runs in BWT
     */
    ulint number_of_runs(bool reversed = false)
    {
        if (!reversed) return bwt.number_of_runs();
        return bwtR.number_of_runs();
    }

    /*
     * get position of terminator symbol in BWT
     */
    ulint get_terminator_position(bool reversed = false)
    {
        if (!reversed) return terminator_position;
        return terminator_positionR;
    }

    /*
     * get string representation of BWT
     */
    std::string get_bwt(bool reversed = false)
    {
        if (!reversed) return bwt.to_string();
        return bwtR.to_string();
    }

    uint serialize(std::ostream& out)
    {
        // TODO
    }

    void load(std::istream& in)
    {
        // TODO
    }

    /*
     * save index to "{path_prefix}.bri" file
     */
    void save_to_file(std::string const& path_prefix)
    {

        std::string path = path_prefix.append(".bri");
        
        std::ofstream out(path);
        serialize(out);
        out.close();
    
    }

    /*
     * load index file from path
     */
    void load_from_file(std::string const& path)
    {

        std::ifstream in(path);
        load(in);
        in.close();

    }

    ulint text_size() { return bwt.size() - 1; }

    ulint bwt_size(bool reversed=false) { return bwt.size(); }

    uchar get_terminator() {
        return TERMINATOR;
    }

    /*
     * space complexity 
     */
    ulint print_space() {
        std::cout << "Number of runs in bwt : " << bwt.number_of_runs() << std::endl;
        std::cout << "Numbef of runs in bwtR: " << bwtR.number_of_runs() << std::endl << std::endl;
        ulint tot_bytes = bwt.print_space();
        tot_bytes += bwtR.print_space();
        std::cout << "\ntotal BWT space: " << tot_bytes << " bytes" << std::endl << std::endl;

        // TODO:calculate space for Phi, Phi^{-1}
        return tot_bytes;
    }

private:

    static bool contains_reserved_chars(std::string& s)
    {
        for (auto c: s)
        {
            if (c == 0 || c == 1) return true;
        }
        return false;
    }

    static const uchar TERMINATOR = 1;

    bool sais = true;

    /*
     * sparse RLBWT for both direction
     */
    std::vector<ulint> F;
    
    rle_string_t bwt;
    ulint terminator_position = 0;
    ulint r = 0;

    rle_string_t bwtR;
    ulint terminator_positionR = 0;
    ulint rR = 0;

    sparse_bitvector_t pred;
    sdsl::int_vector<> samples_last;
    sdsl::int_vector<> pred_to_run;

    sparse_bitvector_t predR;
    sdsl::int_vector<> samples_lastR;
    sdsl::int_vector<> pred_to_runR;

    /*
     * state variables for left_extension & right_extension
     * [left,right]: BWT range of P
     * p: sample pos in BWT
     * j: SA[p]
     * d: offset between starting position of the pattern & j
     * leftR, rightR, pR, jR, dR: correspondents to left,right,p,j,d in BWT^R
     * len: current pattern length
     */
    ulint left, right, p, j, d;
    ulint leftR, rightR, pR, jR, dR;
    ulint len;

};

};

#endif /* INCLUDED_BR_INDEX_HPP */