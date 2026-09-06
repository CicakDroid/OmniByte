// WorkingMode — Working modes implementation.

#include "WorkingMode.h"
#include <filesystem>

namespace omnibyte::dumper {

// ── ManualWorkingMode ──

bool ManualWorkingMode::initialize(const WorkingModeConfig& config) {
    config_ = config;
    return true;
}

WorkingModeResult ManualWorkingMode::execute(const std::string& targetPath) {
    WorkingModeResult result;
    result.modeName = name();
    
    namespace fs = std::filesystem;
    
    if (!fs::exists(targetPath)) {
        result.errorMessage = "Target path does not exist: " + targetPath;
        return result;
    }

    // Manual mode: user selects files
    if (fileSelectCallback_) {
        // Get available files
        std::vector<std::string> files;
        if (fs::is_directory(targetPath)) {
            for (const auto& entry : fs::recursive_directory_iterator(targetPath)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
        } else {
            files.push_back(targetPath);
        }
        
        if (files.empty()) {
            result.errorMessage = "No files found in target path";
            return result;
        }
        
        // Let user select file
        std::string selectedFile = fileSelectCallback_(files);
        if (selectedFile.empty()) {
            result.errorMessage = "User cancelled file selection";
            return result;
        }
        
        result.filesProcessed.push_back(selectedFile);
        result.success = true;
    } else {
        // No callback, process target directly
        result.filesProcessed.push_back(targetPath);
        result.success = true;
    }
    
    return result;
}

void ManualWorkingMode::setFileSelectCallback(FileSelectCallback callback) {
    fileSelectCallback_ = callback;
}

void ManualWorkingMode::setAvailableFiles(const std::vector<std::string>& files) {
    availableFiles_ = files;
}

// ── SemiAutoWorkingMode ──

bool SemiAutoWorkingMode::initialize(const WorkingModeConfig& config) {
    config_ = config;
    return true;
}

WorkingModeResult SemiAutoWorkingMode::execute(const std::string& targetPath) {
    WorkingModeResult result;
    result.modeName = name();
    
    namespace fs = std::filesystem;
    
    if (!fs::exists(targetPath)) {
        result.errorMessage = "Target path does not exist: " + targetPath;
        return result;
    }

    // Semi-Auto mode: auto-detect files
    std::vector<std::string> detectedFiles;
    
    if (fs::is_directory(targetPath)) {
        // Auto-detect relevant files
        for (const auto& entry : fs::recursive_directory_iterator(targetPath)) {
            if (!entry.is_regular_file()) continue;
            
            auto ext = entry.path().extension().string();
            
            // Detect relevant file types
            if (ext == ".so" || ext == ".dll" || ext == ".apk" || 
                ext == ".dat" || ext == ".bin" || ext == ".pak" ||
                ext == ".win" || ext == ".pck" || ext == ".assets") {
                detectedFiles.push_back(entry.path().string());
            }
        }
    } else {
        detectedFiles.push_back(targetPath);
    }

    if (detectedFiles.empty()) {
        result.errorMessage = "No relevant files detected";
        return result;
    }

    // User confirmation if callback provided
    if (userConfirmCallback_) {
        std::string message = "Found " + std::to_string(detectedFiles.size()) + " files. Process?";
        if (!userConfirmCallback_(message)) {
            result.errorMessage = "User cancelled operation";
            return result;
        }
    }

    result.filesProcessed = detectedFiles;
    result.success = true;
    
    return result;
}

void SemiAutoWorkingMode::setUserConfirmCallback(UserConfirmCallback callback) {
    userConfirmCallback_ = callback;
}

// ── FullAutoWorkingMode ──

bool FullAutoWorkingMode::initialize(const WorkingModeConfig& config) {
    config_ = config;
    return true;
}

WorkingModeResult FullAutoWorkingMode::execute(const std::string& targetPath) {
    WorkingModeResult result;
    result.modeName = name();
    
    namespace fs = std::filesystem;
    
    if (!fs::exists(targetPath)) {
        result.errorMessage = "Target path does not exist: " + targetPath;
        return result;
    }

    // Full Auto mode: process all files automatically
    std::vector<std::string> allFiles;
    
    if (fs::is_directory(targetPath)) {
        for (const auto& entry : fs::recursive_directory_iterator(targetPath)) {
            if (entry.is_regular_file()) {
                allFiles.push_back(entry.path().string());
            }
        }
    } else {
        allFiles.push_back(targetPath);
    }

    if (allFiles.empty()) {
        result.errorMessage = "No files found in target path";
        return result;
    }

    result.filesProcessed = allFiles;
    result.success = true;
    
    return result;
}

// ── WorkingModeFactory ──

std::unique_ptr<IWorkingMode> WorkingModeFactory::create(WorkingModeType type) {
    switch (type) {
        case WorkingModeType::Manual:
            return std::make_unique<ManualWorkingMode>();
        case WorkingModeType::SemiAuto:
            return std::make_unique<SemiAutoWorkingMode>();
        case WorkingModeType::FullAuto:
            return std::make_unique<FullAutoWorkingMode>();
    }
    return nullptr;
}

std::unique_ptr<IWorkingMode> WorkingModeFactory::create(const WorkingModeConfig& config) {
    auto mode = create(config.type);
    if (mode) {
        mode->initialize(config);
    }
    return mode;
}

std::vector<WorkingModeType> WorkingModeFactory::availableModes() {
    return {
        WorkingModeType::Manual,
        WorkingModeType::SemiAuto,
        WorkingModeType::FullAuto
    };
}

std::string WorkingModeFactory::modeDescription(WorkingModeType type) {
    switch (type) {
        case WorkingModeType::Manual:
            return "User manually selects files and targets";
        case WorkingModeType::SemiAuto:
            return "Auto-detect with user confirmation";
        case WorkingModeType::FullAuto:
            return "Fully automatic operation (may require root)";
    }
    return "Unknown mode";
}

} // namespace omnibyte::dumper
