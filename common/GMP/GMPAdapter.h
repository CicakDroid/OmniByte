#pragma once
// GMPAdapter — C++ adapter for GNU Multiple Precision Arithmetic Library.
// Source: https://gmplib.org (LGPL v3)
// Version: 6.3.0
//
// Purpose: 64-bit offsets, checksums, crypto constants in binary analysis.
// NOT new cryptography — no key generation, no signature schemes.
//
// Singleton; thread-safe for independent values (mpz_class is not
// shared across threads without external locking).

#include <gmpxx.h>
#include <string>

namespace omnibyte::common {

class GMPAdapter {
public:
    static GMPAdapter& instance();

    mpz_class toBigInt(const std::string& s);
    std::string toString(const mpz_class& v, int base = 10);

    mpz_class add(const mpz_class& a, const mpz_class& b);
    mpz_class sub(const mpz_class& a, const mpz_class& b);
    mpz_class mul(const mpz_class& a, const mpz_class& b);
    mpz_class div(const mpz_class& a, const mpz_class& b);
    mpz_class mod(const mpz_class& a, const mpz_class& b);
    mpz_class pow(const mpz_class& base, unsigned long exp);

    mpf_class toBigFloat(const std::string& s, unsigned precisionBits = 64);

    GMPAdapter(const GMPAdapter&) = delete;
    GMPAdapter& operator=(const GMPAdapter&) = delete;

private:
    GMPAdapter() = default;
};

} // namespace omnibyte::common
