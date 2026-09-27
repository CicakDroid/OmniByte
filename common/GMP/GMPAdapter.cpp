// GMPAdapter — C++ adapter for GNU Multiple Precision Arithmetic Library.

#include "GMPAdapter.h"

#include <android/log.h>

#define LOG_TAG "GMPAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)

namespace omnibyte::common {

GMPAdapter& GMPAdapter::instance() {
    static GMPAdapter inst;
    return inst;
}

mpz_class GMPAdapter::toBigInt(const std::string& s) {
    return mpz_class(s, 10);
}

std::string GMPAdapter::toString(const mpz_class& v, int base) {
    return v.get_str(base);
}

mpz_class GMPAdapter::add(const mpz_class& a, const mpz_class& b) { return a + b; }
mpz_class GMPAdapter::sub(const mpz_class& a, const mpz_class& b) { return a - b; }
mpz_class GMPAdapter::mul(const mpz_class& a, const mpz_class& b) { return a * b; }

mpz_class GMPAdapter::div(const mpz_class& a, const mpz_class& b) {
    if (b == 0) {
        LOGI("div by zero, returning 0");
        return mpz_class(0);
    }
    return a / b;
}

mpz_class GMPAdapter::mod(const mpz_class& a, const mpz_class& b) {
    if (b == 0) {
        LOGI("mod by zero, returning 0");
        return mpz_class(0);
    }
    return a % b;
}

mpz_class GMPAdapter::pow(const mpz_class& base, unsigned long exp) {
    mpz_class r;
    mpz_pow_ui(r.get_mpz_t(), base.get_mpz_t(), exp);
    return r;
}

mpf_class GMPAdapter::toBigFloat(const std::string& s, unsigned precisionBits) {
    mpf_class f(0, precisionBits);
    if (mpf_set_str(f.get_mpf_t(), s.c_str(), 10) != 0) {
        LOGI("invalid float string, returning 0");
        return mpf_class(0, precisionBits);
    }
    return f;
}

} // namespace omnibyte::common
