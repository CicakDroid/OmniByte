// config/ConfigManager.cpp — central config loader implementation.

#include "ConfigManager.h"
#include <common/Serialization/JsonLoader.h>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace omnibyte::dumper::config {

namespace {
    // Log a warning when a config field falls back to default.
    void logFallback(const std::string& configName, const std::string& detail) {
        std::cerr << "[ConfigManager] WARNING: " << configName
                  << " fallback to default — " << detail << "\n";
    }

    // Write nlohmann::json to a file, pretty-printed.
    bool writeJsonFile(const std::string& path, const nlohmann::json& j) {
        std::ofstream file(path);
        if (!file.is_open()) return false;
        file << j.dump(4) << "\n";
        return file.good();
    }
} // anonymous namespace

ConfigManager& ConfigManager::instance() {
    static ConfigManager s_instance;
    return s_instance;
}

std::string ConfigManager::resolvePath(const std::string& configDir,
                                       const std::string& fileName) const {
    // If configDir is provided, use it directly.
    // Otherwise fall back to Android internal filesDir/config/ convention.
    std::string dir = configDir.empty() ? configDir_ : configDir;
    if (dir.empty()) {
        // Last resort: current working directory
        dir = ".";
    }
    // Ensure trailing separator
    if (!dir.empty() && dir.back() != '/') {
        dir += '/';
    }
    return dir + fileName;
}

template <typename T>
void ConfigManager::loadOne(const std::string& configDir,
                            const std::string& fileName,
                            const std::string& configName,
                            T& out) {
    std::string path = resolvePath(configDir, fileName);
    auto j = omnibyte::common::loadJsonFile(path);

    if (!j) {
        // File missing or unparseable — keep defaults, log once.
        out = T::defaults();
        logFallback(configName, "file missing or corrupt: " + path);
        return;
    }

    try {
        out = T::fromJson(*j);
    } catch (const std::exception& e) {
        // fromJson itself failed — keep defaults for this config.
        out = T::defaults();
        logFallback(configName, std::string("fromJson exception: ") + e.what());
    }
}

template <typename T>
bool ConfigManager::saveOne(const std::string& configDir,
                            const std::string& fileName,
                            const T& cfg) {
    std::string path = resolvePath(configDir, fileName);
    try {
        nlohmann::json j = cfg.toJson();
        return writeJsonFile(path, j);
    } catch (const std::exception& e) {
        std::cerr << "[ConfigManager] ERROR: save " << fileName
                  << " failed: " << e.what() << "\n";
        return false;
    }
}

void ConfigManager::load(const std::string& configDir) {
    configDir_ = configDir;

    // Load each config independently — one corrupt file never blocks another.
    loadOne(configDir, "file_limits.json",      "FileLimitsConfig",      fileLimits_);
    loadOne(configDir, "engine_detection.json",  "EngineDetectionConfig", engineDetection_);
    loadOne(configDir, "runtime.json",           "RuntimeConfig",         runtime_);
    loadOne(configDir, "network.json",           "NetworkConfig",         network_);
    loadOne(configDir, "storage.json",           "StorageConfig",         storage_);
    loadOne(configDir, "ui.json",                "UIConfig",              ui_);
    loadOne(configDir, "logging.json",           "LoggingConfig",         logging_);
}

bool ConfigManager::save(const std::string& configName) {
    if (configName == "file_limits")      return saveOne(configDir_, "file_limits.json",      fileLimits_);
    if (configName == "engine_detection")  return saveOne(configDir_, "engine_detection.json",  engineDetection_);
    if (configName == "runtime")           return saveOne(configDir_, "runtime.json",           runtime_);
    if (configName == "network")           return saveOne(configDir_, "network.json",           network_);
    if (configName == "storage")           return saveOne(configDir_, "storage.json",           storage_);
    if (configName == "ui")                return saveOne(configDir_, "ui.json",                ui_);
    if (configName == "logging")           return saveOne(configDir_, "logging.json",           logging_);
    return false;
}

void ConfigManager::saveAll() {
    saveOne(configDir_, "file_limits.json",      fileLimits_);
    saveOne(configDir_, "engine_detection.json",  engineDetection_);
    saveOne(configDir_, "runtime.json",           runtime_);
    saveOne(configDir_, "network.json",           network_);
    saveOne(configDir_, "storage.json",           storage_);
    saveOne(configDir_, "ui.json",                ui_);
    saveOne(configDir_, "logging.json",           logging_);
}

} // namespace omnibyte::dumper::config
