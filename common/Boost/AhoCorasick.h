#pragma once
// AhoCorasick — vendored header-only extension (not upstream Boost).
// Source: https://github.com/cjgdev/aho_corasick (MIT)
// Fetched via CMake FetchContent; do NOT vendor under boost-src (FetchContent wipe).
//
// Thin alias only — business logic lives in BoostAdapter::ahoCorasickSearch.

#include <aho_corasick/aho_corasick.hpp>

#include <string>
#include <utility>
#include <vector>

namespace omnibyte::boost_ext {

// aho_corasick::trie — insert patterns, parse_text in one O(n+m) pass.
using AhoTrie = aho_corasick::trie;

// One match: [start, end] inclusive range in the haystack + which pattern hit.
struct AhoMatch {
    size_t start = 0;
    size_t end = 0;
    std::string keyword;
    unsigned patternIndex = 0;
};

// Convenience: build trie from patterns and scan text in a single pass.
inline std::vector<AhoMatch> ahoSearch(
    const std::vector<std::string>& patterns,
    const std::string& text
) {
    AhoTrie trie;
    for (const auto& p : patterns) {
        trie.insert(p);
    }
    std::vector<AhoMatch> out;
    for (const auto& e : trie.parse_text(text)) {
        out.push_back(AhoMatch{
            e.get_start(), e.get_end(), e.get_keyword(), e.get_index()
        });
    }
    return out;
}

} // namespace omnibyte::boost_ext
