#pragma once
// FileLimits/FileLimitsLoader.h — load FileLimitsConfig from JSON.

#include "FileLimitsConfig.h"
#include <common/Deserialization-Serialization/Json/Json.h>

namespace omnibyte::dumper::config {

inline FileLimitsConfig loadFileLimitsConfig(const std::string& path) {
    auto j = omnibyte::common::loadJsonFile(path);
    if (!j) return FileLimitsConfig::defaults();
    return FileLimitsConfig::fromJson(*j);
}

} // namespace omnibyte::dumper::config
