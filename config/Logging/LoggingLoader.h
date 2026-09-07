#pragma once
// Logging/LoggingLoader.h — load LoggingConfig from JSON.

#include "LoggingConfig.h"
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

inline LoggingConfig loadLoggingConfig(const std::string& path) {
    auto j = omnibyte::common::loadJsonFile(path);
    if (!j) return LoggingConfig::defaults();
    return LoggingConfig::fromJson(*j);
}

} // namespace omnibyte::dumper::config
