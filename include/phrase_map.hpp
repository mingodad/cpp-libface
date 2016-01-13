#if !defined LIBFACE_PHRASE_MAP_HPP
#define LIBFACE_PHRASE_MAP_HPP

#ifdef USE_USTL
#include <ustl.h>
namespace nw=ustl;
#else
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <string>
namespace nw=std;
#endif // USE_USTL

#include <stdio.h>
#include <assert.h>

#include <include/types.hpp>



struct PrefixFinder {
    bool
    operator()(nw::string const& prefix, phrase_t const &target) {
#if 1
        const int ppos = target.phrase.compare(0, prefix.size(), prefix);
        return ppos > 0;
#else
        const int ppos = target.phrase.find(prefix);
        if (!ppos) {
            return false;
        }
        return prefix < target.phrase;
#endif
    }

    bool
    operator()(phrase_t const& target, nw::string const &prefix) {
#if 1
        const int ppos = target.phrase.compare(0, prefix.size(), prefix);
        return ppos < 0;
#else
        const int ppos = target.phrase.find(prefix);
        if (!ppos) {
            return false;
        }
        return target.phrase < prefix;
#endif
    }
};

class PhraseMap {
public:
    vp_t repr;

public:
    PhraseMap(uint_t _len = 15000000) {
        this->repr.reserve(_len);
    }

    void
    insert(uint_t weight, nw::string const& p, StringProxy const& s) {
        this->repr.push_back(phrase_t(weight, p, s));
    }

    void
    finalize(int sorted = 0) {
        if (!sorted) {
            nw::sort(this->repr.begin(), this->repr.end());
        }
    }

    pvpi_t
    query(nw::string const &prefix) {
        return nw::equal_range(this->repr.begin(), this->repr.end(),
                                prefix, PrefixFinder());
    }

};


pvpi_t
naive_query(PhraseMap &pm, nw::string prefix) {
    vpi_t f = pm.repr.begin(), l = pm.repr.begin();
    while (f != pm.repr.end() && f->phrase.substr(0, prefix.size()) < prefix) {
        ++f;
    }
    l = f;
    while (l != pm.repr.end() && l->phrase.substr(0, prefix.size()) == prefix) {
        ++l;
    }
    return nw::make_pair(f, l);
}

void
show_indexes(PhraseMap &pm, nw::string prefix) {
    pvpi_t nq = naive_query(pm, prefix);
    pvpi_t q  = pm.query(prefix);

    nw::cout<<"naive[first] = "<<nq.first - pm.repr.begin()<<", naive[last] = "<<nq.second - pm.repr.begin()<<nw::endl;
    nw::cout<<"phmap[first] = "<<q.first - pm.repr.begin()<<", phmap[last] = "<<q.second - pm.repr.begin()<<nw::endl;
}


namespace phrase_map {
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

        show_indexes(pm, "a");
        assert(naive_query(pm, "a") == pm.query("a"));

        show_indexes(pm, "b");
        assert(naive_query(pm, "b") == pm.query("b"));

        show_indexes(pm, "c");
        assert(naive_query(pm, "c") == pm.query("c"));

        show_indexes(pm, "d");
        assert(naive_query(pm, "d") == pm.query("d"));

        show_indexes(pm, "duck");
        assert(naive_query(pm, "duck") == pm.query("duck"));

        show_indexes(pm, "ka");
        assert(naive_query(pm, "ka") == pm.query("ka"));

        return 0;
    }
}

#endif // LIBFACE_PHRASE_MAP_HPP
