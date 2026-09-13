// plugin_signatures — Signatures plugin implementation.

#include "plugin_signatures.h"
#include "MagicBytes/MagicBytesDetector.h"
#include "MagicBytes/MagicBytesExtractor.h"
#include "SignaturesDatabase/SignaturesDatabase.h"
#include "Pattern/PatternScanner.h"
#include "Pattern/PatternMatcher.h"
#include "Yara/YaraEngine.h"
#include "Yara/YaraRulesManager.h"
#include <fstream>

namespace omnibyte::signatures {

struct SignaturesPlugin::Impl {
    std::unique_ptr<MagicBytesDetector> detector;
    std::unique_ptr<MagicBytesExtractor> extractor;
    std::unique_ptr<SignaturesDatabase> database;
    std::unique_ptr<PatternScanner> scanner;
    std::unique_ptr<PatternMatcher> matcher;
    std::unique_ptr<Yara::YaraEngine> yaraEngine;
    std::unique_ptr<Yara::YaraRulesManager> yaraRulesManager;
    std::string databasePath;
    std::string yaraRulesPath;
    bool initialized = false;
};

SignaturesPlugin::SignaturesPlugin() : impl_(std::make_unique<Impl>()) {}

SignaturesPlugin::~SignaturesPlugin() = default;

SignaturesPlugin::SignaturesPlugin(SignaturesPlugin&&) noexcept = default;
SignaturesPlugin& SignaturesPlugin::operator=(SignaturesPlugin&&) noexcept = default;

bool SignaturesPlugin::initialize(const std::string& databasePath,
                                 const std::string& yaraRulesPath) {
    impl_->databasePath = databasePath;
    impl_->yaraRulesPath = yaraRulesPath;

    // Initialize subsystems
    impl_->detector = std::make_unique<MagicBytesDetector>();
    impl_->extractor = std::make_unique<MagicBytesExtractor>();
    impl_->database = std::make_unique<SignaturesDatabase>();
    impl_->scanner = std::make_unique<PatternScanner>();
    impl_->matcher = std::make_unique<PatternMatcher>();

    // Load database if provided
    if (!databasePath.empty()) {
        impl_->database->loadFromDirectory(databasePath);
    }

    // Initialize YARA if available
    impl_->yaraEngine = std::make_unique<Yara::YaraEngine>();
    if (impl_->yaraEngine->initialize()) {
        impl_->yaraRulesManager = std::make_unique<Yara::YaraRulesManager>();
        
        if (!yaraRulesPath.empty()) {
            impl_->yaraRulesManager->loadFromDirectory(yaraRulesPath);
            auto rules = impl_->yaraRulesManager->rulesByCategory("all");
            for (const auto& rule : rules) {
                impl_->yaraEngine->addRulesFromFile(rule.path);
            }
        }
    }

    impl_->initialized = true;
    return true;
}

SignatureDetectionResult SignaturesPlugin::detectFileType(const std::string& filePath) {
    SignatureDetectionResult result;
    
    if (!impl_->initialized) {
        result.errorMessage = "Plugin not initialized";
        return result;
    }

    auto detection = impl_->detector->detectFromFile(filePath);
    if (detection.detected) {
        result.success = true;
        result.detectedType = detection.signature.name;
        result.typeConfidence = detection.confidence;
    } else {
        result.errorMessage = "No magic bytes detected";
    }

    return result;
}

SignatureDetectionResult SignaturesPlugin::detectFileType(const uint8_t* data, size_t size) {
    SignatureDetectionResult result;
    
    if (!impl_->initialized) {
        result.errorMessage = "Plugin not initialized";
        return result;
    }

    auto detection = impl_->detector->detectFromMemory(data, size);
    if (detection.detected) {
        result.success = true;
        result.detectedType = detection.signature.name;
        result.typeConfidence = detection.confidence;
    } else {
        result.errorMessage = "No magic bytes detected";
    }

    return result;
}

SignatureDetectionResult SignaturesPlugin::scanFile(const std::string& filePath) {
    SignatureDetectionResult result;
    
    if (!impl_->initialized) {
        result.errorMessage = "Plugin not initialized";
        return result;
    }

    // Detect file type first
    result = detectFileType(filePath);
    if (!result.success) {
        return result;
    }

    // Scan with patterns if available
    auto scanResult = impl_->scanner->scanFile(filePath, PatternDefinition());
    for (const auto& match : scanResult) {
        result.matchedPatterns.push_back(match.patternName);
    }

    // Scan with YARA if available
    if (yaraAvailable()) {
        std::ifstream file(filePath, std::ios::binary);
        if (file.is_open()) {
            std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)),
                                        std::istreambuf_iterator<char>());
            auto yaraResult = impl_->yaraEngine->scanBuffer(buffer.data(), buffer.size());
            for (const auto& match : yaraResult.matches) {
                result.yaraMatches.push_back(match.ruleName);
            }
        }
    }

    return result;
}

SignatureDetectionResult SignaturesPlugin::scanBuffer(const uint8_t* data, size_t size) {
    SignatureDetectionResult result;
    
    if (!impl_->initialized) {
        result.errorMessage = "Plugin not initialized";
        return result;
    }

    // Detect type from buffer
    result = detectFileType(data, size);
    if (!result.success) {
        return result;
    }

    // Scan with patterns if available
    auto scanResult = impl_->scanner->scan(data, size, PatternDefinition());
    for (const auto& match : scanResult) {
        result.matchedPatterns.push_back(match.patternName);
    }

    // Scan with YARA if available
    if (yaraAvailable()) {
        auto yaraResult = impl_->yaraEngine->scanBuffer(data, size);
        for (const auto& match : yaraResult.matches) {
            result.yaraMatches.push_back(match.ruleName);
        }
    }

    return result;
}

bool SignaturesPlugin::extractSignature(const std::string& filePath, const std::string& outputPath) {
    if (!impl_->initialized) {
        return false;
    }

    return impl_->extractor->extractFromFile(filePath, outputPath);
}

SignatureDetectionResult SignaturesPlugin::scanWithPatterns(const uint8_t* data, size_t size,
                                                          const std::string& patternFile) {
    SignatureDetectionResult result;
    
    if (!impl_->initialized) {
        result.errorMessage = "Plugin not initialized";
        return result;
    }

    // Load patterns from file
    PatternScanner scanner;
    if (!scanner.loadPatterns(patternFile)) {
        result.errorMessage = "Failed to load patterns from: " + patternFile;
        return result;
    }

    // Scan with loaded patterns
    auto patterns = scanner.patterns();
    for (const auto& pattern : patterns) {
        auto matches = scanner.scan(data, size, pattern);
        if (!matches.empty()) {
            result.matchedPatterns.push_back(pattern.name);
        }
    }

    result.success = !result.matchedPatterns.empty();
    return result;
}

SignatureDetectionResult SignaturesPlugin::scanWithYara(const uint8_t* data, size_t size) {
    SignatureDetectionResult result;
    
    if (!impl_->initialized) {
        result.errorMessage = "Plugin not initialized";
        return result;
    }

    if (!yaraAvailable()) {
        result.errorMessage = "YARA not available";
        return result;
    }

    auto yaraResult = impl_->yaraEngine->scanBuffer(data, size);
    for (const auto& match : yaraResult.matches) {
        result.yaraMatches.push_back(match.ruleName);
    }

    result.success = !result.yaraMatches.empty();
    return result;
}

bool SignaturesPlugin::yaraAvailable() const {
    return impl_->yaraEngine && impl_->yaraEngine->isReady();
}

std::string SignaturesPlugin::version() const {
    return "1.0.0";
}

bool SignaturesPlugin::subsystemStatus() const {
    return impl_->initialized;
}

} // namespace omnibyte::signatures
