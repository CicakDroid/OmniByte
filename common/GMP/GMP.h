#pragma once
// GMP -- OmniByte GMP wrapper for reverse engineering operations.
// Source: https://gmplib.org (LGPL v3)
// Version: 6.3.0
//
// Provides arbitrary-precision arithmetic functions useful for:
// - Disassembler: large address calculations, offset analysis
// - Decompiler: expression evaluation, constant folding with big numbers
// - Dumper: checksum verification (SHA256/MD5), memory dump validation
// - Hooking: address computation, trampoline calculation
// - Patching: binary diff, patch generation with large values
// - Runtime: precise timing, performance counters
//
// Citation: GNU MP is used under LGPL v3.
//           See https://gmplib.org/manual/Copying.html
//
// ponytail: wraps mpz_t/mpq_t/mpf_t with RAII and RE-specific helpers.

#include <gmp.h>
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace omnibyte::common {

/// RAII wrapper for mpz_t (arbitrary-precision integer).
class BigInt {
public:
    BigInt();
    explicit BigInt(long val);
    explicit BigInt(const char* str, int base = 10);
    BigInt(const BigInt& other);
    BigInt(BigInt&& other) noexcept;
    ~BigInt();

    BigInt& operator=(const BigInt& other);
    BigInt& operator=(BigInt&& other) noexcept;
    BigInt& operator=(long val);

    /// Convert to string in given base.
    std::string toString(int base = 10) const;

    /// Convert to uint64_t (truncates if larger).
    uint64_t toUInt64() const;

    /// Convert to int64_t (truncates if larger).
    int64_t toInt64() const;

    /// Get raw mpz_t for interop with GMP functions.
    const mpz_t& raw() const { return val_; }

    // Arithmetic operators.
    BigInt operator+(const BigInt& rhs) const;
    BigInt operator-(const BigInt& rhs) const;
    BigInt operator*(const BigInt& rhs) const;
    BigInt operator/(const BigInt& rhs) const;
    BigInt operator%(const BigInt& rhs) const;
    BigInt& operator+=(const BigInt& rhs);
    BigInt& operator-=(const BigInt& rhs);
    BigInt& operator*=(const BigInt& rhs);

    // Comparison.
    bool operator==(const BigInt& rhs) const;
    bool operator!=(const BigInt& rhs) const;
    bool operator<(const BigInt& rhs) const;
    bool operator>(const BigInt& rhs) const;
    bool operator<=(const BigInt& rhs) const;
    bool operator>=(const BigInt& rhs) const;

    /// Bitwise operations.
    BigInt operator&(const BigInt& rhs) const;
    BigInt operator|(const BigInt& rhs) const;
    BigInt operator^(const BigInt& rhs) const;
    BigInt operator~() const;
    BigInt operator<<(unsigned shift) const;
    BigInt operator>>(unsigned shift) const;

    /// Check if value is zero.
    bool isZero() const;

    /// Check if value is negative.
    bool isNegative() const;

    /// Get absolute value.
    BigInt abs() const;

private:
    mpz_t val_;
};

/// RAII wrapper for mpf_t (arbitrary-precision float).
class BigFloat {
public:
    BigFloat();
    explicit BigFloat(double val);
    explicit BigFloat(const char* str, int base = 10);
    BigFloat(const BigFloat& other);
    BigFloat(BigFloat&& other) noexcept;
    ~BigFloat();

    BigFloat& operator=(const BigFloat& other);
    BigFloat& operator=(BigFloat&& other) noexcept;
    BigFloat& operator=(double val);

    /// Convert to string with given precision.
    std::string toString(int precision = 10) const;

    /// Convert to double.
    double toDouble() const;

    /// Get raw mpf_t for interop.
    const mpf_t& raw() const { return val_; }

    BigFloat operator+(const BigFloat& rhs) const;
    BigFloat operator-(const BigFloat& rhs) const;
    BigFloat operator*(const BigFloat& rhs) const;
    BigFloat operator/(const BigFloat& rhs) const;

    bool operator==(const BigFloat& rhs) const;
    bool operator<(const BigFloat& rhs) const;
    bool operator>(const BigFloat& rhs) const;

private:
    mpf_t val_;
};

// =========================================================================
// RE-specific utility functions
// =========================================================================

/// Compute CRC32 of binary data using GMP-assisted arithmetic.
/// Useful for: dump integrity verification, anti-tamper bypass.
uint32_t gmp_crc32(const uint8_t* data, size_t len);

/// Compute large-number modular exponentiation (base^exp mod mod).
/// Useful for: RSA analysis, crypto reverse engineering.
BigInt modPow(const BigInt& base, const BigInt& exp, const BigInt& mod);

/// Compute greatest common divisor.
/// Useful for: offset analysis, finding common factors in addresses.
BigInt gcd(const BigInt& a, const BigInt& b);

/// Compute modular inverse (a^(-1) mod m).
/// Useful for: cryptographic analysis, key derivation.
BigInt modInverse(const BigInt& a, const BigInt& m);

/// Analyze memory address alignment (check power-of-2 alignment).
/// Useful for: hooking, trampoline placement, memory alignment.
bool isAligned(uintptr_t address, size_t alignment);

/// Round up address to next alignment boundary.
/// Useful for: memory allocation, hook placement.
uintptr_t alignUp(uintptr_t address, size_t alignment);

/// Compute offset between two addresses.
/// Useful for: relative address calculation, binary patching.
BigInt addressOffset(uintptr_t from, uintptr_t to);

/// Verify memory region integrity via GMP-assisted checksum.
/// Useful for: dump verification, anti-tamper detection.
bool verifyIntegrity(const uint8_t* data, size_t len, uint32_t expectedCrc);

/// Compute factorial using GMP (for statistical analysis).
BigInt factorial(unsigned n);

/// Compute binomial coefficient C(n,k).
/// Useful for: combinatorial analysis in crypto algorithms.
BigInt binomial(unsigned n, unsigned k);

} // namespace omnibyte::common
