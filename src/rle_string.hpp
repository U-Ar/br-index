/*
 * rle_string.hpp
 *
 *  Original author: nicola
 *
 *  A run-length encoded string with rank/access functionalities.
 *
 *
 *  space of the structure: R * (H0 + log(n/R) + log(n/R)/B ) (1+o(1)) bits, n being text length,
 *  R number of runs, B block length, and H0 zero-order entropy of the run heads.
 *
 *  Time for all operations: O( B*(log(n/R)+H0) )
 *
 *  From the paper
 *
 *  Djamal Belazzougui, Fabio Cunial, Travis Gagie, Nicola Prezza and Mathieu Raffinot.
 *  Flexible Indexing of Repetitive Collections. Computability in Europe (CiE) 2017)
 *
 */

#ifndef INCLUDED_RLE_STRING_HPP
#define INCLUDED_RLE_STRING_HPP

#include "definitions.hpp"
#include "huffman_string.hpp"
#include "sparse_sd_vector.hpp"

namespace bri {

template<
    class sparse_bitvector_t = sparse_sd_vector,
    class string_t = huffman_string
>
class rle_string {

public:
    rle_string() {}

    /*
     * constructor
     * \param input the input string
     * \param B block size
     */
    rle_string(std::string& input, ulint B = 2)
    {
        assert(!contains0(input));

        this->B = B;
        n = input.size();
        r = 0;

        auto runs_per_letter_bv = std::vector<std::vector<bool> >(256);

        std::vector<bool> runs_bv;
        std::string run_heads_s;

        uchar last_c = input[0];

        for (ulint i = 1; i < input.size(); ++i)
        {
            if ((uchar)input[i] != last_c)
            {
                run_heads_s.push_back(last_c);
                runs_per_letter_bv[last_c].push_back(true);

                last_c = input[i];

                // push back a bit set only at the end of a block
                runs_bv.push_back(r%B == B-1);

                r++;

            } else {

                runs_bv.push_back(false);
                runs_per_letter_bv[last_c].push_back(false);

            }

            run_heads_s.push_back(last_c);
            runs_per_letter_bv[last_c].push_back(true);
            runs_bv.push_back(false);
            r++;

            assert(run_heads_s.size()==r);
		    assert(r==count_runs(input));
            assert(runs_bv.size()==input.size());

            ulint t = 0;
            for (ulint i = 0; i < 256; ++i) t += runs_per_letter_bv[i].size();
            assert(t == input.size());

            runs = sparse_bitvector_t(runs_bv);
            runs_per_letter = std::vector<sparse_bitvector_t>(256);
            for (ulint i = 0; i < 256; ++i)
                runs_per_letter[i] = sparse_bitvector_t(runs_per_letter_bv[i]);
            
            run_heads = string_t(run_heads_s);

            assert(run_heads.size() == r);

        }

        //WIP
        uchar operator[](ulint i)
        {
            assert(i < n);
            return run_heads[run_of(i).first];
        }
    }

private:

    bool contains0(std::string& s)
    {
        for (auto c: s) if (c==0) return true;
        return false;
    }

    ulint B = 0;

    sparse_bitvector_t runs;

    // runs for each letter
    std::vector<sparse_bitvector_t> runs_per_letter;

    // run heads with rank/select support
    string_t run_heads;

    // text length
    ulint n = 0;

    // number of runs
    ulint r = 0;

};

};

#endif