// YaraEngine — YARA rule scanning engine implementation.
// Supports bundled libyara or system library.

#include "YaraEngine.h"
#include <cstring>
#include <fstream>
#include <filesystem>
#include <mutex>

#ifdef HAS_YARA
#include <yara.h>

namespace omnibyte::signatures {

struct YaraEngine::Impl {
    YR_COMPILER* compiler = nullptr;
    YR_RULES* rules = nullptr;
    YaraStatus status = YaraStatus::NotInitialized;
    YaraConfig config;
    mutable std::mutex mutex;
    std::string lastError;
    size_t ruleCount = 0;

    ~Impl() {
        cleanup();
    }

    void cleanup() {
        if (rules) {
            yr_rules_destroy(rules);
            rules = nullptr;
        }
        if (compiler) {
            yr_compiler_destroy(compiler);
            compiler = nullptr;
        }
        ruleCount = 0;
    }
};

YaraEngine::YaraEngine() : impl_(std::make_unique<Impl>()) {}

YaraEngine::~YaraEngine() = default;

YaraEngine::YaraEngine(YaraEngine&&) noexcept = default;
YaraEngine& YaraEngine::operator=(YaraEngine&&) noexcept = default;

bool YaraEngine::initialize(const YaraConfig& config) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    if (impl_->status == YaraStatus::Ready) {
        return true;
    }

    impl_->status = YaraStatus::Initializing;
    impl_->config = config;

    if (yr_initialize() != ERROR_SUCCESS) {
        impl_->status = YaraStatus::Error;
        impl_->lastError = "Failed to initialize YARA library";
        return false;
    }

    if (yr_compiler_create(&impl_->compiler) != ERROR_SUCCESS) {
        impl_->status = YaraStatus::Error;
        impl_->lastError = "Failed to create YARA compiler";
        return false;
    }

    impl_->status = YaraStatus::Ready;
    return true;
}

void YaraEngine::shutdown() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->cleanup();
    impl_->status = YaraStatus::NotInitialized;
}

YaraStatus YaraEngine::status() const {
    return impl_->status;
}

bool YaraEngine::isReady() const {
    return impl_->status == YaraStatus::Ready;
}

bool YaraEngine::addRules(const std::string& rulesText) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    if (!impl_->compiler) {
        impl_->lastError = "Compiler not initialized";
        return false;
    }

    int errors = yr_compiler_add_string(impl_->compiler, rulesText.c_str(), nullptr);
    if (errors > 0) {
        impl_->lastError = "Failed to compile YARA rules (" + std::to_string(errors) + " errors)";
        return false;
    }

    // Recompile rules
    if (impl_->rules) {
        yr_rules_destroy(impl_->rules);
        impl_->rules = nullptr;
    }

    if (yr_compiler_get_rules(impl_->compiler, &impl_->rules) != ERROR_SUCCESS) {
        impl_->lastError = "Failed to get compiled rules";
        return false;
    }

    impl_->ruleCount++;
    return true;
}

bool YaraEngine::addRulesFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        impl_->lastError = "Failed to open rules file: " + filePath;
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return addRules(content);
}

bool YaraEngine::addRulesFromDirectory(const std::string& dirPath) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        impl_->lastError = "Invalid directory: " + dirPath;
        return false;
    }

    bool allSuccess = true;
    for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".yar") {
            if (!addRulesFromFile(entry.path().string())) {
                allSuccess = false;
            }
        }
    }

    return allSuccess;
}

void YaraEngine::clearRules() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    if (impl_->rules) {
        yr_rules_destroy(impl_->rules);
        impl_->rules = nullptr;
    }
    
    if (impl_->compiler) {
        yr_compiler_destroy(impl_->compiler);
        impl_->compiler = nullptr;
    }

    // Recreate compiler
    yr_compiler_create(&impl_->compiler);
    impl_->ruleCount = 0;
}

size_t YaraEngine::ruleCount() const {
    return impl_->ruleCount;
}

static int yara_callback(YR_MATCH* match, void* user_data) {
    auto* hits = static_cast<std::vector<YaraHit>*>(user_data);
    
    YaraHit hit;
    hit.offset = static_cast<uintptr_t>(match->base + match->offset);
    hit.ruleName = match->rule->identifier;
    hit.confidence = 0.85;

    // Extract meta
    if (match->meta) {
        for (YR_META* meta = match->meta; meta->identifier; ++meta) {
            if (meta->type == META_TYPE_STRING) {
                hit.meta = meta->string;
                break;
            }
        }
    }

    // Extract tags
    if (match->rule->tags) {
        for (const char** tag = match->rule->tags; *tag; ++tag) {
            if (!hit.tags.empty()) hit.tags += ",";
            hit.tags += *tag;
        }
    }

    hits->push_back(hit);
    return CALLBACK_CONTINUE;
}

std::vector<YaraHit> YaraEngine::scanRegion(const uint8_t* data, size_t size) const {
    std::vector<YaraHit> hits;
    
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (!impl_->rules) return hits;

    yr_rules_scan_mem(impl_->rules, data, size, 0, yara_callback, &hits, nullptr);
    return hits;
}

std::vector<YaraHit> YaraEngine::scanFile(const std::string& filePath) const {
    std::vector<YaraHit> hits;
    
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (!impl_->rules) return hits;

    yr_rules_scan_file(impl_->rules, filePath.c_str(), 0, yara_callback, &hits, nullptr);
    return hits;
}

std::vector<YaraHit> YaraEngine::scanProcessMemory(int pid, uintptr_t address, size_t size) const {
    std::vector<YaraHit> hits;
    
    // Read process memory via /proc/pid/mem
    char path[64];
    std::snprintf(path, sizeof(path), "/proc/%d/mem", pid);
    
    FILE* f = std::fopen(path, "rb");
    if (!f) return hits;

    if (std::fseek(f, static_cast<long>(address), SEEK_SET) != 0) {
        std::fclose(f);
        return hits;
    }

    std::vector<uint8_t> buffer(size);
    size_t bytesRead = std::fread(buffer.data(), 1, size, f);
    std::fclose(f);

    if (bytesRead > 0) {
        buffer.resize(bytesRead);
        return scanRegion(buffer.data(), buffer.size());
    }

    return hits;
}

std::string YaraEngine::lastError() const {
    return impl_->lastError;
}

std::string YaraEngine::libraryVersion() {
    int major, minor, patch;
    yr_get_property(YR_PROPERTY_VERSION_STRING, nullptr);
    // Fallback version string
    return "4.x";
}

} // namespace omnibyte::signatures

#else

// Stub implementation when libyara is not available
namespace omnibyte::signatures {

struct YaraEngine::Impl {
    YaraStatus status = YaraStatus::NotInitialized;
    std::string lastError;
};

YaraEngine::YaraEngine() : impl_(std::make_unique<Impl>()) {}
YaraEngine::~YaraEngine() = default;
YaraEngine::YaraEngine(YaraEngine&&) noexcept = default;
YaraEngine& YaraEngine::operator=(YaraEngine&&) noexcept = default;

bool YaraEngine::initialize(const YaraConfig&) {
    impl_->status = YaraStatus::Error;
    impl_->lastError = "YARA not available (libyara not linked)";
    return false;
}

void YaraEngine::shutdown() {
    impl_->status = YaraStatus::NotInitialized;
}

YaraStatus YaraEngine::status() const {
    return impl_->status;
}

bool YaraEngine::isReady() const {
    return false;
}

bool YaraEngine::addRules(const std::string&) {
    impl_->lastError = "YARA not available";
    return false;
}

bool YaraEngine::addRulesFromFile(const std::string&) {
    impl_->lastError = "YARA not available";
    return false;
}

bool YaraEngine::addRulesFromDirectory(const std::string&) {
    impl_->lastError = "YARA not available";
    return false;
}

void YaraEngine::clearRules() {}

size_t YaraEngine::ruleCount() const {
    return 0;
}

std::vector<YaraHit> YaraEngine::scanRegion(const uint8_t*, size_t) const {
    return {};
}

std::vector<YaraHit> YaraEngine::scanFile(const std::string&) const {
    return {};
}

std::vector<YaraHit> YaraEngine::scanProcessMemory(int, uintptr_t, size_t) const {
    return {};
}

std::string YaraEngine::lastError() const {
    return impl_->lastError;
}

std::string YaraEngine::libraryVersion() {
    return "N/A (libyara not linked)";
}

} // namespace omnibyte::signatures

#endif
