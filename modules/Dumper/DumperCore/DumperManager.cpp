// DumperManager — Dumper coordination implementation.

#include "DumperManager.h"
#include "EngineRegistry/EngineRegistry.h"
#include "../SharedUtils/SharedUtils.h"
#include <filesystem>
#include <chrono>

namespace omnibyte::dumper {

struct DumperManager::Impl {
    DumperConfig config;
    std::unique_ptr<IWorkingMode> workingMode;
    ProgressCallback progressCallback;
    ErrorCallback errorCallback;

    void notifyProgress(const std::string& message, float progress) {
        if (progressCallback) {
            progressCallback(message, progress);
        }
    }

    void notifyError(const std::string& error) {
        if (errorCallback) {
            errorCallback(error);
        }
    }
};

DumperManager::DumperManager() : impl_(std::make_unique<Impl>()) {}

DumperManager::~DumperManager() = default;

bool DumperManager::initialize(const DumperConfig& config) {
    impl_->config = config;
    
    // Create working mode
    WorkingModeConfig modeConfig;
    modeConfig.type = config.workingMode;
    modeConfig.enableSignatureDetection = config.enableSignatureDetection;
    modeConfig.enablePatternScanning = config.enablePatternScanning;
    modeConfig.enableYaraScanning = config.enableYaraScanning;
    modeConfig.timeoutMs = config.timeoutMs;
    
    impl_->workingMode = WorkingModeFactory::create(modeConfig);
    if (!impl_->workingMode) {
        impl_->notifyError("Failed to create working mode");
        return false;
    }

    impl_->notifyProgress("DumperManager initialized", 1.0f);
    return true;
}

void DumperManager::setProgressCallback(ProgressCallback callback) {
    impl_->progressCallback = callback;
}

void DumperManager::setErrorCallback(ErrorCallback callback) {
    impl_->errorCallback = callback;
}

DumperResult DumperManager::execute(const AnalysisTarget& target) {
    DumperResult result;
    
    impl_->notifyProgress("Starting dump execution", 0.0f);

    // Execute working mode
    WorkingModeResult modeResult = impl_->workingMode->execute(target.filePath);
    result.workingModeResult = modeResult;
    
    if (!modeResult.success) {
        result.errorMessage = "Working mode failed: " + modeResult.errorMessage;
        impl_->notifyError(result.errorMessage);
        return result;
    }

    impl_->notifyProgress("Files processed by working mode", 0.3f);

    // Detect engine for each file
    for (const auto& file : modeResult.filesProcessed) {
        impl_->notifyProgress("Processing: " + file, 0.4f);
        
        AnalysisTarget fileTarget = AnalysisTarget::fromFile(file);
        auto matchResult = detectTarget(fileTarget);
        
        if (matchResult && matchResult->best) {
            result.detectedEngine = matchResult->best->name();
            result.detectionConfidence = matchResult->bestDetection.confidence;
            
            impl_->notifyProgress("Detected: " + result.detectedEngine, 0.6f);
            
            // Execute engine analysis
            auto profile = matchResult->best->resolveProfile(matchResult->bestDetection.detectedVersion);
            DumpData engineResult = matchResult->best->analyze(fileTarget, profile);
            
            result.engineResults.push_back(engineResult);
            
            if (!engineResult.success) {
                result.errors.push_back("Engine analysis failed for " + file);
            }
        } else {
            result.errors.push_back("No engine matched for " + file);
        }
    }

    impl_->notifyProgress("Dump execution complete", 1.0f);
    
    result.success = !result.engineResults.empty();
    return result;
}

DumperResult DumperManager::executeOnFile(const std::string& filePath) {
    AnalysisTarget target = AnalysisTarget::fromFile(filePath);
    return execute(target);
}

DumperResult DumperManager::executeOnProcess(int pid, uintptr_t baseAddress, const std::string& moduleName) {
    AnalysisTarget target = AnalysisTarget::fromProcess(pid, baseAddress, moduleName);
    return execute(target);
}

WorkingModeType DumperManager::currentMode() const {
    return impl_->config.workingMode;
}

std::vector<std::string> DumperManager::availableEngines() const {
    auto& registry = EngineRegistry::instance();
    std::vector<std::string> engines;
    
    for (const auto& engine : registry.allEngines()) {
        engines.push_back(engine->name());
    }
    
    return engines;
}

std::optional<EngineRegistry::MatchResult> DumperManager::detectTarget(const AnalysisTarget& target) const {
    auto& registry = EngineRegistry::instance();
    return registry.detectBestMatch(target);
}

} // namespace omnibyte::dumper
