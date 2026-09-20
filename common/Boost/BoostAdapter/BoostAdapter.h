#pragma once
// BoostAdapter -- C++ adapter for Boost libraries.
// Source: https://github.com/boostorg/boost (BSL-1.0)
// Version: 1.83.0
//
// Provides version checking and header inclusion for Boost.
// Only includes the subset of Boost we actually use:
//   - boost/container_hash: hash functions for containers
//   - boost/crc: CRC checksums
//   - boost/bimap: bidirectional maps for symbol resolution
//   - boost/dynamic_bitset: dynamic bit arrays
//   - boost/algorithm: string algorithms, searching
//   - boost/format: formatted string output
//
// ponytail: header-only; no adapter implementation needed.

#include <boost/version.hpp>
#include <string>

namespace omnibyte::common {

/// Boost library adapter with version check.
class BoostAdapter {
public:
    BoostAdapter();
    ~BoostAdapter() = default;

    // Non-copyable, non-movable.
    BoostAdapter(const BoostAdapter&) = delete;
    BoostAdapter& operator=(const BoostAdapter&) = delete;
    BoostAdapter(BoostAdapter&&) = delete;
    BoostAdapter& operator=(BoostAdapter&&) = delete;

    /// Check if Boost is available.
    bool isAvailable() const;

    /// Get Boost version string (e.g., "1.83.0").
    static const char* getVersion();

    /// Get Boost version as major.minor.patch integers.
    static void getVersion(int& major, int& minor, int& patch);

    /// Check if Boost version meets minimum requirement.
    static bool checkVersion(int minMajor, int minMinor = 0, int minPatch = 0);

private:
    bool available_ = false;
};

} // namespace omnibyte::common
