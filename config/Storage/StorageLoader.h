#pragma once
// Storage/StorageLoader.h — load StorageConfig from JSON.

#include "StorageConfig.h"
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

inline StorageConfig loadStorageConfig(const std::string& path) {
    auto j = omnibyte::common::loadJsonFile(path);
    if (!j) return StorageConfig::defaults();
    return StorageConfig::fromJson(*j);
}

} // namespace omnibyte::dumper::config
