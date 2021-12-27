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
#include "permuted_lcp.hpp"
#include "utils.hpp"

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

        // configure & build indexes for sufsort & plcp
        sdsl::cache_config cc;

        sdsl::int_vector<8> text(input.size());
        for (ulint i = 0; i < input.size(); ++i)
            text[i] = (uchar)input[i];

        sdsl::append_zero_symbol(text);

        // cache text
        sdsl::store_to_cache(text, sdsl::conf::KEY_TEXT, cc);
        sdsl::construct_config::byte_algo_sa = sais ? sdsl::SE_SAIS : sdsl::LIBDIVSUFSORT;
        
        // cache SA
        sdsl::construct_sa<8>(cc);
        
        // cache ISA 
        sdsl::construct_isa<8>(cc);

        auto bwt_and_samples = sufsort(text,cc);

        plcp = permuted_lcp<>(cc);

        // remove cache of text and SA
        sdsl::remove(sdsl::cache_file_name(sdsl::conf::KEY_TEXT, cc));
        sdsl::remove(sdsl::cache_file_name(sdsl::conf::KEY_SA, cc));




        // configure & build reversed indexes for sufsort
        std::reverse(input.begin(),input.end());

        sdsl::cache_config ccR;

        sdsl::int_vector<8> textR(input.size());
        for (ulint i = 0; i < input.size(); ++i)
            textR[i] = (uchar)input[i];

        sdsl::append_zero_symbol(textR);

        // cache textR
        sdsl::store_to_cache(textR, sdsl::conf::KEY_TEXT, ccR);
        sdsl::construct_config::byte_algo_sa = sais ? sdsl::SE_SAIS : sdsl::LIBDIVSUFSORT;
        
        // cache SAR
        sdsl::construct_sa<8>(ccR);
        
        // cache ISAR
        sdsl::construct_isa<8>(ccR);

        auto bwt_and_samplesR = sufsort(textR,ccR);
        // plcp is not needed in the reversed case

        // remove cache of textR and SAR
        sdsl::remove(sdsl::cache_file_name(sdsl::conf::KEY_TEXT, ccR));
        sdsl::remove(sdsl::cache_file_name(sdsl::conf::KEY_SA, ccR));





        std::string& bwt_s = std::get<0>(bwt_and_samples);
        std::vector<range_t>& samples_first_vec = std::get<1>(bwt_and_samples);
        std::vector<range_t>& samples_last_vec = std::get<2>(bwt_and_samples);

        std::string& bwt_sR = std::get<0>(bwt_and_samplesR);
        std::vector<range_t>& samples_first_vecR = std::get<1>(bwt_and_samplesR);
        std::vector<range_t>& samples_last_vecR = std::get<2>(bwt_and_samplesR);

        std::cout << "done.\n(2/3) Run length encoding BWT ... " << std::flush;


        // run length compression on BWT and BWTR
        bwt = rle_string_t(bwt_s);
        bwtR = rle_string_t(bwt_sR);

        // build F column (common between text and textR)
        F = std::vector<ulint>(256,0);

        for (uchar c : bwt_s) 
            F[c]++;

        for (ulint i = 255; i > 0; --i) 
            F[i] = F[i-1];

        F[0] = 0;

        for(ulint i = 1; i < 256; ++i) 
            F[i] += F[i-1];


        // remember BWT position of terminator
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

        int log_r = bitsize(r);
        int log_rR = bitsize(rR);
        int log_n = bitsize(bwt.size());

        std::cout << "Number of BWT equal-letter runs: r = " << r << std::endl;
		std::cout << "Rate n/r = " << double(bwt.size())/r << std::endl;
		std::cout << "log2(r) = " << std::log2(double(r)) << std::endl;
		std::cout << "log2(n/r) = " << std::log2(double(bwt.size())/r) << std::endl << std::endl;

        std::cout << "Number of BWT^R equal-letter runs: rR = " << rR << std::endl << std::endl;

        // Phi, Phi inverse is needed only in forward case
        std::cout << "(3/3) Building Phi/Phi^{-1} function ..." << std::flush;

        
        samples_last = int_vector<>(r,0,log_n);
        samples_first = int_vector<>(r,0,log_n);
        
        samples_lastR = int_vector<>(rR,0,log_n);

        for (ulint i = 0; i < r; ++i)
        {
            samples_last[i] = samples_last_vec[i].first;
            samples_first[i] = samples_first_vec[i].first;
        }
        for (ulint i = 0; i < rR; ++i)
            samples_lastR[i] = samples_last_vecR[i].first;

        // sort samples of first positions in runs according to text position
        std::sort(samples_first_vec.begin(), samples_first_vec.end());
        // sort samples of last positions in runs according to text position
        std::sort(samples.last_vec.begin(), samples_last_vec.end());

        // build Elias-Fano predecessor
        {
            std::vector<bool> first_bv(bwt_s.size(),false);
            for (auto p: samples_first_vec)
            {
                assert(p.first < pred_bv.size());
                first_bv[p.first] = true;
            }
            first = sparse_bitvector_t(first_bv);
        }
        {
            std::vector<bool> last_bv(bwt_s.size(),false);
            for (auto p: samples_last_vec)
            {
                assert(p.first < last_bv.size());
                last_bv[p.first] = true;
            }
            last = sparse_bitvector_t(last_bv);
        }

        assert(first.rank(first.size()) == r);
        assert(last.rank(last.size()) == r);

        inv_order = int_vector<>(r,0,log_n);
        
        inv_orderR = int_vector<>(rR,0,log_n);

        first_to_run = int_vector<>(r,0,log_r);

        last_to_run = int_vector<>(r,0,log_r);

        // construct first_to_run
        for (ulint i = 0; i < samples_first_vec.size(); ++i)
        {
            first_to_run[i] = samples_first_vec[i].second;
        }

        // construct last_to_run
        for (ulint i = 0; i < samples_last_vec.size(); ++i)
        {
            last_to_run[i] = samples_last_vec[i].second;
        }

        // construct inv_order
        {
            sdsl::int_vector_buffer<> isaR(sdsl::cache_file_name(sdsl::conf::KEY_ISA, ccR));
            for (ulint i = 0; i < samples_last.size(); ++i)
                inv_order[i] = isaR[bwt_s.size()-1-samples_last[i]];
        }

        // construct inv_orderR
        {
            sdsl::int_vector_buffer<> isa(sdsl::cache_file_name(sdsl::conf::KEY_ISA, cc));
            for (ulint i = 0; i < samples_lastR.size(); ++i)
                inv_orderR[i] = isa[bwt_s.size()-1-samples_lastR[i]];
        }

        // release ISA cache
        sdsl::remove(sdsl::cache_file_name(sdsl::conf::KEY_ISA, cc));
        sdsl::remove(sdsl::cache_file_name(sdsl::conf::KEY_ISA, ccR));

        
        std::cout << " done. " << std::endl << std::endl;

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
        // TODO
    }

    /*
     * reset the current searched pattern P
     */
    void reset_pattern()
    {
        // TODO
    }

    /*
     * search the pattern cP (P:the current pattern)
     * returns SA range corresponding to cP
     */
    range_t left_extension(uchar c)
    {
        // TODO
    }

    /*
     * search the pattern Pc (P:the current pattern)
     * return SA range corresponding to Pc
     */
    range_t right_extension(uchar c)
    {
        // TODO
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
    std::tuple<std::string, std::vector<range_t>, std::vector<range_t> > 
    sufsort(sdsl::int_vector<8>& text, sdsl::cache_config& cc)
    {
        std::string bwt_s;

        sdsl::int_vector_buffer<> sa(sdsl::cache_file_name(sdsl::conf::KEY_SA, cc));

        std::vector<range_t> samples_first;
        std::vector<range_t> samples_last;

        {
            for (ulint i = 0; i < sa.size(); ++i)
            {
                auto x = sa[i];

                assert(x <= text.size());

                if (x > 0) 
                    bwt_s.push_back((uchar)text[x-1]);
                else 
                    bwt_s.push_back(TERMINATOR);
                
                // insert samples at beginnings of runs
                if (i > 0)
                {
                    if (i==1 || (i>1 && bwt_s[i-1] != bwt_s[i-2]))
                    {
                        samples_first.push_back({
                            sa[i-1] > 0
                            ? sa[i-1] - 1
                            : sa.size() - 1,
                            samples_first.size()
                        });
                    }
                    if (i==sa.size()-1 && bwt_s[i] != bwt_s[i-1])
                    {
                        samples_first.push_back({
                            sa[i] > 0
                            ? sa[i] - 1
                            : sa.size() - 1,
                            samples_first.size()
                        });
                    }
                }

                // insert samples at ends of runs
                if (i > 0)
                {
                    if (bwt_s[i-1] != bwt_s[i])
                    {
                        samples_last.push_back({
                            sa[i-1] > 0
                            ? sa[i-1] - 1
                            : sa.size() - 1,
                            samples_last.size()
                        });
                    }
                    if (i == sa.size()-1)
                    {
                        samples_last.push_back({
                            sa[i] > 0
                            ? sa[i] - 1
                            : sa.size() - 1,
                            samples_last.size()
                        });
                    }
                }
            }
        }

        return std::tuple<std::string, std::vector<range_t>, std::vector<range_t> >
            (bwt_s, samples_first, samples_last);
    }

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

    // needed for left_extension
    sdsl::int_vector<> samples_last;
    sdsl::int_vector<> inv_order;
    
    // needed for Phi (SA[i] -> SA[i-1])
    sparse_bitvector_t first;
    sdsl::int_vector<> first_to_run;
    
    // needed for Phi^{-1} (SA[i] -> SA[i+1])
    sparse_bitvector_t last;
    sdsl::int_vector<> last_to_run;
    sdsl::int_vector<> samples_first;

    // needed for right_extension
    sdsl::int_vector<> samples_lastR;
    sdsl::int_vector<> inv_orderR;

    permuted_lcp<> plcp;

    /*
     * state variables for left_extension & right_extension
     * range: BWT range of P
     * p: sample pos in BWT
     * j: SA[p]
     * d: offset between starting position of the pattern & j
     * rangeR, pR, jR, dR: correspondents to left,right,p,j,d in BWT^R
     * len: current pattern length
     */
    range_t range;
    ulint p, j, d;
    range_t rangeR;
    ulint pR, jR, dR;
    ulint len;

};

};

#endif /* INCLUDED_BR_INDEX_HPP */