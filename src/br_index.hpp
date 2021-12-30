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

    /*
     * constructor. 
     * \param input: string on which br-index is built
     * \param sais: flag determining if we use SAIS for suffix sort. 
     *              otherwise we use divsufsort
     */
    br_index(std::string const& input, bool sais = true)
    {
        
        this->sais = sais;

        if (input.size() < 1)
        {

            std::cout << "Error: input string is empty" << std::endl;
            exit(1);

        }

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
        sdsl::construct_isa(cc);

        
        sdsl::int_vector_buffer<> sa(sdsl::cache_file_name(sdsl::conf::KEY_SA, cc));
        last_SA_val = sa[sa.size()-1];
        auto bwt_and_samples = sufsort(text,sa);

        plcp = permuted_lcp<>(cc);

        // remove cache of text and SA
        sdsl::remove(sdsl::cache_file_name(sdsl::conf::KEY_TEXT, cc));
        sdsl::remove(sdsl::cache_file_name(sdsl::conf::KEY_SA, cc));




        // configure & build reversed indexes for sufsort
        sdsl::cache_config ccR;

        sdsl::int_vector<8> textR(input.size());
        for (ulint i = 0; i < input.size(); ++i)
            textR[i] = (uchar)input[input.size()-1-i];

        sdsl::append_zero_symbol(textR);

        // cache textR
        sdsl::store_to_cache(textR, sdsl::conf::KEY_TEXT, ccR);
        sdsl::construct_config::byte_algo_sa = sais ? sdsl::SE_SAIS : sdsl::LIBDIVSUFSORT;
        
        // cache SAR
        sdsl::construct_sa<8>(ccR);
        // cache ISAR
        sdsl::construct_isa(ccR);

        sdsl::int_vector_buffer<> saR(sdsl::cache_file_name(sdsl::conf::KEY_SA, ccR));
        auto bwt_and_samplesR = sufsort(textR,saR);
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

        
        samples_last = sdsl::int_vector<>(r,0,log_n);
        samples_first = sdsl::int_vector<>(r,0,log_n);
        
        samples_firstR = sdsl::int_vector<>(rR,0,log_n);
        samples_lastR = sdsl::int_vector<>(rR,0,log_n);

        for (ulint i = 0; i < r; ++i)
        {
            samples_last[i] = samples_last_vec[i].first;
            samples_first[i] = samples_first_vec[i].first;
        }
        for (ulint i = 0; i < rR; ++i)
        {
            samples_lastR[i] = samples_last_vecR[i].first;
            samples_firstR[i] = samples_first_vecR[i].first;
        }

        // sort samples of first positions in runs according to text position
        std::sort(samples_first_vec.begin(), samples_first_vec.end());
        // sort samples of last positions in runs according to text position
        std::sort(samples_last_vec.begin(), samples_last_vec.end());

        // build Elias-Fano predecessor
        {
            std::vector<bool> first_bv(bwt_s.size(),false);
            for (auto p: samples_first_vec)
            {
                assert(p.first < first_bv.size());
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

        inv_order = sdsl::int_vector<>(r,0,log_n);
        
        inv_orderR = sdsl::int_vector<>(rR,0,log_n);

        first_to_run = sdsl::int_vector<>(r,0,log_r);

        last_to_run = sdsl::int_vector<>(r,0,log_r);

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
            //sdsl::int_vector_buffer<> isaR(sdsl::cache_file_name(sdsl::conf::KEY_ISA, ccR));
            sdsl::int_vector<> isaR;
            sdsl::load_from_file(isaR, sdsl::cache_file_name(sdsl::conf::KEY_ISA, ccR));
            assert(isaR.size() == bwt.size());
            for (ulint i = 0; i < samples_last.size(); ++i)
            {
                if (bwt.size() >= samples_last[i] + 2)
                    inv_order[i] = isaR[bwt.size()-2-samples_last[i]];
                else 
                    inv_order[i] = 0;
            }
        }

        // construct inv_orderR
        {
            //sdsl::int_vector_buffer<> isa(sdsl::cache_file_name(sdsl::conf::KEY_ISA, cc));
            sdsl::int_vector<> isa;
            sdsl::load_from_file(isa, sdsl::cache_file_name(sdsl::conf::KEY_ISA, cc));
            assert(isa.size() == bwt.size());
            for (ulint i = 0; i < samples_lastR.size(); ++i)
            {
                if (bwt.size() >= samples_lastR[i] + 2)
                    inv_orderR[i] = isa[bwt.size()-2-samples_lastR[i]];
                else 
                    inv_orderR[i] = 0;
            }
        }

        // release ISA cache
        sdsl::remove(sdsl::cache_file_name(sdsl::conf::KEY_ISA, cc));
        sdsl::remove(sdsl::cache_file_name(sdsl::conf::KEY_ISA, ccR));

        std::cout << " done. " << std::endl << std::endl;

        reset_pattern();
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
        assert(i != bwt.size() - 1);

        ulint jr = first.predecessor_rank_circular(i);

        assert(jr <= r - 1);

        ulint k = first.select(jr);

        assert(jr < r - 1 || k == bwt.size() - 1);

        // distance from predecessor
        ulint delta = k < i ? i - k : i + 1;

        // check if Phi(SA[0]) is not called
        assert(first_to_run[jr] > 0);

        ulint prev_sample = samples_last[first_to_run[jr]-1];

        return (prev_sample + delta) % bwt.size();
    }
    /*
     * Phi inverse
     * get SA[i] from SA[i-1]
     */
    ulint PhiI(ulint i)
    {
        assert(i != last_SA_val);

        ulint jr = last.predecessor_rank_circular(i);

        assert(jr <= r - 1);

        ulint k = last.select(jr);

        assert(jr < r - 1 || k == bwt.size() - 1);

        // distance from predecessor
        ulint delta = k < i ? i - k : i + 1;

        // check if Phi(SA[0]) is not called
        assert(last_to_run[jr] < r-1);

        ulint prev_sample = samples_first[last_to_run[jr]+1];

        return (prev_sample + delta) % bwt.size();
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

        if (!reversed) return range;
        return rangeR;

    }

    /*
     * count occurrences of current pattern P
     */
    ulint count()
    {
        return (range.second + 1) - range.first;
    }

    /*
     * locate occurrences of current pattern P
     * return them as std::vector
     * (space comsuming if result is big)
     */
    std::vector<ulint> locate()
    {
        assert(j >= d);

        ulint sa = j - d;
        ulint pos = sa;

        std::deque<ulint> deq;
        deq.push_back(pos);

        while (plcp[pos] >= len) 
        {
            pos = Phi(pos);
            deq.push_front(pos);
        }
        pos = sa;
        while (true)
        {
            if (pos == last_SA_val) break;
            pos = PhiI(pos);
            if (plcp[pos] < len) break;
            deq.push_back(pos);
        }

        return std::vector<ulint>(deq.begin(),deq.end());
    }

    /*
     * reset the current searched pattern P
     */
    void reset_pattern()
    {
        // entire SA range
        range = full_range();
        // lex order 0
        p = 0;
        // SA[0] = n - 1
        j = bwt_size() - 1;
        // offset 0
        d = 0;

        // entire SAR range
        rangeR = full_range();
        // reversed sample is initialized with 0 (not used instantly)
        pR = jR = dR = 0;

        // null pattern
        len = 0;
    }

    /*
     * get the length of current searched pattern P
     */
    ulint pattern_length()
    {
        return len;
    }

    /*
     * search the pattern cP (P:the current pattern)
     * returns SA range corresponding to cP
     */
    range_t left_extension(uchar c)
    {
        range_t prev_range(range);

        // get SA range of cP
        range = LF(range,c);

        // pattern cP was not found
        if (range.first > range.second) return {1,0};

        // accumulated occ of aP (for any a s.t. a < c)
        ulint acc = 0;

        for (ulint a = 0; a < c; ++a)
        {
            range_t smaller_range = LF(prev_range,(uchar)a);
            acc += (smaller_range.second+1) - smaller_range.first;
        }

        // get SAR range of (cP)^R
        rangeR.second = rangeR.first + acc + range.second - range.first;
        rangeR.first = rangeR.first + acc;

        // cP and aP occurs for some a s.t. a != c
        if (prev_range.second-prev_range.first != 
            range.second-range.first)
        {
            // fint last c in range and get its sample
            // there must be at least one c due to the previous if clause
            ulint rnk = bwt.rank(prev_range.second+1,c);
            assert(rnk > 0);

            // update p by corresponding BWT position
            p = bwt.select(rnk-1,c);
            assert(p >= prev_range.first && p <= prev_range.second);

            // run number of position p
            ulint run_of_p = bwt.run_of_position(p);

            // update j by SA[p]
            if (bwt[prev_range.second] == c)
                j = samples_first[run_of_p];
            else
                j = samples_last[run_of_p];

            // reset d
            d = 0;

            // lex order in SAR of position j
            pR = inv_order[run_of_p];

            // SAR[pR]
            jR = bwt.size()-2-j;

            // reset dR
            dR = len;
        }
        else // only c precedes P 
        {
            d++;
        }
        len++;
        return range;
    }

    /*
     * search the pattern Pc (P:the current pattern)
     * return SA range corresponding to Pc
     */
    range_t right_extension(uchar c)
    {
        range_t prev_rangeR(rangeR);

        // get SAR range of Pc
        rangeR = LFR(rangeR,c);

        // pattern Pc was not found
        if (rangeR.first > rangeR.second) return {1,0};

        // accumulated occ of Pa (for any a s.t. a < c)
        ulint acc = 0;

        for (ulint a = 0; a < c; ++a)
        {
            range_t smaller_rangeR = LFR(prev_rangeR,(uchar)a);
            acc += (smaller_rangeR.second+1) - smaller_rangeR.first;
        }

        // get SA range of Pc
        range.second = range.first + acc + rangeR.second - rangeR.first; 
        range.first = range.first + acc;

        // Pc and Pa occurs for some a s.t. a != c
        if (prev_rangeR.second-prev_rangeR.first != 
            rangeR.second-rangeR.first)
        {
            // fint last c in range and get its sample
            // there must be at least one c due to the previous if clause
            ulint rnk = bwtR.rank(prev_rangeR.second+1,c);
            assert(rnk > 0);

            // update pR by corresponding BWTR position
            pR = bwtR.select(rnk-1,c);
            assert(pR >= prev_rangeR.first && pR <= prev_rangeR.second);

            // run number of position pR
            ulint run_of_pR = bwtR.run_of_position(pR);

            // update jR by SAR[pR]
            if (bwtR[prev_rangeR.second] == c)
                jR = samples_firstR[run_of_pR];
            else
                jR = samples_lastR[run_of_pR];

            // reset dR
            dR = 0;

            // lex order in SA of position jR
            p = inv_orderR[run_of_pR];

            // SA[p]
            j = bwt.size()-2-jR;

            // reset d
            d = len;
        }
        else 
        {
            dR++;
        }
        len++;
        return range;
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
        ulint w_bytes = 0;

        out.write((char*)&terminator_position,sizeof(terminator_position));
        out.write((char*)&terminator_positionR,sizeof(terminator_positionR));
        out.write((char*)&last_SA_val,sizeof(last_SA_val));
        out.write((char*)F.data(),256*sizeof(ulint));

        w_bytes += sizeof(terminator_position)
                   + sizeof(terminator_positionR)
                   + sizeof(last_SA_val)
                   + 256*sizeof(ulint);
        
        w_bytes += bwt.serialize(out);
        w_bytes += bwtR.serialize(out);

        w_bytes += samples_first.serialize(out);
        w_bytes += samples_last.serialize(out);
        w_bytes += inv_order.serialize(out);

        w_bytes += first.serialize(out);
        w_bytes += first_to_run.serialize(out);

        w_bytes += last.serialize(out);
        w_bytes += last_to_run.serialize(out);

        w_bytes += samples_firstR.serialize(out);
        w_bytes += samples_lastR.serialize(out);
        w_bytes += inv_orderR.serialize(out);

        w_bytes += plcp.serialize(out);

        return w_bytes;
    
    }

    void load(std::istream& in)
    {
        
        in.read((char*)&terminator_position,sizeof(terminator_position));
        in.read((char*)&terminator_positionR,sizeof(terminator_positionR));
        in.read((char*)&last_SA_val,sizeof(last_SA_val));
        
        F = std::vector<ulint>(256);
        in.read((char*)F.data(),256*sizeof(ulint));

        bwt.load(in);
        bwtR.load(in);
        r = bwt.number_of_runs();
        rR = bwtR.number_of_runs();

        samples_first.load(in);
        samples_last.load(in);
        inv_order.load(in);

        first.load(in);
        first_to_run.load(in);

        last.load(in);
        last_to_run.load(in);

        samples_firstR.load(in);
        samples_lastR.load(in);
        inv_orderR.load(in);

        plcp.load(in);

    }

    /*
     * save index to "{path_prefix}.bri" file
     */
    void save_to_file(std::string const& path_prefix)
    {

        std::string path = path_prefix + ".bri";
        
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
     * get statistics
     */
    ulint print_space() {
        std::cout << "Number of runs in bwt : " << bwt.number_of_runs() << std::endl;
        std::cout << "Numbef of runs in bwtR: " << bwtR.number_of_runs() << std::endl << std::endl;
        ulint tot_bytes = bwt.print_space();
        tot_bytes += bwtR.print_space();
        std::cout << "\ntotal BWT space: " << tot_bytes << " bytes" << std::endl << std::endl;

        tot_bytes += plcp.print_space();
        std::cout << std::endl;

        std::ofstream out("/dev/null");

        ulint bytes = 0;

        
        bytes =  samples_first.serialize(out);
        tot_bytes += bytes;
        std::cout << "samples_first: " << bytes << std::endl;

        bytes =  samples_last.serialize(out);
        tot_bytes += bytes;
        std::cout << "samples_last: " << bytes << std::endl;

        bytes =  inv_order.serialize(out);
        tot_bytes += bytes;
        std::cout << "inv_order: " << bytes << std::endl;


        bytes =  first.serialize(out);
        tot_bytes += bytes;
        std::cout << "first: " << bytes << std::endl;

        bytes =  first_to_run.serialize(out);
        tot_bytes += bytes;
        std::cout << "first_to_run: " << bytes << std::endl;


        bytes =  last.serialize(out);
        tot_bytes += bytes;
        std::cout << "last: " << bytes << std::endl;

        bytes =  last_to_run.serialize(out);
        tot_bytes += bytes;
        std::cout << "last_to_run: " << bytes << std::endl;


        bytes =  samples_firstR.serialize(out);
        tot_bytes += bytes;
        std::cout << "samples_firstR: " << bytes << std::endl;

        bytes =  samples_lastR.serialize(out);
        tot_bytes += bytes;
        std::cout << "samples_lastR: " << bytes << std::endl;

        bytes =  inv_orderR.serialize(out);
        tot_bytes += bytes;
        std::cout << "inv_orderR: " << bytes << std::endl << std::endl;


        return tot_bytes;
    }

    /*
     * get space complexity
     */
    ulint get_space()
    {
        ulint tot_bytes = bwt.get_space();
        tot_bytes += bwtR.get_space();

        tot_bytes += plcp.get_space();

        std::ofstream out("/dev/null");

        tot_bytes += samples_first.serialize(out);
        tot_bytes += samples_last.serialize(out);
        tot_bytes += inv_order.serialize(out);

        tot_bytes += first.serialize(out);
        tot_bytes += first_to_run.serialize(out);

        tot_bytes += last.serialize(out);
        tot_bytes += last_to_run.serialize(out);

        tot_bytes += samples_firstR.serialize(out);
        tot_bytes += samples_lastR.serialize(out);
        tot_bytes += inv_orderR.serialize(out);

        return tot_bytes;
    }

private:
    std::tuple<std::string, std::vector<range_t>, std::vector<range_t> > 
    sufsort(sdsl::int_vector<8>& text, sdsl::int_vector_buffer<>& sa)
    {
        std::string bwt_s;
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

    static bool contains_reserved_chars(std::string const& s)
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
     * sparse RLBWT for text & textR
     */
    std::vector<ulint> F;
    
    rle_string_t bwt;
    ulint terminator_position = 0;
    ulint last_SA_val = 0;
    ulint r = 0;

    rle_string_t bwtR;
    ulint terminator_positionR = 0;
    ulint rR = 0;

    // needed for left_extension
    sdsl::int_vector<> samples_first;
    sdsl::int_vector<> samples_last;
    sdsl::int_vector<> inv_order;
    
    // needed for Phi (SA[i] -> SA[i-1])
    sparse_bitvector_t first;
    sdsl::int_vector<> first_to_run;
    
    // needed for Phi^{-1} (SA[i] -> SA[i+1])
    sparse_bitvector_t last;
    sdsl::int_vector<> last_to_run;

    // needed for right_extension
    sdsl::int_vector<> samples_firstR;
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