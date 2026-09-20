// GMP.cpp -- OmniByte GMP wrapper for reverse engineering operations.

#include "GMP.h"

#include <android/log.h>
#include <cstring>
#include <stdexcept>

#define LOG_TAG "GMP"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace omnibyte::common {

// =========================================================================
// BigInt implementation
// =========================================================================

BigInt::BigInt() {
    mpz_init(val_);
}

BigInt::BigInt(long val) {
    mpz_init_set_si(val_, val);
}

BigInt::BigInt(const char* str, int base) {
    if (mpz_init_set_str(val_, str, base) != 0) {
        mpz_set_ui(val_, 0);
        LOGE("BigInt: failed to parse string (base=%d)", base);
    }
}

BigInt::BigInt(const BigInt& other) {
    mpz_init_set(val_, other.val_);
}

BigInt::BigInt(BigInt&& other) noexcept {
    mpz_init(val_);
    mpz_swap(val_, other.val_);
    mpz_clear(other.val_);
    mpz_init(other.val_);
}

BigInt::~BigInt() {
    mpz_clear(val_);
}

BigInt& BigInt::operator=(const BigInt& other) {
    if (this != &other) {
        mpz_set(val_, other.val_);
    }
    return *this;
}

BigInt& BigInt::operator=(BigInt&& other) noexcept {
    if (this != &other) {
        mpz_swap(val_, other.val_);
    }
    return *this;
}

BigInt& BigInt::operator=(long val) {
    mpz_set_si(val_, val);
    return *this;
}

std::string BigInt::toString(int base) const {
    char* str = mpz_get_str(nullptr, base, val_);
    std::string result(str);
    void (*freefunc)(void*, size_t);
    mp_get_memory_functions(nullptr, nullptr, &freefunc);
    freefunc(str, strlen(str) + 1);
    return result;
}

uint64_t BigInt::toUInt64() const {
    return mpz_get_ui(val_);
}

int64_t BigInt::toInt64() const {
    return mpz_get_si(val_);
}

BigInt BigInt::operator+(const BigInt& rhs) const {
    BigInt result;
    mpz_add(result.val_, val_, rhs.val_);
    return result;
}

BigInt BigInt::operator-(const BigInt& rhs) const {
    BigInt result;
    mpz_sub(result.val_, val_, rhs.val_);
    return result;
}

BigInt BigInt::operator*(const BigInt& rhs) const {
    BigInt result;
    mpz_mul(result.val_, val_, rhs.val_);
    return result;
}

BigInt BigInt::operator/(const BigInt& rhs) const {
    BigInt result;
    mpz_tdiv_q(result.val_, val_, rhs.val_);
    return result;
}

BigInt BigInt::operator%(const BigInt& rhs) const {
    BigInt result;
    mpz_tdiv_r(result.val_, val_, rhs.val_);
    return result;
}

BigInt& BigInt::operator+=(const BigInt& rhs) {
    mpz_add(val_, val_, rhs.val_);
    return *this;
}

BigInt& BigInt::operator-=(const BigInt& rhs) {
    mpz_sub(val_, val_, rhs.val_);
    return *this;
}

BigInt& BigInt::operator*=(const BigInt& rhs) {
    mpz_mul(val_, val_, rhs.val_);
    return *this;
}

bool BigInt::operator==(const BigInt& rhs) const {
    return mpz_cmp(val_, rhs.val_) == 0;
}

bool BigInt::operator!=(const BigInt& rhs) const {
    return mpz_cmp(val_, rhs.val_) != 0;
}

bool BigInt::operator<(const BigInt& rhs) const {
    return mpz_cmp(val_, rhs.val_) < 0;
}

bool BigInt::operator>(const BigInt& rhs) const {
    return mpz_cmp(val_, rhs.val_) > 0;
}

bool BigInt::operator<=(const BigInt& rhs) const {
    return mpz_cmp(val_, rhs.val_) <= 0;
}

bool BigInt::operator>=(const BigInt& rhs) const {
    return mpz_cmp(val_, rhs.val_) >= 0;
}

BigInt BigInt::operator&(const BigInt& rhs) const {
    BigInt result;
    mpz_and(result.val_, val_, rhs.val_);
    return result;
}

BigInt BigInt::operator|(const BigInt& rhs) const {
    BigInt result;
    mpz_ior(result.val_, val_, rhs.val_);
    return result;
}

BigInt BigInt::operator^(const BigInt& rhs) const {
    BigInt result;
    mpz_xor(result.val_, val_, rhs.val_);
    return result;
}

BigInt BigInt::operator~() const {
    BigInt result;
    mpz_com(result.val_, val_);
    return result;
}

BigInt BigInt::operator<<(unsigned shift) const {
    BigInt result;
    mpz_mul_2exp(result.val_, val_, shift);
    return result;
}

BigInt BigInt::operator>>(unsigned shift) const {
    BigInt result;
    mpz_tdiv_q_2exp(result.val_, val_, shift);
    return result;
}

bool BigInt::isZero() const {
    return mpz_sgn(val_) == 0;
}

bool BigInt::isNegative() const {
    return mpz_sgn(val_) < 0;
}

BigInt BigInt::abs() const {
    BigInt result;
    mpz_abs(result.val_, val_);
    return result;
}

// =========================================================================
// BigFloat implementation
// =========================================================================

BigFloat::BigFloat() {
    mpf_init(val_);
}

BigFloat::BigFloat(double val) {
    mpf_init_set_d(val_, val);
}

BigFloat::BigFloat(const char* str, int base) {
    if (mpf_init_set_str(val_, str, base) != 0) {
        mpf_set_ui(val_, 0);
        LOGE("BigFloat: failed to parse string (base=%d)", base);
    }
}

BigFloat::BigFloat(const BigFloat& other) {
    mpf_init_set(val_, other.val_);
}

BigFloat::BigFloat(BigFloat&& other) noexcept {
    mpf_init(val_);
    mpf_swap(val_, other.val_);
    mpf_clear(other.val_);
    mpf_init(other.val_);
}

BigFloat::~BigFloat() {
    mpf_clear(val_);
}

BigFloat& BigFloat::operator=(const BigFloat& other) {
    if (this != &other) {
        mpf_set(val_, other.val_);
    }
    return *this;
}

BigFloat& BigFloat::operator=(BigFloat&& other) noexcept {
    if (this != &other) {
        mpf_swap(val_, other.val_);
    }
    return *this;
}

BigFloat& BigFloat::operator=(double val) {
    mpf_set_d(val_, val);
    return *this;
}

std::string BigFloat::toString(int precision) const {
    mp_exp_t exp;
    char* str = mpf_get_str(nullptr, &exp, 10, precision, val_);
    std::string result(str);
    void (*freefunc)(void*, size_t);
    mp_get_memory_functions(nullptr, nullptr, &freefunc);
    freefunc(str, strlen(str) + 1);
    return result;
}

double BigFloat::toDouble() const {
    return mpf_get_d(val_);
}

BigFloat BigFloat::operator+(const BigFloat& rhs) const {
    BigFloat result;
    mpf_add(result.val_, val_, rhs.val_);
    return result;
}

BigFloat BigFloat::operator-(const BigFloat& rhs) const {
    BigFloat result;
    mpf_sub(result.val_, val_, rhs.val_);
    return result;
}

BigFloat BigFloat::operator*(const BigFloat& rhs) const {
    BigFloat result;
    mpf_mul(result.val_, val_, rhs.val_);
    return result;
}

BigFloat BigFloat::operator/(const BigFloat& rhs) const {
    BigFloat result;
    mpf_div(result.val_, val_, rhs.val_);
    return result;
}

bool BigFloat::operator==(const BigFloat& rhs) const {
    return mpf_cmp(val_, rhs.val_) == 0;
}

bool BigFloat::operator<(const BigFloat& rhs) const {
    return mpf_cmp(val_, rhs.val_) < 0;
}

bool BigFloat::operator>(const BigFloat& rhs) const {
    return mpf_cmp(val_, rhs.val_) > 0;
}

// =========================================================================
// RE-specific utility functions
// =========================================================================

uint32_t gmp_crc32(const uint8_t* data, size_t len) {
    // Standard CRC32 (used in dump integrity checks and anti-tamper).
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

BigInt modPow(const BigInt& base, const BigInt& exp, const BigInt& mod) {
    BigInt result;
    mpz_powm(result.raw(), base.raw(), exp.raw(), mod.raw());
    return result;
}

BigInt gcd(const BigInt& a, const BigInt& b) {
    BigInt result;
    mpz_gcd(result.raw(), a.raw(), b.raw());
    return result;
}

BigInt modInverse(const BigInt& a, const BigInt& m) {
    BigInt result;
    if (mpz_invert(result.raw(), a.raw(), m.raw()) == 0) {
        LOGE("modInverse: no inverse exists");
    }
    return result;
}

bool isAligned(uintptr_t address, size_t alignment) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) return false;
    return (address & (alignment - 1)) == 0;
}

uintptr_t alignUp(uintptr_t address, size_t alignment) {
    if (alignment == 0) return address;
    return (address + alignment - 1) & ~(alignment - 1);
}

BigInt addressOffset(uintptr_t from, uintptr_t to) {
    if (to >= from) {
        return BigInt(static_cast<long>(to - from));
    }
    BigInt result(static_cast<long>(from - to));
    return -result;
}

bool verifyIntegrity(const uint8_t* data, size_t len, uint32_t expectedCrc) {
    return gmp_crc32(data, len) == expectedCrc;
}

BigInt factorial(unsigned n) {
    BigInt result;
    mpz_fac_ui(result.raw(), n);
    return result;
}

BigInt binomial(unsigned n, unsigned k) {
    BigInt result;
    mpz_bin_uiui(result.raw(), n, k);
    return result;
}

} // namespace omnibyte::common
