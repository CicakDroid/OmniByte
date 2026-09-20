// DumperManager — Dumper coordination implementation.

#include "DumperManager.h"
#include "SharedUtils/SharedUtils.h"
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
    auto& registry = EngineRegistry::instance();

    impl_->notifyProgress("Starting dump execution", 0.0f);

    WorkingModeResult modeResult = impl_->workingMode->execute(target.filePath);
    result.workingModeResult = modeResult;

    if (!modeResult.success) {
        result.errorMessage = "Working mode failed: " + modeResult.errorMessage;
        impl_->notifyError(result.errorMessage);
        return result;
    }

    impl_->notifyProgress("Files processed by working mode", 0.3f);

    for (const auto& file : modeResult.filesProcessed) {
        AnalysisTarget fileTarget = AnalysisTarget::fromFile(file);

        switch (impl_->config.dumperMode) {
        case DumperMode::Auto: {
            impl_->notifyProgress("Auto-detect: " + file, 0.4f);
            auto match = detectTarget(fileTarget);
            if (match && match->best) {
                result.detectedEngine = match->best->name();
                result.detectionConfidence = match->bestDetection.confidence;
                impl_->notifyProgress("Detected: " + result.detectedEngine, 0.6f);
                auto profile = match->best->resolveProfile(match->bestDetection.detectedVersion);
                DumpData engineResult = match->best->analyze(fileTarget, profile);
                result.engineResults.push_back(engineResult);
                if (!engineResult.success) {
                    result.errors.push_back("Engine analysis failed for " + file);
                }
            } else {
                result.errors.push_back("No engine matched for " + file);
            }
            break;
        }
        case DumperMode::Manual: {
            impl_->notifyProgress("Manual: " + impl_->config.manualEngineName, 0.4f);
            auto engine = registry.findEngine(impl_->config.manualEngineName);
            if (engine) {
                auto match = engine->detect(fileTarget);
                auto profile = engine->resolveProfile(match.detectedVersion);
                DumpData engineResult = engine->analyze(fileTarget, profile);
                result.detectedEngine = engine->name();
                result.detectionConfidence = match.confidence;
                result.engineResults.push_back(engineResult);
                if (!engineResult.success) {
                    result.errors.push_back("Engine analysis failed for " + file);
                }
            } else {
                result.errors.push_back("Engine not found: " + impl_->config.manualEngineName);
            }
            break;
        }
        case DumperMode::MultiDump: {
            impl_->notifyProgress("MultiDump all engines: " + file, 0.4f);
            for (const auto& engine : registry.allEngines()) {
                auto detection = engine->detect(fileTarget);
                if (detection.confidence > 0.0f) {
                    auto profile = engine->resolveProfile(detection.detectedVersion);
                    DumpData engineResult = engine->analyze(fileTarget, profile);
                    result.engineResults.push_back(engineResult);
                    if (!engineResult.success) {
                        result.errors.push_back(engine->name() + " analysis failed for " + file);
                    }
                }
            }
            break;
        }
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
