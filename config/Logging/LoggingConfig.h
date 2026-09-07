#pragma once
// Logging/LoggingConfig.h — configuration for log output and rotation.

#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

struct LoggingConfig {
    std::string logLevel = "info";
    uint32_t rotationMaxFiles = 5;
    bool crashReportOptIn = false;

    static LoggingConfig defaults() { return {}; }

    static LoggingConfig fromJson(const nlohmann::json& j) {
        LoggingConfig cfg;
        cfg.logLevel         = omnibyte::common::getOr<std::string>(j, "logLevel", cfg.logLevel);
        cfg.rotationMaxFiles = omnibyte::common::getOr<uint32_t>(j, "rotationMaxFiles", cfg.rotationMaxFiles);
        cfg.crashReportOptIn = omnibyte::common::getOr<bool>(j, "crashReportOptIn", cfg.crashReportOptIn);
        return cfg;
    }

    nlohmann::json toJson() const {
        return {
            {"logLevel",         logLevel},
            {"rotationMaxFiles", rotationMaxFiles},
            {"crashReportOptIn", crashReportOptIn}
        };
    }
};

} // namespace omnibyte::dumper::config
