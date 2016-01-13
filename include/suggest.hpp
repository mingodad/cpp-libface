#if !defined LIBFACE_SUGGEST_HPP
#define LIBFACE_SUGGEST_HPP

#ifdef USE_USTL
#include <ustl.h>
namespace nw=ustl;
#else
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <string>
#include <queue>
namespace nw=std;
#endif // USE_USTL
#include <stdio.h>
#include <assert.h>

#include <include/utils.hpp>
#include <include/types.hpp>


struct PhraseRange {
    // first & last are both inclusive of the range. i.e. The range is
    // [first, last] and NOT [first, last)
    uint_t first, last;

    // weight is the value of the best solution in the range [first,
    // last] and index is the index of this best solution in the
    // original array of strings.
    uint_t weight, index;

    PhraseRange(uint_t f, uint_t l, uint_t w, uint_t i)
        : first(f), last(l), weight(w), index(i)
    { }

    bool
    operator<(PhraseRange const &rhs) const {
        return this->weight < rhs.weight;
    }
};

typedef nw::priority_queue<PhraseRange> pqpr_t;

#define DEFAULT_SUGGEST_GROUP_KEY "__default__"
#define EMPTY_SUGGEST_GROUP_KEY "__empty__"

struct SuggestGroup {
    PhraseMap pm;                   // Phrase Map (usually a sorted array of strings)
    RMQ st;                         // An instance of the RMQ Data Structure

    bool is_input_sorted;

    SuggestGroup() : pm(), st(), is_input_sorted(true)
    { }

    // should be called after initilizaing phrase map
    size_t
    initialize_RMQ() {
        vui_t weights;
        for (size_t i = 0; i < pm.repr.size(); ++i) {
            weights.push_back(pm.repr[i].weight);
        }
        st.initialize(weights);
        return weights.size();
    }

    static nw::vector<nw::string>
    split(nw::string const& str, char delim) {
        nw::vector<nw::string> elems;
        nw::istringstream ss(str);
        nw::string elem;

        while (nw::getline(ss, elem, delim)) {
            elems.push_back(elem);
        }
        return elems;
    }

    /*
     * Generates the key to namespace suggest groups from given input string
     *
     * The input string should be of the following format:
     *  1) key-value pairs separated by ':'
     *  2) multiple key-value pairs separated by ','
     * e.g. 'k1:v1,k2:v2'
     *
     * Output key will be concatenation of value parts separated by ':'
     * e.g. 'v1:v2'
     *
     * If input is empty, output key will be DEFAULT_SUGGEST_GROUP_KEY
     */
    static nw::string
    parse_key(nw::string const& group_namespace) {
        nw::vector<nw::string> elems = split(group_namespace, ',');

        nw::string group_key;
        for (size_t i = 0; i < elems.size(); ++i) {
            nw::vector<nw::string> kv = split(elems[i], ':');
            assert(2 == kv.size());

            group_key.append(kv[1]);
            group_key.append(":");
        }

        if (group_key.empty()) {
            group_key = DEFAULT_SUGGEST_GROUP_KEY;
        }
        return group_key;
    }

    static nw::string
    build_key(nw::string const& countryCode) {
        if (!countryCode.empty()) {
            return countryCode + ":";
        }
        else {
            return "";
        }
    }
};

namespace _suggest_group {
    int
    test() {
        nw::string result0 = SuggestGroup::parse_key("k1:v1");
        assert("v1:" == result0);

        nw::string result1 = SuggestGroup::parse_key("k1:v1,k2:v2,k3:v3");
        assert("v1:v2:v3:" == result1);

        nw::string result2 = SuggestGroup::parse_key("");
        assert("__default__" == result2);

        nw::string result3 = SuggestGroup::build_key("v1");
        assert("v1:" == result3);

        nw::string result4 = SuggestGroup::build_key("");
        assert("" == result4);

        return 0;
    }
}

vp_t
suggest(PhraseMap &pm, RMQ &st, nw::string prefix, uint_t n = 16) {
    pvpi_t phrases = pm.query(prefix);
    // cerr<<"Got "<<phrases.second - phrases.first<<" candidate phrases from PhraseMap"<<nw::endl;

    uint_t first = phrases.first  - pm.repr.begin();
    uint_t last  = phrases.second - pm.repr.begin();

    if (first == last) {
        return vp_t();
    }

    vp_t ret;
    --last;

    pqpr_t heap;
    pui_t best = st.query_max(first, last);
    heap.push(PhraseRange(first, last, best.first, best.second));

    while (ret.size() < n && !heap.empty()) {
        PhraseRange pr = heap.top();
        heap.pop();
        // cerr<<"Top phrase is at index: "<<pr.index<<nw::endl;
        // cerr<<"And is: "<<pm.repr[pr.index].first<<nw::endl;

        ret.push_back(pm.repr[pr.index]);

        uint_t lower = pr.first;
        uint_t upper = pr.index - 1;

        // Prevent underflow
        if (pr.index - 1 < pr.index && lower <= upper) {
            // cerr<<"[1] adding to heap: "<<lower<<", "<<upper<<", "<<best.first<<", "<<best.second<<nw::endl;

            best = st.query_max(lower, upper);
            heap.push(PhraseRange(lower, upper, best.first, best.second));
        }

        lower = pr.index + 1;
        upper = pr.last;

        // Prevent overflow
        if (pr.index + 1 > pr.index && lower <= upper) {
            // cerr<<"[2] adding to heap: "<<lower<<", "<<upper<<", "<<best.first<<", "<<best.second<<nw::endl;

            best = st.query_max(lower, upper);
            heap.push(PhraseRange(lower, upper, best.first, best.second));
        }
    }

    return ret;
}

vp_t
naive_suggest(PhraseMap& pm, RMQ& st, nw::string prefix, uint_t n = 16) {
    pvpi_t phrases = pm.query(prefix);
    nw::vector<uint_t> indexes;
    vp_t ret;

    while (phrases.first != phrases.second) {
        indexes.push_back(phrases.first - pm.repr.begin());
        ++phrases.first;
    }

    while (ret.size() < n && !indexes.empty()) {
        uint_t mi = 0;
        for (size_t i = 1; i < indexes.size(); ++i) {
            if (pm.repr[indexes[i]].weight > pm.repr[indexes[mi]].weight) {
                mi = i;
            }
        }
        ret.push_back(pm.repr[indexes[mi]]);
        indexes.erase(indexes.begin() + mi);
    }
    return ret;
}

namespace _suggest {
    int
    test() {
        PhraseMap pm;
        pm.insert(1, "duckduckgo", "");
        pm.insert(2, "duckduckgeese", "");
        pm.insert(1, "duckduckgoose", "");
        pm.insert(9, "duckduckgoo", "");
        pm.insert(10, "duckgo", "");
        pm.insert(3, "dukgo", "");
        pm.insert(2, "luckkuckgo", "");
        pm.insert(5, "chuckchuckgo", "");
        pm.insert(15, "dilli - no one killed jessica", "");
        pm.insert(11, "aaitbaar - no one killed jessica", "");

        pm.finalize();

        RMQ st;
        vui_t weights;
        for (size_t i = 0; i < pm.repr.size(); ++i) {
            weights.push_back(pm.repr[i].weight);
        }

        st.initialize(weights);
#ifndef USE_USTL
        nw::cout<<"\n";
        nw::cout<<"suggest(\"d\"):\n"<<suggest(pm, st, "d")<<nw::endl;
        nw::cout<<"naive_suggest(\"d\"):\n"<<naive_suggest(pm, st, "d")<<nw::endl;

        nw::cout<<"\n";
        nw::cout<<"suggest(\"a\"):\n"<<suggest(pm, st, "a")<<nw::endl;
        nw::cout<<"naive_suggest(\"a\"):\n"<<naive_suggest(pm, st, "a")<<nw::endl;

        nw::cout<<"\n";
        nw::cout<<"suggest(\"b\"):\n"<<suggest(pm, st, "b")<<nw::endl;
        nw::cout<<"naive_suggest(\"b\"):\n"<<naive_suggest(pm, st, "b")<<nw::endl;

        nw::cout<<"\n";
        nw::cout<<"suggest(\"duck\"):\n"<<suggest(pm, st, "duck")<<nw::endl;
        nw::cout<<"naive_suggest(\"duck\"):\n"<<naive_suggest(pm, st, "duck")<<nw::endl;

        nw::cout<<"\n";
        nw::cout<<"suggest(\"k\"):\n"<<suggest(pm, st, "k")<<nw::endl;
        nw::cout<<"naive_suggest(\"k\"):\n"<<naive_suggest(pm, st, "k")<<nw::endl;

        nw::cout<<"\n";
        nw::cout<<"suggest(\"ka\"):\n"<<suggest(pm, st, "ka")<<nw::endl;
        nw::cout<<"naive_suggest(\"ka\"):\n"<<naive_suggest(pm, st, "ka")<<nw::endl;

        nw::cout<<"\n";
        nw::cout<<"suggest(\"c\"):\n"<<suggest(pm, st, "c")<<nw::endl;
        nw::cout<<"naive_suggest(\"c\"):\n"<<naive_suggest(pm, st, "c")<<nw::endl;
#endif // USE_USTL

        return 0;
    }
}

#endif // LIBFACE_PHRASE_MAP_HPP
