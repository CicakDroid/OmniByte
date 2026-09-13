// MagicBytesDetector — Magic bytes detection implementation.
// Includes built-in signatures for common file formats.

#include "MagicBytesDetector.h"
#include <fstream>
#include <filesystem>
#include <cstring>

namespace omnibyte::signatures {

struct MagicBytesDetector::Impl {
    std::vector<FileSignature> signatures;
};

MagicBytesDetector::MagicBytesDetector() : impl_(std::make_unique<Impl>()) {}

MagicBytesDetector::~MagicBytesDetector() = default;

MagicBytesDetector::MagicBytesDetector(MagicBytesDetector&&) noexcept = default;
MagicBytesDetector& MagicBytesDetector::operator=(MagicBytesDetector&&) noexcept = default;

void MagicBytesDetector::loadBuiltinSignatures() {
    // ── Android ──
    {
        FileSignature sig;
        sig.name = "Android APK";
        sig.extension = ".apk";
        sig.type = FileType::Android;
        sig.description = "Android Application Package";
        
        MagicPattern pat;
        pat.bytes = {'P', 'K', 0x03, 0x04};
        sig.patterns.push_back(pat);
        
        sig.tags = {"android", "apk", "zip"};
        impl_->signatures.push_back(sig);
    }
    
    {
        FileSignature sig;
        sig.name = "Android DEX";
        sig.extension = ".dex";
        sig.type = FileType::Android;
        sig.description = "Dalvik Executable";
        
        MagicPattern pat;
        pat.bytes = {'d', 'e', 'x', '\n'};
        sig.patterns.push_back(pat);
        
        sig.tags = {"android", "dex", "dalvik"};
        impl_->signatures.push_back(sig);
    }
    
    {
        FileSignature sig;
        sig.name = "Android OAT";
        sig.extension = ".oat";
        sig.type = FileType::Android;
        sig.description = "Android OAT (ahead-of-time compiled)";
        
        MagicPattern pat;
        pat.bytes = {'o', 'a', 't', '\n'};
        sig.patterns.push_back(pat);
        
        sig.tags = {"android", "oat", "art"};
        impl_->signatures.push_back(sig);
    }
    
    {
        FileSignature sig;
        sig.name = "Android ART";
        sig.extension = ".art";
        sig.type = FileType::Android;
        sig.description = "Android ART image";
        
        MagicPattern pat;
        pat.bytes = {'a', 'r', 't', '\n'};
        sig.patterns.push_back(pat);
        
        sig.tags = {"android", "art"};
        impl_->signatures.push_back(sig);
    }
    
    // ── Unity ──
    {
        FileSignature sig;
        sig.name = "Unity AssetBundle";
        sig.extension = ".unity3d";
        sig.type = FileType::GameEngine;
        sig.description = "Unity AssetBundle format";
        
        MagicPattern pat;
        pat.bytes = {'U', 'n', 'i', 't', 'y'};
        sig.patterns.push_back(pat);
        
        sig.tags = {"unity", "assetbundle", "game"};
        impl_->signatures.push_back(sig);
    }
    
    {
        FileSignature sig;
        sig.name = "Unity Serialized File";
        sig.extension = ".assets";
        sig.type = FileType::GameEngine;
        sig.description = "Unity serialized file";
        
        MagicPattern pat;
        pat.bytes = {'U', 'n', 'i', 't', 'y', 'F', 'S'};
        sig.patterns.push_back(pat);
        
        sig.tags = {"unity", "serialized", "game"};
        impl_->signatures.push_back(sig);
    }
    
    // ── Unreal Engine ──
    {
        FileSignature sig;
        sig.name = "Unreal Pak";
        sig.extension = ".pak";
        sig.type = FileType::GameEngine;
        sig.description = "Unreal Engine PAK file";
        
        MagicPattern pat;
        pat.bytes = {0xE1, 0x12, 0x6F, 0x5A};  // 0x5A6F12E1 little-endian
        sig.patterns.push_back(pat);
        
        sig.tags = {"unreal", "pak", "game"};
        impl_->signatures.push_back(sig);
    }
    
    // ── Godot ──
    {
        FileSignature sig;
        sig.name = "Godot PCK";
        sig.extension = ".pck";
        sig.type = FileType::GameEngine;
        sig.description = "Godot Engine PCK package";
        
        MagicPattern pat;
        pat.bytes = {'G', 'D', 'P', 'C'};
        sig.patterns.push_back(pat);
        
        sig.tags = {"godot", "pck", "game"};
        impl_->signatures.push_back(sig);
    }
    
    // ── Cocos2d ──
    {
        FileSignature sig;
        sig.name = "Cocos2d-x Lua Bytecode";
        sig.extension = ".luac";
        sig.type = FileType::GameEngine;
        sig.description = "Cocos2d-x compiled Lua";
        
        MagicPattern pat;
        pat.bytes = {0x1B, 0x4C, 0x75, 0x61};  // Lua bytecode header
        sig.patterns.push_back(pat);
        
        sig.tags = {"cocos2d", "lua", "game"};
        impl_->signatures.push_back(sig);
    }
    
    // ── GameMaker ──
    {
        FileSignature sig;
        sig.name = "GameMaker Data Win";
        sig.extension = ".win";
        sig.type = FileType::GameEngine;
        sig.description = "GameMaker Studio data.win";
        
        MagicPattern pat;
        pat.bytes = {'F', 'O', 'R', 'M'};
        sig.patterns.push_back(pat);
        
        sig.tags = {"gamemaker", "data", "game"};
        impl_->signatures.push_back(sig);
    }
    
    // ── Source Engine ──
    {
        FileSignature sig;
        sig.name = "Source VPK";
        sig.extension = ".vpk";
        sig.type = FileType::GameEngine;
        sig.description = "Valve Pak format";
        
        MagicPattern pat;
        pat.bytes = {0x34, 0x12, 0xAA, 0x55};  // VPK signature
        sig.patterns.push_back(pat);
        
        sig.tags = {"source", "vpk", "game"};
        impl_->signatures.push_back(sig);
    }
    
    // ── Executables ──
    {
        FileSignature sig;
        sig.name = "ELF";
        sig.extension = ".so";
        sig.type = FileType::Executable;
        sig.description = "Executable and Linkable Format";
        
        MagicPattern pat;
        pat.bytes = {0x7F, 'E', 'L', 'F'};
        sig.patterns.push_back(pat);
        
        sig.tags = {"elf", "executable", "linux"};
        impl_->signatures.push_back(sig);
    }
    
    {
        FileSignature sig;
        sig.name = "PE";
        sig.extension = ".exe";
        sig.type = FileType::Executable;
        sig.description = "Portable Executable";
        
        MagicPattern pat;
        pat.bytes = {'M', 'Z'};
        sig.patterns.push_back(pat);
        
        sig.tags = {"pe", "executable", "windows"};
        impl_->signatures.push_back(sig);
    }
    
    // ── Archives ──
    {
        FileSignature sig;
        sig.name = "ZIP";
        sig.extension = ".zip";
        sig.type = FileType::Archive;
        sig.description = "ZIP archive";
        
        MagicPattern pat;
        pat.bytes = {0x50, 0x4B, 0x03, 0x04};
        sig.patterns.push_back(pat);
        
        sig.tags = {"zip", "archive"};
        impl_->signatures.push_back(sig);
    }
    
    {
        FileSignature sig;
        sig.name = "RAR";
        sig.extension = ".rar";
        sig.type = FileType::Archive;
        sig.description = "RAR archive";
        
        MagicPattern pat;
        pat.bytes = {0x52, 0x61, 0x72, 0x21};
        sig.patterns.push_back(pat);
        
        sig.tags = {"rar", "archive"};
        impl_->signatures.push_back(sig);
    }
    
    {
        FileSignature sig;
        sig.name = "7Z";
        sig.extension = ".7z";
        sig.type = FileType::Archive;
        sig.description = "7-Zip archive";
        
        MagicPattern pat;
        pat.bytes = {0x37, 0x7A, 0xBC, 0xAF};
        sig.patterns.push_back(pat);
        
        sig.tags = {"7z", "archive"};
        impl_->signatures.push_back(sig);
    }
    
    // ── Database ──
    {
        FileSignature sig;
        sig.name = "SQLite";
        sig.extension = ".db";
        sig.type = FileType::Database;
        sig.description = "SQLite database";
        
        MagicPattern pat;
        pat.bytes = {'S', 'Q', 'L', 'i', 't', 'e', ' ', 'f', 'o', 'r', 'm', 'a', 't'};
        sig.patterns.push_back(pat);
        
        sig.tags = {"sqlite", "database"};
        impl_->signatures.push_back(sig);
    }
}

bool MagicBytesDetector::loadSignatures(const std::string& filePath) {
    // TODO: Implement signature file loading (JSON/YAML format)
    return false;
}

bool MagicBytesDetector::loadSignaturesFromDirectory(const std::string& dirPath) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        return false;
    }

    bool allSuccess = true;
    for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            if (!loadSignatures(entry.path().string())) {
                allSuccess = false;
            }
        }
    }

    return allSuccess;
}

void MagicBytesDetector::addSignature(const FileSignature& signature) {
    impl_->signatures.push_back(signature);
}

void MagicBytesDetector::removeSignature(const std::string& name) {
    impl_->signatures.erase(
        std::remove_if(impl_->signatures.begin(), impl_->signatures.end(),
                       [&name](const FileSignature& s) { return s.name == name; }),
        impl_->signatures.end());
}

std::vector<FileSignature> MagicBytesDetector::signatures() const {
    return impl_->signatures;
}

DetectionResult MagicBytesDetector::detectFile(const std::string& filePath) const {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }

    auto fileSize = file.tellg();
    if (fileSize <= 0) {
        return {};
    }

    // Read first 256 bytes for magic detection
    size_t readSize = std::min(static_cast<size_t>(fileSize), size_t(256));
    std::vector<uint8_t> buffer(readSize);
    
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), readSize);
    
    return detectMemory(buffer.data(), buffer.size());
}

DetectionResult MagicBytesDetector::detectMemory(const uint8_t* data, size_t size) const {
    DetectionResult result;
    
    if (!data || size == 0) {
        return result;
    }

    float bestConfidence = 0.0f;

    for (const auto& sig : impl_->signatures) {
        for (const auto& pattern : sig.patterns) {
            // Check if we have enough data
            if (pattern.offset + pattern.bytes.size() > size) {
                continue;
            }

            // Compare bytes
            bool match = true;
            for (size_t i = 0; i < pattern.bytes.size(); ++i) {
                if (i < pattern.mask.size() && pattern.mask[i]) {
                    continue;  // Wildcard
                }
                if (data[pattern.offset + i] != pattern.bytes[i]) {
                    match = false;
                    break;
                }
            }

            if (match) {
                float confidence = 0.9f;  // Default high confidence
                
                if (confidence > bestConfidence) {
                    bestConfidence = confidence;
                    result.detected = true;
                    result.signatureName = sig.name;
                    result.fileType = sig.type;
                    result.confidence = confidence;
                    result.matchOffset = pattern.offset;
                    result.matchedExtension = sig.extension;
                }
            }
        }
    }

    return result;
}

DetectionResult MagicBytesDetector::detectBytes(const std::vector<uint8_t>& data) const {
    return detectMemory(data.data(), data.size());
}

bool MagicBytesDetector::matchesSignature(const uint8_t* data, size_t size,
                                         const std::string& signatureName) const {
    for (const auto& sig : impl_->signatures) {
        if (sig.name == signatureName) {
            for (const auto& pattern : sig.patterns) {
                if (pattern.offset + pattern.bytes.size() > size) {
                    continue;
                }

                bool match = true;
                for (size_t i = 0; i < pattern.bytes.size(); ++i) {
                    if (i < pattern.mask.size() && pattern.mask[i]) {
                        continue;
                    }
                    if (data[pattern.offset + i] != pattern.bytes[i]) {
                        match = false;
                        break;
                    }
                }

                if (match) return true;
            }
        }
    }

    return false;
}

std::vector<FileSignature> MagicBytesDetector::signaturesByType(FileType type) const {
    std::vector<FileSignature> result;
    for (const auto& sig : impl_->signatures) {
        if (sig.type == type) {
            result.push_back(sig);
        }
    }
    return result;
}

size_t MagicBytesDetector::signatureCount() const {
    return impl_->signatures.size();
}

} // namespace omnibyte::signatures
