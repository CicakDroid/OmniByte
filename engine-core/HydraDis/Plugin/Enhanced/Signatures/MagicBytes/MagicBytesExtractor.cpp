// MagicBytesExtractor — Signature maker implementation.

#include "MagicBytesExtractor.h"
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace omnibyte::signatures {

struct MagicBytesExtractor::Impl {
    // No state needed for now
};

MagicBytesExtractor::MagicBytesExtractor() : impl_(std::make_unique<Impl>()) {}

MagicBytesExtractor::~MagicBytesExtractor() = default;

MagicBytesExtractor::MagicBytesExtractor(MagicBytesExtractor&&) noexcept = default;
MagicBytesExtractor& MagicBytesExtractor::operator=(MagicBytesExtractor&&) noexcept = default;

ExtractionResult MagicBytesExtractor::extractFromFile(const std::string& filePath,
                                                    const SignatureCreateOptions& options) const {
    ExtractionResult result;
    
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        result.errorMessage = "Failed to open file: " + filePath;
        return result;
    }

    auto fileSize = file.tellg();
    if (fileSize <= 0) {
        result.errorMessage = "File is empty";
        return result;
    }

    // Read file header
    size_t readSize = std::min(static_cast<size_t>(fileSize), 
                              options.magicOffset + options.magicLength + 256);
    std::vector<uint8_t> buffer(readSize);
    
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), readSize);
    
    return extractFromMemory(buffer.data(), buffer.size(), options);
}

ExtractionResult MagicBytesExtractor::extractFromMemory(const uint8_t* data, size_t size,
                                                      const SignatureCreateOptions& options) const {
    ExtractionResult result;
    
    if (!data || size == 0) {
        result.errorMessage = "No data provided";
        return result;
    }

    if (options.magicOffset + options.magicLength > size) {
        result.errorMessage = "Not enough data for extraction";
        return result;
    }

    // Extract magic bytes
    result.magicBytes.assign(
        data + options.magicOffset,
        data + options.magicOffset + options.magicLength
    );
    result.magicOffset = options.magicOffset;
    result.magicLength = options.magicLength;
    result.success = true;

    // Detect type using MagicBytesDetector
    MagicBytesDetector detector;
    detector.loadBuiltinSignatures();
    
    auto detection = detector.detectMemory(data, size);
    if (detection.detected) {
        result.detectedType = detection.fileType;
        result.detectedName = detection.signatureName;
    }

    return result;
}

CreatedSignature MagicBytesExtractor::createSignature(const std::string& filePath,
                                                    const SignatureCreateOptions& options) const {
    CreatedSignature created;
    created.sourceFile = filePath;
    
    auto extraction = extractFromFile(filePath, options);
    if (!extraction.success) {
        return created;
    }

    created.rawMagicBytes = extraction.magicBytes;
    created.signature = createSignatureFromBytes(extraction.magicBytes, options);
    created.signature.name = options.name.empty() ? extraction.detectedName : options.name;
    
    return created;
}

FileSignature MagicBytesExtractor::createSignatureFromBytes(const std::vector<uint8_t>& magicBytes,
                                                          const SignatureCreateOptions& options) const {
    FileSignature sig;
    sig.name = options.name;
    sig.extension = options.extension;
    sig.type = options.type;
    
    MagicPattern pattern;
    pattern.bytes = magicBytes;
    pattern.offset = options.magicOffset;
    
    if (options.useWildcards && magicBytes.size() > 4) {
        // Simple wildcard detection: if byte appears multiple times, use wildcard
        // This is a basic heuristic - can be improved
        std::vector<uint8_t> byteCounts(256, 0);
        for (uint8_t b : magicBytes) {
            byteCounts[b]++;
        }
        
        pattern.mask.resize(magicBytes.size(), false);
        for (size_t i = 0; i < magicBytes.size(); ++i) {
            if (byteCounts[magicBytes[i]] > 2) {
                pattern.mask[i] = true;  // Mark as wildcard
            }
        }
    }
    
    sig.patterns.push_back(pattern);
    return sig;
}

std::vector<uint8_t> MagicBytesExtractor::findCommonMagic(const std::string& file1,
                                                        const std::string& file2,
                                                        size_t maxOffset) const {
    std::ifstream f1(file1, std::ios::binary | std::ios::ate);
    std::ifstream f2(file2, std::ios::binary | std::ios::ate);
    
    if (!f1.is_open() || !f2.is_open()) {
        return {};
    }

    auto size1 = f1.tellg();
    auto size2 = f2.tellg();
    
    if (size1 <= 0 || size2 <= 0) {
        return {};
    }

    size_t readSize = std::min({static_cast<size_t>(size1), 
                               static_cast<size_t>(size2), 
                               maxOffset});
    
    std::vector<uint8_t> buf1(readSize), buf2(readSize);
    
    f1.seekg(0);
    f1.read(reinterpret_cast<char*>(buf1.data()), readSize);
    
    f2.seekg(0);
    f2.read(reinterpret_cast<char*>(buf2.data()), readSize);
    
    // Find common prefix
    std::vector<uint8_t> common;
    for (size_t i = 0; i < readSize; ++i) {
        if (buf1[i] == buf2[i]) {
            common.push_back(buf1[i]);
        } else {
            break;
        }
    }
    
    return common;
}

ExtractionResult MagicBytesExtractor::analyzeFile(const std::string& filePath) const {
    SignatureCreateOptions options;
    options.magicOffset = 0;
    options.magicLength = 32;  // Extract more bytes for analysis
    
    return extractFromFile(filePath, options);
}

bool MagicBytesExtractor::exportSignature(const CreatedSignature& signature,
                                         const std::string& outputPath) const {
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        return false;
    }

    file << "# Signature: " << signature.signature.name << "\n";
    file << "# Extension: " << signature.signature.extension << "\n";
    file << "# Type: " << fileTypeToString(signature.signature.type) << "\n";
    file << "# Source: " << signature.sourceFile << "\n";
    
    if (!signature.notes.empty()) {
        file << "# Notes: " << signature.notes << "\n";
    }
    
    file << "# Magic bytes (hex): ";
    for (uint8_t b : signature.rawMagicBytes) {
        char hex[4];
        std::snprintf(hex, sizeof(hex), "%02X ", b);
        file << hex;
    }
    file << "\n\n";
    
    // Output as YARA rule format
    file << "rule " << signature.signature.name << " {\n";
    file << "  meta:\n";
    file << "    description = \"" << (signature.notes.empty() ? signature.signature.name : signature.notes) << "\"\n";
    file << "    extension = \"" << signature.signature.extension << "\"\n";
    file << "    file_type = \"" << fileTypeToString(signature.signature.type) << "\"\n";
    file << "\n";
    file << "  strings:\n";
    file << "    $magic = { ";
    for (size_t i = 0; i < signature.rawMagicBytes.size(); ++i) {
        char hex[4];
        std::snprintf(hex, sizeof(hex), "%02X", signature.rawMagicBytes[i]);
        file << hex;
        if (i + 1 < signature.rawMagicBytes.size()) file << " ";
    }
    file << " }\n";
    file << "\n";
    file << "  condition:\n";
    file << "    $magic at 0\n";
    file << "}\n";
    
    return true;
}

bool MagicBytesExtractor::exportSignatures(const std::vector<CreatedSignature>& signatures,
                                          const std::string& outputPath) const {
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        return false;
    }

    file << "// Auto-generated signatures by OmniByte Signature Maker\n";
    file << "// Generated: " << __DATE__ << " " << __TIME__ << "\n\n";
    
    for (const auto& sig : signatures) {
        file << "rule " << sig.signature.name << " {\n";
        file << "  meta:\n";
        file << "    description = \"" << sig.signature.name << "\"\n";
        file << "    extension = \"" << sig.signature.extension << "\"\n";
        file << "    file_type = \"" << fileTypeToString(sig.signature.type) << "\"\n";
        file << "\n";
        file << "  strings:\n";
        file << "    $magic = { ";
        for (size_t i = 0; i < sig.rawMagicBytes.size(); ++i) {
            char hex[4];
            std::snprintf(hex, sizeof(hex), "%02X", sig.rawMagicBytes[i]);
            file << hex;
            if (i + 1 < sig.rawMagicBytes.size()) file << " ";
        }
        file << " }\n";
        file << "\n";
        file << "  condition:\n";
        file << "    $magic at 0\n";
        file << "}\n\n";
    }
    
    return true;
}

} // namespace omnibyte::signatures
