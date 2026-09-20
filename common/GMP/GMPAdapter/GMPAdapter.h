#pragma once
// GMPAdapter — C++ adapter for GNU Multiple Precision Arithmetic Library.
// Source: https://gmplib.org (LGPL v3)
// Version: 6.3.0

#include <gmp.h>

namespace omnibyte::common {

class GMPAdapter {
public:
    GMPAdapter();
    ~GMPAdapter() = default;

    GMPAdapter(const GMPAdapter&) = delete;
    GMPAdapter& operator=(const GMPAdapter&) = delete;
    GMPAdapter(GMPAdapter&&) = delete;
    GMPAdapter& operator=(GMPAdapter&&) = delete;
};

} // namespace omnibyte::common
