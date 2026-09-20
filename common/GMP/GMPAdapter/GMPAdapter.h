#pragma once
// GMPAdapter — C++ adapter for GNU Multiple Precision Arithmetic Library.
// Source: https://gmplib.org (LGPL v3)
// Version: 6.3.0
//
// Provides version checking and basic initialization for GMP.
// Citation: GNU MP is used under LGPL v3. See https://gmplib.org/manual/Copying.html
//
// ponytail: thin adapter; GMP does its own initialization.

#include <gmp.h>
#include <string>

namespace omnibyte::common {

/// GMP library adapter with version check and initialization.
class GMPAdapter {
public:
    GMPAdapter();
    ~GMPAdapter() = default;

    // Non-copyable, non-movable (GMP state is global).
    GMPAdapter(const GMPAdapter&) = delete;
    GMPAdapter& operator=(const GMPAdapter&) = delete;
    GMPAdapter(GMPAdapter&&) = delete;
    GMPAdapter& operator=(GMPAdapter&&) = delete;

    /// Check if GMP library is available and initialized.
    bool isAvailable() const;

    /// Get GMP version string (e.g., "6.3.0").
    static const char* getVersion();

    /// Get GMP version as major.minor.patch integers.
    static void getVersion(int& major, int& minor, int& patch);

    /// Check if GMP version meets minimum requirement.
    static bool checkVersion(int minMajor, int minMinor = 0, int minPatch = 0);

private:
    bool available_ = false;
};

} // namespace omnibyte::common
