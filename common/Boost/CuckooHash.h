#pragma once
// CuckooHash — vendored header-only extension (not upstream Boost).
// Source: https://github.com/efficient/libcuckoo (Apache-2.0)
// Fetched via CMake FetchContent; do NOT vendor under boost-src (FetchContent wipe).
//
// Thin alias only — business logic lives in BoostAdapter::makeCuckooMap.

#include <libcuckoo/cuckoohash_map.hh>

#include <cstddef>
#include <utility>

namespace omnibyte::boost_ext {

// Concurrent cuckoo hash table (multi-reader / multi-writer safe).
// Use for symbol tables, disassembly caches, and other hot lookups.
template <typename K, typename V>
using CuckooMap = libcuckoo::cuckoohash_map<K, V>;

// Empty map with default bucket count (libcuckoo DEFAULT_SIZE).
template <typename K, typename V>
inline CuckooMap<K, V> makeCuckooMap() {
    return CuckooMap<K, V>{};
}

} // namespace omnibyte::boost_ext
