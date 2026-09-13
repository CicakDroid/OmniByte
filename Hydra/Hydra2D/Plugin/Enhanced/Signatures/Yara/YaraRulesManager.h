#pragma once
// YaraRulesManager — Manages YARA rules loading and organization.
// Supports rules from files, directories, and embedded sources.

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace omnibyte::signatures {

/// Rule metadata
struct RuleInfo {
    std::string name;
    std::string author;
    std::string description;
    std::string license;
    std::string version;
    std::vector<std::string> tags;
    std::string sourceFile;
    size_t ruleCount = 0;
};

/// Rules category
enum class RuleCategory {
    Malware,
    Games,
    Libraries,
    Packer,
    Crypto,
    Generic,
    Custom
};

/// Convert category to string
inline const char* ruleCategoryToString(RuleCategory cat) {
    switch (cat) {
        case RuleCategory::Malware: return "malware";
        case RuleCategory::Games: return "games";
        case RuleCategory::Libraries: return "libraries";
        case RuleCategory::Packer: return "packer";
        case RuleCategory::Crypto: return "crypto";
        case RuleCategory::Generic: return "generic";
        case RuleCategory::Custom: return "custom";
    }
    return "unknown";
}

/// Convert string to category
inline RuleCategory stringToRuleCategory(const std::string& str) {
    if (str == "malware") return RuleCategory::Malware;
    if (str == "games") return RuleCategory::Games;
    if (str == "libraries") return RuleCategory::Libraries;
    if (str == "packer") return RuleCategory::Packer;
    if (str == "crypto") return RuleCategory::Crypto;
    if (str == "generic") return RuleCategory::Generic;
    if (str == "custom") return RuleCategory::Custom;
    return RuleCategory::Generic;
}

/// Manages YARA rules loading and organization
class YaraRulesManager {
public:
    YaraRulesManager();
    ~YaraRulesManager();

    // Non-copyable, movable
    YaraRulesManager(const YaraRulesManager&) = delete;
    YaraRulesManager& operator=(const YaraRulesManager&) = delete;
    YaraRulesManager(YaraRulesManager&&) noexcept;
    YaraRulesManager& operator=(YaraRulesManager&&) noexcept;

    /// Load rules from file
    bool loadFromFile(const std::string& filePath, RuleCategory category = RuleCategory::Generic);

    /// Load rules from directory (recursively)
    bool loadFromDirectory(const std::string& dirPath, RuleCategory category = RuleCategory::Generic);

    /// Load rules from embedded data
    bool loadFromMemory(const std::string& rulesText, const std::string& name,
                       RuleCategory category = RuleCategory::Generic);

    /// Unload rules by name
    bool unloadRules(const std::string& name);

    /// Unload all rules
    void unloadAll();

    /// Get all loaded rule info
    std::vector<RuleInfo> loadedRules() const;

    /// Get rules by category
    std::vector<RuleInfo> rulesByCategory(RuleCategory category) const;

    /// Get total rule count
    size_t totalRuleCount() const;

    /// Check if rules are loaded
    bool hasRules() const;

    /// Get rules directory
    std::string rulesDirectory() const;

    /// Set rules directory
    void setRulesDirectory(const std::string& dirPath);

    /// Refresh rules from directory
    bool refresh();

    /// Export rules to file
    bool exportToFile(const std::string& outputPath,
                     const std::vector<std::string>& ruleNames = {}) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace omnibyte::signatures
