#pragma once
// YaraUpdater — Handles YARA rule updates and version checking.
// Supports checking for updates from remote repositories.

#include <string>
#include <vector>
#include <functional>
#include <chrono>

namespace omnibyte::signatures {

/// Update source configuration
struct UpdateSource {
    std::string name;           // Display name
    std::string url;            // URL to check for updates
    std::string type;           // "github", "gitlab", "custom"
    bool enabled = true;
};

/// Update check result
struct UpdateCheckResult {
    bool updateAvailable = false;
    std::string currentVersion;
    std::string latestVersion;
    std::string downloadUrl;
    std::string releaseNotes;
    std::chrono::system_clock::time_point releaseDate;
};

/// Update download progress
struct UpdateProgress {
    enum class Status {
        Idle,
        Checking,
        Downloading,
        Extracting,
        Installing,
        Complete,
        Error
    };

    Status status = Status::Idle;
    float progress = 0.0f;          // 0.0 - 1.0
    std::string currentFile;
    std::string errorMessage;
};

/// Callback for update progress
using UpdateProgressCallback = std::function<void(const UpdateProgress&)>;

/// YARA rules updater
class YaraUpdater {
public:
    YaraUpdater();
    ~YaraUpdater();

    // Non-copyable, movable
    YaraUpdater(const YaraUpdater&) = delete;
    YaraUpdater& operator=(const YaraUpdater&) = delete;
    YaraUpdater(YaraUpdater&&) noexcept;
    YaraUpdater& operator=(YaraUpdater&&) noexcept;

    /// Add an update source
    void addSource(const UpdateSource& source);

    /// Remove update source by name
    void removeSource(const std::string& name);

    /// Get all configured sources
    std::vector<UpdateSource> sources() const;

    /// Check for updates from all enabled sources
    std::vector<UpdateCheckResult> checkForUpdates() const;

    /// Check for updates from specific source
    UpdateCheckResult checkForSource(const std::string& sourceName) const;

    /// Download and install update
    bool installUpdate(const UpdateCheckResult& update,
                       UpdateProgressCallback progressCallback = nullptr);

    /// Get local rules version
    std::string localVersion() const;

    /// Set local rules directory
    void setLocalRulesDir(const std::string& dirPath);

    /// Get local rules directory
    std::string localRulesDir() const;

    /// Export current rules to file
    bool exportRules(const std::string& outputPath) const;

    /// Import rules from file
    bool importRules(const std::string& inputPath);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace omnibyte::signatures
