#pragma once
// Network/NetworkLoader.h — load NetworkConfig from JSON.

#include "NetworkConfig.h"
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

inline NetworkConfig loadNetworkConfig(const std::string& path) {
    auto j = omnibyte::common::loadJsonFile(path);
    if (!j) return NetworkConfig::defaults();

    NetworkConfig cfg;
    cfg.downloadTimeoutMs = omnibyte::common::getOr<uint32_t>(*j, "downloadTimeoutMs", cfg.downloadTimeoutMs);
    cfg.retryCount        = omnibyte::common::getOr<uint32_t>(*j, "retryCount", cfg.retryCount);
    return cfg;
}

} // namespace omnibyte::dumper::config
