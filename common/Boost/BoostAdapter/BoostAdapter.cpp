// BoostAdapter -- C++ adapter for Boost libraries.

#include "BoostAdapter.h"

#include <android/log.h>

#define LOG_TAG "BoostAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)

namespace omnibyte::common {

BoostAdapter::BoostAdapter() {
    LOGI("BoostAdapter initialized");
}

} // namespace omnibyte::common
