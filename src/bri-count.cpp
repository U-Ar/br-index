#include <iostream>
#include <chrono>

#include "br_index.hpp"
#include "utils.hpp"

using namespace bri;
using namespace std;

long allowed = 0;

void help()
{
	cout << "bri-count: count the number of occurrences of the input patterns" << endl;
    cout << "            allowing some mismatched characters."                 << endl << endl;

	cout << "Usage: bri-count [options] <index> <patterns>" << endl;
    cout << "   -m <number>  max number of mismatched characters allowed (supported: 0,1,2 (0 by default))" << endl;
	cout << "   <index>      index file (with extension .bri)" << endl;
	cout << "   <patterns>   file in pizza&chili format containing the patterns." << endl;
	exit(0);
}

void parse_args(char** argv, int argc, int &ptr){

	assert(ptr<argc);

	string s(argv[ptr]);
	ptr++;

    if(s.compare("-m")==0){

        if(ptr>=argc-1){
            cout << "Error: missing parameter after -m option." << endl;
            help();
        }

        char* e;
        allowed = strtol(argv[ptr],&e,10);

        if(*e != '\0' || allowed < 0 || allowed > 2){
            cout << "Error: invalid value not 0, 1, 2 after -m option." << endl;
            help();
        }

        ptr++;

	}else{

        cout << "Error: unknown option " << s << endl;
        help();

    }
}


void count_all(ifstream& in, string patterns)
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    string text;
    bool c = false;


    auto t1 = high_resolution_clock::now();

    br_index<> idx;

    idx.load(in);

    auto t2 = high_resolution_clock::now();

    cout << "searching patterns with mismatches fewer than " << allowed << " ... " << endl;
    ifstream ifs(patterns);

    //read header of the pizza&chilli input file
	//header example:
	//# number=7 length=10 file=genome.fasta forbidden=\n\t
    string header;
    getline(ifs,header);

    ulint n = get_number_of_patterns(header);
    ulint m = get_patterns_length(header);

    ulint last_perc = 0;

    ulint occ0 = 0, occ1 = 0, occ2 = 0;
    ulint occ_tot = 0;

    // extract patterns from file and search them in the index
    for (ulint i = 0; i < n; ++i)
    {
        ulint perc = 100 * i / n;
        if (perc > last_perc)
        {
            cout << perc << "% done ..." << endl;
            last_perc = perc;
        }

        string p = string();

        for (ulint j = 0; j < m; ++j)
        {
            char c;
            ifs.get(c);
            p += c;
        }

        // exact match
        ulint o0 = idx.count(p);
        occ0 += o0;
        occ_tot += o0;

        if (allowed < 1) continue;

        // approximate match with 1 miss
        ulint o1 = idx.count1(p);
        occ1 += o1;
        occ_tot += o1;

        if (allowed < 2) continue;

        ulint o2 = idx.count2(p);
        occ2 += o2;
        occ_tot += o2;

    }

    double occ_avg = (double)occ_tot / n;
    
    cout << endl << occ_avg << " average occurrences per pattern" << endl;

    ifs.close();

    auto t3 = high_resolution_clock::now();

    ulint load = duration_cast<milliseconds>(t2-t1).count();
    cout << "Load time  : " << load << " milliseconds" << endl;

    ulint search = duration_cast<milliseconds>(t3-t2).count();
    cout << "Number of patterns n = " << n << endl;
	cout << "Pattern length     m = " << m << endl;
    cout << "Occurrences with 0 mismatch   occ0 = " << occ0 << endl;
    if (allowed >= 1)
    {
        cout << "Occurrences with 1 mismatch   occ1 = " << occ1 << endl;
        if (allowed >= 2)
        {
            cout << "Occurrences with 2 mismatches occ2 = " << occ2 << endl;
        }
    }
	cout << "Total number of occurrences   occt = " << occ_tot << endl << endl;

    cout << "Total time : " << search << " milliseconds" << endl;
	cout << "Search time: " << (double)search/n << " milliseconds/pattern (total: " << n << " patterns)" << endl;
	cout << "Search time: " << (double)search/occ_tot << " milliseconds/occurrence (total: " << occ_tot << " occurrences)" << endl;
}



int main(int argc, char** argv)
{
    if (argc < 3) help();

    int ptr = 1;

    while (ptr < argc - 2) parse_args(argv, argc, ptr);

    string idx_file(argv[ptr]);
    string patt_file(argv[ptr+1]);

    ifstream in(idx_file);

    cout << "Loading br-index" << endl;
    count_all(in, patt_file);

    in.close();

}