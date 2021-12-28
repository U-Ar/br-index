#include <iostream>
#include <chrono>

#include "br_index.hpp"
#include "utils.hpp"

using namespace bri;
using namespace std;

string check = string();

void help()
{
	cout << "bri-locate: locate all occurrences of the input patterns." << endl << endl;

	cout << "Usage: bri-locate [options] <index> <patterns>" << endl;
	cout << "   -c <text>    check correctness of each pattern occurrence on this text file (must be the same indexed)" << endl;
	cout << "   <index>      index file (with extension .bri)" << endl;
	cout << "   <patterns>   file in pizza&chili format containing the patterns." << endl;
	exit(0);
}

void parse_args(char** argv, int argc, int &ptr){

	assert(ptr<argc);

	string s(argv[ptr]);
	ptr++;

	if(s.compare("-c")==0){

		if(ptr>=argc-1){
			cout << "Error: missing parameter after -c option." << endl;
			help();
		}

		check = string(argv[ptr]);
		ptr++;

	}else{

		cout << "Error: unknown option " << s << endl;
		help();

	}

}


void locate_all(ifstream& in, string patterns)
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    string text;
    bool c = false;

    if (check.compare(string()) != 0)
    {
        c = true;

        ifstream ifs1(check);
        stringstream ss;
        ss << ifs1.rdbuf();
        text = ss.str();
    }

    auto t1 = high_resolution_clock::now();

    br_index<> idx;

    idx.load(in);

    auto t2 = high_resolution_clock::now();

    cout << "searching patterns ... " << endl;
    ifstream ifs(patterns);

    //read header of the pizza&chilli input file
	//header example:
	//# number=7 length=10 file=genome.fasta forbidden=\n\t
    string header;
    getline(ifs,header);

    ulint n = get_number_of_patterns(header);
    ulint m = get_patterns_length(header);

    ulint last_perc = 0;

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

        idx.reset_pattern();
        range_t range{1,0};

        for (ulint j = 0; j < m; ++j)
        {
            range = idx.right_extension((uchar)p[j]);
            if (range.first > range.second) break;
        }

        vector<ulint> occs;

        if (range.first <= range.second) occs = idx.locate();

        occ_tot += (range.second + 1) - range.first;

        if (c) // check occurrences
        {
            for (auto o : occs)
            {
                if (text.substr(o,p.size()).compare(p) != 0) 
                {
                    cout << "Error: wrong occurrence: " << o << endl;
                    exit(0);
                }
            }
        }
    }

    double occ_avg = (double)occ_tot / n;
    
    cout << endl << occ_avg << " average occurrences per pattern" << endl;

    ifs.close();

    auto t3 = high_resolution_clock::now();

    ulint load = duration_cast<milliseconds>(t2-t1).count();
    cout << "Load time: " << load << " milliseconds" << endl;

    ulint search = duration_cast<milliseconds>(t3-t2).count();
    cout << "Number of patterns n = " << n << endl;
	cout << "Pattern length m = " << m << endl;
	cout << "Total number of occurrences  occ_t = " << occ_tot << endl << endl;

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
    locate_all(in, patt_file);

    in.close();

}