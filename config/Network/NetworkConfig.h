#pragma once
// Network/NetworkConfig.h — configuration for network operations (downloads, retries).

#include <cstdint>

namespace omnibyte::dumper::config {

struct NetworkConfig {
    // Timeout in milliseconds for a single network download.
    uint32_t downloadTimeoutMs = 30000;  // 30 seconds

    // Number of retry attempts on network failure before giving up.
    uint32_t retryCount = 3;

    static NetworkConfig defaults() { return {}; }
};

} // namespace omnibyte::dumper::config
