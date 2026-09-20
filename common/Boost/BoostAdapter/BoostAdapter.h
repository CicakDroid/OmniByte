#pragma once
// BoostAdapter -- C++ adapter for Boost libraries.
// Source: https://github.com/boostorg/boost (BSL-1.0)
// Version: 1.92.0
//
// Only includes the subset of Boost we actually use:
//   - boost/container_hash: hash functions for containers
//   - boost/crc: CRC checksums
//   - boost/bimap: bidirectional maps for symbol resolution
//   - boost/dynamic_bitset: dynamic bit arrays
//   - boost/algorithm: string algorithms, searching
//   - boost/format: formatted string output

namespace omnibyte::common {

class BoostAdapter {
public:
    BoostAdapter();
    ~BoostAdapter() = default;

    BoostAdapter(const BoostAdapter&) = delete;
    BoostAdapter& operator=(const BoostAdapter&) = delete;
    BoostAdapter(BoostAdapter&&) = delete;
    BoostAdapter& operator=(BoostAdapter&&) = delete;
};

} // namespace omnibyte::common
