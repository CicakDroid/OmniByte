#pragma once
// Network/NetworkConfig.h — configuration for network operations (downloads, retries).

#include <cstdint>
#include <nlohmann/json.hpp>
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

struct NetworkConfig {
    uint32_t downloadTimeoutMs = 15000;  // 15 seconds
    uint32_t retryCount = 3;

    static NetworkConfig defaults() { return {}; }

    static NetworkConfig fromJson(const nlohmann::json& j) {
        NetworkConfig cfg;
        cfg.downloadTimeoutMs = omnibyte::common::getOr<uint32_t>(j, "downloadTimeoutMs", cfg.downloadTimeoutMs);
        cfg.retryCount        = omnibyte::common::getOr<uint32_t>(j, "retryCount", cfg.retryCount);
        return cfg;
    }

    nlohmann::json toJson() const {
        return {
            {"downloadTimeoutMs", downloadTimeoutMs},
            {"retryCount",        retryCount}
        };
    }
};

} // namespace omnibyte::dumper::config
