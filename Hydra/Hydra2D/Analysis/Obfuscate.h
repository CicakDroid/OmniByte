#pragma once
// Obfuscate — Binary obfuscation detection and analysis.
// Identifies common obfuscation techniques: control flow flattening,
// opaque predicates, junk code insertion, string encryption, and dead code.
//
// Integrations:
//   - Boost: boost_crc32 for pattern hashing, BitSet for code coverage
//   - GMP: BigInt for arithmetic-based opaque predicate evaluation
//   - Taskflow: parallel analysis of code blocks
//
// Detection methods (proven, not speculative):
//   - Control flow flattening: dispatch table + switch-case pattern
//   - Opaque predicates: always-true/false conditions (x*(x-1)%2==0)
//   - Junk code: NOP slides, unreachable instructions, dead registers
//   - String encryption: XOR/AES encoded strings with decoder stubs
//   - Dead code: unreachable basic blocks
//
// Usage:
//   Obfuscate analyzer;
//   auto result = analyzer.analyzeObfuscation(data, len);
//   bool cff = analyzer.detectControlFlowFlattening(data, len);
//   bool op = analyzer.detectOpaquePredicates(data, len);

#include "IAnalysis.h"
#include <string>
#include <vector>
#include <array>

namespace omnibyte::hydradis {

/// Obfuscation technique types.
enum class ObfuscationType {
    None,
    ControlFlowFlattening,  // switch-case dispatch on state variable
    OpaquePredicate,        // always-true/false conditional
    JunkCode,               // NOP slides, dead instructions
    StringEncryption,       // XOR/AES encoded strings
    DeadCode,               // unreachable basic blocks
    BogusControlFlow,       // fake branches with always-true conditions
    InstructionSubstitution // equivalent but complex instruction sequences
};

/// Detection result for a specific obfuscation technique.
struct ObfuscationIndicator {
    ObfuscationType type = ObfuscationType::None;
    std::string description;
    uint64_t offset = 0;       // offset in binary where detected
    size_t length = 0;         // length of affected region
    double confidence = 0.0;   // 0.0-1.0 detection confidence
};

/// Binary obfuscation detection and analysis.
class Obfuscate : public IAnalysis {
public:
    Obfuscate() = default;
    ~Obfuscate() override = default;

    std::string name() const override { return "obfuscate"; }

    // ── IAnalysis overrides ──────────────────────────────────────────

    ObfuscateResult analyzeObfuscation(
        const uint8_t* data, size_t dataSize
    ) const override;

    // ── Obfuscate-specific methods ───────────────────────────────────

    /// Detect control flow flattening pattern.
    /// Pattern: large switch on state variable, each case sets next state.
    bool detectControlFlowFlattening(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Detect opaque predicates.
    /// Pattern: conditional branch where condition is always true/false.
    /// Examples: x*(x-1)%2==0 (always true), x*x<0 (always false on signed).
    bool detectOpaquePredicates(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Detect junk/dead code insertion.
    /// Pattern: NOP slides, unreachable instructions after unconditional branch.
    bool detectJunkCode(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Detect string encryption stubs.
    /// Pattern: XOR loop over byte array, constant key, write-back loop.
    bool detectStringEncryption(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Get all obfuscation indicators with confidence scores.
    std::vector<ObfuscationIndicator> getAllIndicators(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Compute obfuscation density: ratio of obfuscated to total code.
    /// Returns value between 0.0 (clean) and 1.0 (fully obfuscated).
    double obfuscationDensity(
        const uint8_t* data, size_t dataSize
    ) const;

private:
    /// Count switch-case dispatch patterns (CMP + conditional branch chains).
    size_t countSwitchPatterns(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Count NOP/UNDEF sleds (3+ consecutive NOP instructions).
    size_t countNOPSleds(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Find XOR decode loops (LDRB+EOR+STRB pattern).
    size_t countXORDecodeLoops(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Find unconditional branches followed by reachable code (dead code).
    size_t countDeadCodeBlocks(
        const uint8_t* data, size_t dataSize
    ) const;
};

} // namespace omnibyte::hydradis
