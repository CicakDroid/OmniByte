// Packers.cpp — Binary packer and compiler detection implementation.
// Detection based on proven heuristics: byte signatures, entropy, section names.

#include "Packers.h"

#include <common/Boost/Boost.h>
#include <common/Taskflow/TaskflowAdapter.h>

#include <cmath>
#include <cstring>
#include <algorithm>
#include <array>

namespace omnibyte::hydradis {

// ── IAnalysis overrides ─────────────────────────────────────────────────

PackersResult Packers::detectPacker(
    const uint8_t* data, size_t dataSize
) const {
    PackersResult result;
    if (!data || dataSize == 0) {
        result.success = true;
        return result;
    }

    // 1. Check entropy — packed sections have high entropy
    double entropy = calculateEntropy(data, dataSize);
    result.entropy = entropy;

    // 2. Match known packer signatures
    const auto& signatures = getSignatures();
    for (const auto& sig : signatures) {
        if (sig.pattern.size() > dataSize) continue;

        // Search for signature in binary
        for (size_t offset = 0; offset + sig.pattern.size() <= dataSize; ++offset) {
            if (matchPattern(data, dataSize, offset, sig.pattern)) {
                result.packerName = sig.name;
                result.isPacked = true;
                result.indicators.push_back(
                    "Signature '" + sig.name + "' at offset 0x" +
                    omnibyte::common::formatAddress(offset)
                );

                // Verify entropy if signature specifies minimum
                if (sig.minEntropy > 0.0 && entropy >= sig.minEntropy) {
                    result.indicators.push_back(
                        "Entropy " + std::to_string(entropy) +
                        " >= threshold " + std::to_string(sig.minEntropy)
                    );
                }
                break;
            }
        }
        if (result.isPacked) break;
    }

    // 3. Check for common packer section names in ELF strings
    static const char* packerSections[] = {
        "UPX0", "UPX1", "UPX2",       // UPX
        ".themida",                     // Themida
        ".vmp0", ".vmp1", ".vmp2",     // VMProtect
        ".adata",                       // Bangcle
        ".dex",                         // DEX stub
        ".ollvm",                       // Obfuscator-LLVM
        nullptr
    };

    for (const char** sec = packerSections; *sec; ++sec) {
        if (findString(data, dataSize, *sec) >= 0) {
            result.isPacked = true;
            result.packerName = *sec;
            result.indicators.push_back(
                std::string("Section name '") + *sec + "' found"
            );
            break;
        }
    }

    result.success = true;
    return result;
}

PackersResult Packers::detectCompiler(
    const uint8_t* data, size_t dataSize
) const {
    PackersResult result;
    if (!data || dataSize == 0) {
        result.success = true;
        return result;
    }

    const auto& compilers = getCompilerSignatures();
    for (const auto& comp : compilers) {
        int64_t offset = findString(data, dataSize, comp.pattern);
        if (offset >= 0) {
            result.compilerName = comp.name;
            result.indicators.push_back(
                "Compiler string '" + comp.pattern + "' at offset 0x" +
                omnibyte::common::formatAddress(static_cast<uint64_t>(offset))
            );
            break;
        }
    }

    result.success = true;
    return result;
}

// ── Packers-specific methods ────────────────────────────────────────────

std::vector<SectionInfo> Packers::analyzeSections(
    const uint8_t* data, size_t dataSize
) const {
    std::vector<SectionInfo> sections;
    if (!data || dataSize == 0) return sections;

    // For raw binary without ELF headers, split into 4KB chunks for analysis
    const size_t chunkSize = 4096;
    size_t numChunks = (dataSize + chunkSize - 1) / chunkSize;

    for (size_t i = 0; i < numChunks; ++i) {
        size_t offset = i * chunkSize;
        size_t len = std::min(chunkSize, dataSize - offset);

        SectionInfo sec;
        sec.name = "chunk_" + std::to_string(i);
        sec.virtualAddress = offset;
        sec.virtualSize = len;
        sec.rawSize = len;
        sec.entropy = calculateEntropy(data + offset, len);

        sections.push_back(sec);
    }

    return sections;
}

double Packers::calculateEntropy(
    const uint8_t* data, size_t dataSize
) const {
    if (!data || dataSize == 0) return 0.0;

    auto freq = computeByteFrequency(data, dataSize);
    double entropy = 0.0;
    double log2 = std::log(2.0);

    for (size_t count : freq) {
        if (count == 0) continue;
        double p = static_cast<double>(count) / static_cast<double>(dataSize);
        entropy -= p * (std::log(p) / log2);
    }

    return entropy;
}

bool Packers::isLikelyPacked(
    const uint8_t* data, size_t dataSize
) const {
    if (!data || dataSize == 0) return false;

    double entropy = calculateEntropy(data, dataSize);

    // Heuristic: packed binaries have high entropy (> 7.0)
    // and large code sections relative to data sections
    if (entropy > 7.0) return true;

    // Check for packer signatures
    const auto& signatures = getSignatures();
    for (const auto& sig : signatures) {
        if (sig.pattern.size() > dataSize) continue;
        for (size_t offset = 0; offset + sig.pattern.size() <= dataSize; ++offset) {
            if (matchPattern(data, dataSize, offset, sig.pattern)) {
                return true;
            }
        }
    }

    return false;
}

std::string Packers::findCompilerString(
    const uint8_t* data, size_t dataSize
) const {
    const auto& compilers = getCompilerSignatures();
    for (const auto& comp : compilers) {
        int64_t offset = findString(data, dataSize, comp.pattern);
        if (offset >= 0) return comp.name;
    }
    return "";
}

const std::vector<PackerSignature>& Packers::getSignatures() {
    static const std::vector<PackerSignature> signatures = {
        // UPX signatures
        {"UPX!", {0x55, 0x50, 0x58, 0x21}, 0, 7.0},
        {"UPX1", {0x55, 0x50, 0x58, 0x31}, 0, 7.0},

        // Themida/WinLicense
        {".themida", {0x2E, 0x74, 0x68, 0x65, 0x6D, 0x69, 0x64, 0x61}, 0, 7.5},

        // VMProtect
        {".vmp0", {0x2E, 0x76, 0x6D, 0x70, 0x30}, 0, 7.0},

        // Bangcle (Android)
        {".adata", {0x2E, 0x61, 0x64, 0x61, 0x74, 0x61}, 0, 6.5},

        // Obfuscator-LLVM
        {".ollvm", {0x2E, 0x6F, 0x6C, 0x6C, 0x76, 0x6D}, 0, 7.0},

        // DexGuard
        {"dexguard", {0x64, 0x65, 0x78, 0x67, 0x75, 0x61, 0x72, 0x64}, 0, 7.0},
    };
    return signatures;
}

const std::vector<CompilerSignature>& Packers::getCompilerSignatures() {
    static const std::vector<CompilerSignature> compilers = {
        {"GCC",       "GCC:"},
        {"GCC",       "gcc version"},
        {"Clang",     "clang version"},
        {"Clang",     "LLVM"},
        {"MSVC",      "Microsoft C/C++"},
        {"MSVC",      "Visual C++"},
        {"Intel",     "Intel(R) C++"},
        {"Android NDK", "Android NDK"},
        {"Android NDK", "android-ndk"},
    };
    return compilers;
}

// ── Private helpers ─────────────────────────────────────────────────────

bool Packers::matchPattern(
    const uint8_t* data, size_t dataSize,
    size_t offset,
    const std::vector<uint8_t>& pattern
) const {
    if (offset + pattern.size() > dataSize) return false;
    return std::memcmp(data + offset, pattern.data(), pattern.size()) == 0;
}

int64_t Packers::findString(
    const uint8_t* data, size_t dataSize,
    const std::string& target
) const {
    if (target.empty() || target.size() > dataSize) return -1;

    for (size_t i = 0; i + target.size() <= dataSize; ++i) {
        if (std::memcmp(data + i, target.data(), target.size()) == 0) {
            return static_cast<int64_t>(i);
        }
    }
    return -1;
}

std::array<size_t, 256> Packers::computeByteFrequency(
    const uint8_t* data, size_t dataSize
) const {
    std::array<size_t, 256> freq{};
    for (size_t i = 0; i < dataSize; ++i) {
        ++freq[data[i]];
    }
    return freq;
}

} // namespace omnibyte::hydradis
