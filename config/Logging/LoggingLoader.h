#pragma once
// Logging/LoggingLoader.h — load LoggingConfig from JSON.

#include "LoggingConfig.h"
#include "../Common/ConfigLoader.h"

namespace omnibyte::dumper::config {

inline LoggingConfig loadLoggingConfig(const std::string& path) {
    auto j = loadJsonFile(path);
    if (!j) return LoggingConfig::defaults();

    LoggingConfig cfg;
    cfg.logLevel          = getOr<std::string>(*j, "logLevel", cfg.logLevel);
    cfg.rotationMaxFiles  = getOr<uint32_t>(*j, "rotationMaxFiles", cfg.rotationMaxFiles);
    cfg.crashReportOptIn  = getOr<bool>(*j, "crashReportOptIn", cfg.crashReportOptIn);
    return cfg;
}

} // namespace omnibyte::dumper::config
