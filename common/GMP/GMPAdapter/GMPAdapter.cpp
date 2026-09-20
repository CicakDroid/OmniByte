// GMPAdapter — C++ adapter for GNU Multiple Precision Arithmetic Library.

#include "GMPAdapter.h"

#include <android/log.h>

#define LOG_TAG "GMPAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace omnibyte::common {

GMPAdapter::GMPAdapter() {
    // GMP uses malloc-based memory allocation by default.
    // No explicit initialization required for basic usage.
    LOGI("GMPAdapter initialized, GMP version: %s", gmp_version);
    available_ = true;
}

bool GMPAdapter::isAvailable() const {
    return available_;
}

const char* GMPAdapter::getVersion() {
    return gmp_version;  // defined by <gmp.h>
}

void GMPAdapter::getVersion(int& major, int& minor, int& patch) {
    major = __GNU_MP_VERSION;
    minor = __GNU_MP_VERSION_MINOR;
    patch = __GNU_MP_VERSION_PATCHLEVEL;
}

bool GMPAdapter::checkVersion(int minMajor, int minMinor, int minPatch) {
    int major, minor, patch;
    getVersion(major, minor, patch);
    if (major != minMajor) return major > minMajor;
    if (minor != minMinor) return minor > minMinor;
    return patch >= minPatch;
}

} // namespace omnibyte::common
