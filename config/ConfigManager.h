#pragma once
// config/ConfigManager.h — central config loader and accessor.
//
// Singleton that loads all 7 configs at startup. Modules call:
//   ConfigManager::instance().get<RuntimeConfig>()
// to read config values. UI Settings call save()/saveAll() after edits.

#include "FileLimits/FileLimitsConfig.h"
#include "EngineDetection/EngineDetectionConfig.h"
#include "Runtime/RuntimeConfig.h"
#include "Network/NetworkConfig.h"
#include "Storage/StorageConfig.h"
#include "UI/UIConfig.h"
#include "Logging/LoggingConfig.h"
#include <string>
#include <functional>

namespace omnibyte::dumper::config {

class ConfigManager {
public:
    // Singleton access.
    static ConfigManager& instance();

    // Load all configs from their JSON files.
    // Uses Android internal filesDir/config/ as base path.
    // Falls back to defaults for any file that is missing or corrupt.
    // Logs warnings per-file to LoggingConfig when fallback happens.
    void load(const std::string& configDir = "");

    // Save a single config by name to its JSON file.
    // Returns false if configName is unknown.
    bool save(const std::string& configName);

    // Save all 7 configs to their JSON files.
    void saveAll();

    // Template accessor — call from any module:
    //   auto& cfg = ConfigManager::instance().get<RuntimeConfig>();
    template <typename T>
    const T& get() const;

    // Mutable accessor for UI settings that modify config in-place.
    template <typename T>
    T& getMutable();

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    std::string resolvePath(const std::string& configDir,
                            const std::string& fileName) const;

    template <typename T>
    void loadOne(const std::string& configDir,
                 const std::string& fileName,
                 const std::string& configName,
                 T& out);

    template <typename T>
    bool saveOne(const std::string& configDir,
                 const std::string& fileName,
                 const T& cfg);

    FileLimitsConfig      fileLimits_;
    EngineDetectionConfig engineDetection_;
    RuntimeConfig         runtime_;
    NetworkConfig         network_;
    StorageConfig         storage_;
    UIConfig              ui_;
    LoggingConfig         logging_;

    std::string configDir_;  // resolved once during load()
};

// ── Template specializations for get<T>() ──

template <> inline const FileLimitsConfig&      ConfigManager::get<FileLimitsConfig>()      const { return fileLimits_; }
template <> inline const EngineDetectionConfig&  ConfigManager::get<EngineDetectionConfig>()  const { return engineDetection_; }
template <> inline const RuntimeConfig&          ConfigManager::get<RuntimeConfig>()          const { return runtime_; }
template <> inline const NetworkConfig&          ConfigManager::get<NetworkConfig>()          const { return network_; }
template <> inline const StorageConfig&          ConfigManager::get<StorageConfig>()          const { return storage_; }
template <> inline const UIConfig&               ConfigManager::get<UIConfig>()               const { return ui_; }
template <> inline const LoggingConfig&          ConfigManager::get<LoggingConfig>()          const { return logging_; }

template <> inline FileLimitsConfig&      ConfigManager::getMutable<FileLimitsConfig>()      { return fileLimits_; }
template <> inline EngineDetectionConfig&  ConfigManager::getMutable<EngineDetectionConfig>()  { return engineDetection_; }
template <> inline RuntimeConfig&          ConfigManager::getMutable<RuntimeConfig>()          { return runtime_; }
template <> inline NetworkConfig&          ConfigManager::getMutable<NetworkConfig>()          { return network_; }
template <> inline StorageConfig&          ConfigManager::getMutable<StorageConfig>()          { return storage_; }
template <> inline UIConfig&               ConfigManager::getMutable<UIConfig>()               { return ui_; }
template <> inline LoggingConfig&          ConfigManager::getMutable<LoggingConfig>()          { return logging_; }

} // namespace omnibyte::dumper::config
