#pragma once
#include "IEngine.h"
#include "DumperCore/DumpResult.h"
#include "DumperCore/AnalysisTarget.h"
#include "DumperCore/Detector/Detector.h"
#include "DumperCore/ResultNormalizer/ResultNormalizer.h"
#include "Export/ExportCore/ExportRegistry/ExportRegistry.h"
#include "Export/ExportCore/IExporter/IExporter.h"
#include "config/EngineDetection/EngineDetectionConfig.h"
#include "config/FileLimits/FileLimitsConfig.h"
#include "config/Logging/LoggingConfig.h"

#include <string>
#include <memory>
#include <functional>
#include <sys/types.h>

namespace omnibyte::dumper {

enum class DumpResult {
    Success,
    EngineNotDetected,
    AnalyzerFailed,
    ResolverFailed,
    ExportFailed,
    InvalidRequest,
    InsufficientPermission,
    FileNotFound,
    RootUnavailable,
    StealthUnavailable,
    HookFailed
};

struct DumpRequest {
    pid_t pid = 0;
    std::string apkPath;
    std::optional<EngineType> manualOverride;
    bool enableYara = false;
    std::string exportFormat = "json";
    std::string exportPath;
    std::function<void(const std::string&, float)> progressCb;
    std::function<void(const std::string&)> errorCb;
};

struct DumperOutcome {
    DumpResult status = DumpResult::InvalidRequest;
    DumpData data;
    std::string engineName;
    std::string detectedVersion;
    double detectionConfidence = 0.0;
};

class Dumper {
public:
    DumperOutcome execute(const DumpRequest& request);
    DumperOutcome dumpFile(const std::string& apkPath,
                           const std::optional<EngineType>& override = std::nullopt);
    DumperOutcome dumpProcess(pid_t pid,
                              const std::optional<EngineType>& override = std::nullopt);
    std::vector<EngineType> availableEngines() const;
    void loadConfig(const config::EngineDetectionConfig& detection,
                    const config::FileLimitsConfig& fileLimits,
                    const config::LoggingConfig& logging);

private:
    std::optional<DetectionOutcome> detectEngine(const DumpRequest& request);
    DumpResult validateTarget(const DumpRequest& request);
    std::pair<std::shared_ptr<IEngineAnalyzer>, std::shared_ptr<IEngineResolver>>
        createEnginePair(EngineType type);
    DumpResult exportJson(const DumpData& data, const std::string& path);

    config::EngineDetectionConfig detectionCfg_;
    config::FileLimitsConfig fileLimitsCfg_;
    config::LoggingConfig loggingCfg_;
};

} // namespace omnibyte::dumper
