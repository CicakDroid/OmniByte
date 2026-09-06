#pragma once
// WorkingMode — Defines operating modes for the Dumper system.
// Supports Manual, Semi-Auto, and Full Auto modes.

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace omnibyte::dumper {

/// Working mode types
enum class WorkingModeType {
    Manual,         // User manually selects files and targets
    SemiAuto,       // Auto-detect with user confirmation
    FullAuto        // Fully automatic operation
};

/// Convert WorkingModeType to string
inline const char* workingModeTypeToString(WorkingModeType type) {
    switch (type) {
        case WorkingModeType::Manual: return "manual";
        case WorkingModeType::SemiAuto: return "semi_auto";
        case WorkingModeType::FullAuto: return "full_auto";
    }
    return "manual";
}

/// Convert string to WorkingModeType
inline WorkingModeType stringToWorkingModeType(const std::string& str) {
    if (str == "semi_auto") return WorkingModeType::SemiAuto;
    if (str == "full_auto") return WorkingModeType::FullAuto;
    return WorkingModeType::Manual;
}

/// File input source
enum class FileInputSource {
    ManualInput,        // User manually selects file
    AutoDetect,         // Auto-detect from directory
    ProcessMemory       // Read from live process
};

/// Working mode configuration
struct WorkingModeConfig {
    WorkingModeType type = WorkingModeType::Manual;
    FileInputSource inputSource = FileInputSource::ManualInput;
    bool requireRoot = false;           // Require root access
    bool enableSignatureDetection = true;
    bool enablePatternScanning = true;
    bool enableYaraScanning = true;
    uint32_t timeoutMs = 30000;         // Operation timeout
};

/// Working mode result
struct WorkingModeResult {
    bool success = false;
    std::string errorMessage;
    std::string modeName;
    std::vector<std::string> filesProcessed;
    std::vector<std::string> errors;
};

/// Callback for user confirmation in SemiAuto mode
using UserConfirmCallback = std::function<bool(const std::string& message)>;

/// Callback for file selection in Manual mode
using FileSelectCallback = std::function<std::string(const std::vector<std::string>& options)>;

/// Base class for working modes
class IWorkingMode {
public:
    virtual ~IWorkingMode() = default;
    
    /// Get mode type
    virtual WorkingModeType type() const = 0;
    
    /// Get mode name
    virtual std::string name() const = 0;
    
    /// Initialize mode
    virtual bool initialize(const WorkingModeConfig& config) = 0;
    
    /// Execute operation
    virtual WorkingModeResult execute(const std::string& targetPath) = 0;
    
    /// Check if root is required
    virtual bool requiresRoot() const = 0;
    
    /// Get description
    virtual std::string description() const = 0;
};

/// Manual working mode
class ManualWorkingMode : public IWorkingMode {
public:
    WorkingModeType type() const override { return WorkingModeType::Manual; }
    std::string name() const override { return "Manual"; }
    
    bool initialize(const WorkingModeConfig& config) override;
    WorkingModeResult execute(const std::string& targetPath) override;
    bool requiresRoot() const override { return false; }
    std::string description() const override {
        return "User manually selects files and targets";
    }
    
    /// Set file select callback
    void setFileSelectCallback(FileSelectCallback callback);
    
    /// Set available files
    void setAvailableFiles(const std::vector<std::string>& files);
    
private:
    WorkingModeConfig config_;
    FileSelectCallback fileSelectCallback_;
    std::vector<std::string> availableFiles_;
};

/// Semi-Auto working mode
class SemiAutoWorkingMode : public IWorkingMode {
public:
    WorkingModeType type() const override { return WorkingModeType::SemiAuto; }
    std::string name() const override { return "Semi-Auto"; }
    
    bool initialize(const WorkingModeConfig& config) override;
    WorkingModeResult execute(const std::string& targetPath) override;
    bool requiresRoot() const override { return false; }
    std::string description() const override {
        return "Auto-detect with user confirmation";
    }
    
    /// Set user confirm callback
    void setUserConfirmCallback(UserConfirmCallback callback);
    
private:
    WorkingModeConfig config_;
    UserConfirmCallback userConfirmCallback_;
};

/// Full Auto working mode
class FullAutoWorkingMode : public IWorkingMode {
public:
    WorkingModeType type() const override { return WorkingModeType::FullAuto; }
    std::string name() const override { return "Full-Auto"; }
    
    bool initialize(const WorkingModeConfig& config) override;
    WorkingModeResult execute(const std::string& targetPath) override;
    bool requiresRoot() const override { return config_.requireRoot; }
    std::string description() const override {
        return "Fully automatic operation (may require root)";
    }
    
private:
    WorkingModeConfig config_;
};

/// Working mode factory
class WorkingModeFactory {
public:
    /// Create working mode by type
    static std::unique_ptr<IWorkingMode> create(WorkingModeType type);
    
    /// Create working mode from config
    static std::unique_ptr<IWorkingMode> create(const WorkingModeConfig& config);
    
    /// Get available modes
    static std::vector<WorkingModeType> availableModes();
    
    /// Get mode description
    static std::string modeDescription(WorkingModeType type);
};

} // namespace omnibyte::dumper
