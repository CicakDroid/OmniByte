#pragma once
// Logging/LoggingConfig.h — configuration for log output and rotation.

#include <cstdint>
#include <string>

namespace omnibyte::dumper::config {

struct LoggingConfig {
    // Minimum log level to emit: "trace", "debug", "info", "warn", "error", "fatal".
    std::string logLevel = "info";

    // Maximum number of rotated log files to keep before oldest is deleted.
    uint32_t rotationMaxFiles = 5;

    // Whether the user has opted in to anonymous crash reporting.
    bool crashReportOptIn = true;

    static LoggingConfig defaults() { return {}; }
};

} // namespace omnibyte::dumper::config
