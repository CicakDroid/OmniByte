#pragma once
// DumperManager — Coordinates WorkingModes with the Dumper engine system.
// Manages the flow from target selection to engine execution.

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "../WorkingModes/WorkingMode.h"
#include "AnalysisTarget.h"
#include "IDumperEngine.h"
#include "DumpResult.h"

namespace omnibyte::dumper {

/// Dumper execution configuration
struct DumperConfig {
    WorkingModeType workingMode = WorkingModeType::Manual;
    bool enableSignatureDetection = true;
    bool enablePatternScanning = true;
    bool enableYaraScanning = true;
    std::string databasePath;           // Path to signature database
    std::string yaraRulesPath;          // Path to YARA rules
    uint32_t timeoutMs = 30000;
};

/// Dumper execution result
struct DumperResult {
    bool success = false;
    std::string errorMessage;
    WorkingModeResult workingModeResult;
    std::vector<DumpResult> engineResults;
    std::string detectedEngine;
    double detectionConfidence = 0.0;
};

/// Progress callback for long operations
using ProgressCallback = std::function<void(const std::string& message, float progress)>;

/// Error callback for error handling
using ErrorCallback = std::function<void(const std::string& error)>;

/// DumperManager — coordinates WorkingModes with engine execution
class DumperManager {
public:
    DumperManager();
    ~DumperManager();

    // Non-copyable
    DumperManager(const DumperManager&) = delete;
    DumperManager& operator=(const DumperManager&) = delete;

    /// Initialize with configuration
    bool initialize(const DumperConfig& config);

    /// Set progress callback
    void setProgressCallback(ProgressCallback callback);

    /// Set error callback
    void setErrorCallback(ErrorCallback callback);

    /// Execute dump on a target (file or process)
    DumperResult execute(const AnalysisTarget& target);

    /// Execute dump on a file path
    DumperResult executeOnFile(const std::string& filePath);

    /// Execute dump on a live process
    DumperResult executeOnProcess(int pid, uintptr_t baseAddress, const std::string& moduleName = "");

    /// Get current working mode
    WorkingModeType currentMode() const;

    /// Get available engines
    std::vector<std::string> availableEngines() const;

    /// Get detection result for target (without full dump)
    std::optional<EngineRegistry::MatchResult> detectTarget(const AnalysisTarget& target) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace omnibyte::dumper
