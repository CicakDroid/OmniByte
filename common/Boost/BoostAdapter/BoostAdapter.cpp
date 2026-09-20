// BoostAdapter -- C++ adapter for Boost libraries.

#include "BoostAdapter.h"

#include <android/log.h>

#define LOG_TAG "BoostAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace omnibyte::common {

BoostAdapter::BoostAdapter() {
    LOGI("BoostAdapter initialized, Boost version: %s", BOOST_LIB_VERSION);
    available_ = true;
}

bool BoostAdapter::isAvailable() const {
    return available_;
}

const char* BoostAdapter::getVersion() {
    return BOOST_LIB_VERSION;  // defined by <boost/version.hpp>
}

void BoostAdapter::getVersion(int& major, int& minor, int& patch) {
    major = BOOST_VERSION / 100000;
    minor = BOOST_VERSION / 100 % 1000;
    patch = BOOST_VERSION % 100;
}

bool BoostAdapter::checkVersion(int minMajor, int minMinor, int minPatch) {
    int major, minor, patch;
    getVersion(major, minor, patch);
    if (major != minMajor) return major > minMajor;
    if (minor != minMinor) return minor > minMinor;
    return patch >= minPatch;
}

} // namespace omnibyte::common
