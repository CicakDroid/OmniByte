#pragma once
// Runtime/RuntimeLoader.h — load RuntimeConfig from JSON.

#include "RuntimeConfig.h"
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

inline RuntimeConfig loadRuntimeConfig(const std::string& path) {
    auto j = omnibyte::common::loadJsonFile(path);
    if (!j) return RuntimeConfig::defaults();
    return RuntimeConfig::fromJson(*j);
}

} // namespace omnibyte::dumper::config
