#pragma once
// Packers — Binary packer and compiler detection/analysis.
// Identifies common Android packers (UPX, Themida, VMProtect, Bangcle)
// and compilers (GCC, Clang, MSVC) by signature, entropy, and structural heuristics.
//
// Integrations:
//   - Boost: boost_crc32 for section signature matching, boost_hash_data for hashing
//   - GMP: BigInt for entropy calculation with arbitrary precision
//   - Taskflow: parallel section scanning
//
// Detection methods (proven, not speculative):
//   - Byte-pattern signature matching against known packer stubs
//   - Section name matching (UPX0/UPX1, .themida, .vmp, etc.)
//   - Entropy analysis (packed sections have entropy > 7.5)
//   - Entry-point offset heuristic (packer stubs jump far from original EP)
//
// Usage:
//   Packers analyzer;
//   auto packResult = analyzer.detectPacker(data, len);
//   auto compResult = analyzer.detectCompiler(data, len);
//   auto sections = analyzer.analyzeSections(data, len);

#include "IAnalysis.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace omnibyte::hydradis {

/// Packer signature for pattern matching.
struct PackerSignature {
    std::string name;                    // packer name
    std::vector<uint8_t> pattern;        // byte signature
    size_t offset = 0;                   // offset within section
    double minEntropy = 0.0;            // minimum entropy to confirm
};

/// Binary section metadata for packer/compiler analysis.
struct SectionInfo {
    std::string name;
    uint64_t virtualAddress = 0;
    size_t virtualSize = 0;
    size_t rawSize = 0;
    double entropy = 0.0;
    bool isExecutable = false;
    bool isWritable = false;
};

/// Known compiler string patterns.
struct CompilerSignature {
    std::string name;                    // compiler name
    std::string pattern;                 // string pattern to search for
    uint64_t minVersion = 0;            // minimum version (0 = any)
};

/// Binary packer and compiler detection/analysis.
/// Uses proven heuristics: signatures, entropy, section names.
class Packers : public IAnalysis {
public:
    Packers() = default;
    ~Packers() override = default;

    std::string name() const override { return "packers"; }

    // ── IAnalysis overrides ──────────────────────────────────────────

    PackersResult detectPacker(
        const uint8_t* data, size_t dataSize
    ) const override;

    PackersResult detectCompiler(
        const uint8_t* data, size_t dataSize
    ) const override;

    // ── Packers-specific methods ─────────────────────────────────────

    /// Analyze all sections for entropy and structural properties.
    /// Requires ELF/PE section headers; works on raw section data.
    std::vector<SectionInfo> analyzeSections(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Calculate Shannon entropy of a data block.
    /// Returns value between 0.0 (uniform) and 8.0 (random/compressed).
    double calculateEntropy(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Check if binary is likely packed based on entropy + structure.
    /// Heuristic: high entropy in code sections + unusual EP offset = packed.
    bool isLikelyPacked(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Search for compiler identification strings in binary.
    /// Matches against known patterns: "GCC:", "clang version", "Microsoft C/C++".
    std::string findCompilerString(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Get all known packer signatures.
    static const std::vector<PackerSignature>& getSignatures();

    /// Get all known compiler signatures.
    static const std::vector<CompilerSignature>& getCompilerSignatures();

private:
    /// Match byte pattern at offset.
    bool matchPattern(
        const uint8_t* data, size_t dataSize,
        size_t offset,
        const std::vector<uint8_t>& pattern
    ) const;

    /// Search for null-terminated string in binary data.
    /// Returns offset if found, -1 if not found.
    int64_t findString(
        const uint8_t* data, size_t dataSize,
        const std::string& target
    ) const;

    /// Compute byte frequency distribution for entropy calculation.
    std::array<size_t, 256> computeByteFrequency(
        const uint8_t* data, size_t dataSize
    ) const;
};

} // namespace omnibyte::hydradis
