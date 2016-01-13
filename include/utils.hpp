#if !defined LIBFACE_UTILS_HPP
#define LIBFACE_UTILS_HPP

#ifdef USE_USTL
#include <ustl.h>
namespace nw=ustl;
#else
#include <iostream>
#include <vector>
#include <utility>
#include <ostream>
namespace nw=std;
#endif // USE_USTL
#include <assert.h>
#include <stdio.h>

#include <include/types.hpp>

#if defined DEBUG
#define DCERR(X) nw::cerr<<X;
#define DPRINTF(ARGS...) fprintf(stderr, ARGS);
#else
#define DCERR(X)
#define DPRINTF(ARGS...)
#endif

#define assert_lt(X,Y) if (!((X)<(Y))) { fprintf(stderr, "%d < %d FAILED\n", int(X), int(Y)); assert((X)<(Y)); }
#define assert_gt(X,Y) if (!((X)>(Y))) { fprintf(stderr, "%d > %d FAILED\n", (X), (Y)); assert((X)>(Y)); }
#define assert_le(X,Y) if (!((X)<=(Y))) { fprintf(stderr, "%d <= %d FAILED\n", (X), (Y)); assert((X)<=(Y)); }
#define assert_eq(X,Y) if (!((X)==(Y))) { fprintf(stderr, "%d == %d FAILED\n", int(X), int(Y)); assert((X)==(Y)); }
#define assert_ne(X,Y) if (!((X)!=(Y))) { fprintf(stderr, "%d != %d FAILED\n", (X), (Y)); assert((X)!=(Y)); }

inline uint_t log2(uint_t n) {
    uint_t lg2 = 0;
    while (n > 1) {
        n /= 2;
        ++lg2;
    }
    return lg2;
}

const uint_t minus_one = (uint_t)0 - 1;

template <typename T>
#ifdef USE_USTL
nw::ofstream&
operator<<(nw::ofstream& out, nw::vector<T> const& vec) {
#else
nw::ostream&
operator<<(nw::ostream& out, nw::vector<T> const& vec) {
#endif // USE_USTL
    for (size_t i = 0; i < vec.size(); ++i) {
        out<<vec[i]<<nw::endl;
    }
    return out;
}

template <typename T, typename U>
#ifdef USE_USTL
nw::ofstream&
operator<<(nw::ofstream& out, nw::pair<T, U> const& p) {
#else
nw::ostream&
operator<<(nw::ostream& out, nw::pair<T, U> const& p) {
#endif // USE_USTL
    out<<"("<<p.first<<", "<<p.second<<")";
    return out;
}

#ifdef USE_USTL
inline nw::ofstream&
operator<<(nw::ofstream& out, phrase_t const& p) {
#else
inline nw::ostream&
operator<<(nw::ostream& out, phrase_t const& p) {
#endif // USE_USTL
    out<<"("<<p.phrase<<", "<<p.weight<<")";
    return out;
}


#endif // LIBFACE_UTILS_HPP
