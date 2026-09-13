// YaraRulesManager — YARA rules manager implementation.

#include "YaraRulesManager.h"
#include <filesystem>
#include <fstream>

namespace omnibyte::signatures {

struct YaraRulesManager::Impl {
    std::string rulesDirectory;
    std::unordered_map<std::string, RuleInfo> loadedRules;
};

YaraRulesManager::YaraRulesManager() : impl_(std::make_unique<Impl>()) {}

YaraRulesManager::~YaraRulesManager() = default;

YaraRulesManager::YaraRulesManager(YaraRulesManager&&) noexcept = default;
YaraRulesManager& YaraRulesManager::operator=(YaraRulesManager&&) noexcept = default;

bool YaraRulesManager::loadFromFile(const std::string& filePath, RuleCategory category) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(filePath)) {
        return false;
    }

    RuleInfo info;
    info.name = fs::path(filePath).stem().string();
    info.sourceFile = filePath;
    info.tags.push_back(ruleCategoryToString(category));

    // Count rules in file (simple heuristic: count "rule " keywords)
    std::ifstream file(filePath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("rule ") == 0) {
                info.ruleCount++;
            }
        }
    }

    impl_->loadedRules[info.name] = info;
    return true;
}

bool YaraRulesManager::loadFromDirectory(const std::string& dirPath, RuleCategory category) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        return false;
    }

    bool allSuccess = true;
    for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".yar") {
            if (!loadFromFile(entry.path().string(), category)) {
                allSuccess = false;
            }
        }
    }

    return allSuccess;
}

bool YaraRulesManager::loadFromMemory(const std::string& rulesText, const std::string& name,
                                     RuleCategory category) {
    RuleInfo info;
    info.name = name;
    info.tags.push_back(ruleCategoryToString(category));

    // Count rules
    size_t pos = 0;
    while ((pos = rulesText.find("rule ", pos)) != std::string::npos) {
        info.ruleCount++;
        pos += 5;
    }

    impl_->loadedRules[name] = info;
    return true;
}

bool YaraRulesManager::unloadRules(const std::string& name) {
    return impl_->loadedRules.erase(name) > 0;
}

void YaraRulesManager::unloadAll() {
    impl_->loadedRules.clear();
}

std::vector<RuleInfo> YaraRulesManager::loadedRules() const {
    std::vector<RuleInfo> result;
    for (const auto& [name, info] : impl_->loadedRules) {
        result.push_back(info);
    }
    return result;
}

std::vector<RuleInfo> YaraRulesManager::rulesByCategory(RuleCategory category) const {
    std::vector<RuleInfo> result;
    std::string catStr = ruleCategoryToString(category);
    
    for (const auto& [name, info] : impl_->loadedRules) {
        for (const auto& tag : info.tags) {
            if (tag == catStr) {
                result.push_back(info);
                break;
            }
        }
    }
    
    return result;
}

size_t YaraRulesManager::totalRuleCount() const {
    size_t count = 0;
    for (const auto& [name, info] : impl_->loadedRules) {
        count += info.ruleCount;
    }
    return count;
}

bool YaraRulesManager::hasRules() const {
    return !impl_->loadedRules.empty();
}

std::string YaraRulesManager::rulesDirectory() const {
    return impl_->rulesDirectory;
}

void YaraRulesManager::setRulesDirectory(const std::string& dirPath) {
    impl_->rulesDirectory = dirPath;
}

bool YaraRulesManager::refresh() {
    if (impl_->rulesDirectory.empty()) {
        return false;
    }
    
    impl_->loadedRules.clear();
    return loadFromDirectory(impl_->rulesDirectory);
}

bool YaraRulesManager::exportToFile(const std::string& outputPath,
                                   const std::vector<std::string>& ruleNames) const {
    // TODO: Implement rules export
    return false;
}

} // namespace omnibyte::signatures
