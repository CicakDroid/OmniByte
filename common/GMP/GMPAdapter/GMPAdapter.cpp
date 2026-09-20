// GMPAdapter — C++ adapter for GNU Multiple Precision Arithmetic Library.

#include "GMPAdapter.h"

#include <android/log.h>

#define LOG_TAG "GMPAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)

namespace omnibyte::common {

GMPAdapter::GMPAdapter() {
    LOGI("GMPAdapter initialized, GMP version: %s", gmp_version);
}

} // namespace omnibyte::common
