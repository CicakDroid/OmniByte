#pragma once
// Logging/LoggingLoader.h — load LoggingConfig from JSON.

#include "LoggingConfig.h"
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

inline LoggingConfig loadLoggingConfig(const std::string& path) {
    auto j = omnibyte::common::loadJsonFile(path);
    if (!j) return LoggingConfig::defaults();

    LoggingConfig cfg;
    cfg.logLevel          = omnibyte::common::getOr<std::string>(*j, "logLevel", cfg.logLevel);
    cfg.rotationMaxFiles  = omnibyte::common::getOr<uint32_t>(*j, "rotationMaxFiles", cfg.rotationMaxFiles);
    cfg.crashReportOptIn  = omnibyte::common::getOr<bool>(*j, "crashReportOptIn", cfg.crashReportOptIn);
    return cfg;
}

} // namespace omnibyte::dumper::config
