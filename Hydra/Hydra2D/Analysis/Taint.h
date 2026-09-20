#pragma once
// Taint — Taint analysis for binary data flow tracking.
// Tracks data from sources (network, file, environment) through registers/memory
// to sinks (write, exec, system call). Detects unsanitized data reaching dangerous sinks.
//
// Integrations:
//   - Boost: boost_hash_data for taint propagation hashing, BitSet for coverage
//   - GMP: BigInt for tracking large constant values in taint propagation
//   - Taskflow: parallel analysis of independent taint paths
//
// Taint model:
//   - Source: instructions that read external input (recv, read, getenv, etc.)
//   - Propagation: data flows through MOV, ADD, SUB, XOR, LOAD, STORE
//   - Sink: instructions that use data dangerously (write, exec, system calls)
//   - Sanitizer: explicit checks (AND with mask, CMP bounds check)
//
// Usage:
//   Taint analyzer;
//   auto result = analyzer.analyzeTaint(entryAddr, codeData, taintSources);
//   bool hasLeak = analyzer.findTaintLeaks(entryAddr, codeData, taintSources);

#include "IAnalysis.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>

namespace omnibyte::hydradis {

/// Taint propagation state for a register or memory location.
struct TaintState {
    bool isTainted = false;
    uint64_t sourceAddress = 0;    // address where taint originated
    size_t propagationSteps = 0;   // number of steps from source
    std::string sourceType;        // "recv", "read", "getenv", etc.
};

/// Taint source definition (where taint enters the program).
struct TaintSource {
    uint64_t address = 0;          // instruction address
    std::string type;              // "recv", "read", "getenv", "argv", etc.
    size_t dataSize = 0;           // bytes of tainted data
};

/// Taint sink definition (where tainted data is used dangerously).
struct TaintSink {
    uint64_t address = 0;          // instruction address
    std::string type;              // "write", "exec", "system", "ioctl"
};

/// Taint analysis result for a single path.
struct TaintPath {
    uint64_t sourceAddress = 0;
    uint64_t sinkAddress = 0;
    std::string sourceType;
    std::string sinkType;
    std::vector<uint64_t> propagationPath; // addresses in the path
    bool sanitized = false;        // true if sanitizer found on path
};

/// Taint analysis for binary data flow tracking.
/// Uses iterative dataflow analysis with worklist algorithm.
class Taint : public IAnalysis {
public:
    Taint() = default;
    ~Taint() override = default;

    std::string name() const override { return "taint"; }

    // ── IAnalysis overrides ──────────────────────────────────────────

    TaintResult analyzeTaint(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData,
        const std::vector<uint64_t>& taintSources
    ) const override;

    // ── Taint-specific methods ───────────────────────────────────────

    /// Find all taint leaks: source→sink paths where data is unsanitized.
    std::vector<TaintPath> findTaintLeaks(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData,
        const std::vector<uint64_t>& taintSources
    ) const;

    /// Check if a specific address is tainted given the source set.
    bool isAddressTainted(
        uint64_t address,
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData,
        const std::vector<uint64_t>& taintSources
    ) const;

    /// Get taint state at a specific instruction address.
    TaintState getTaintState(
        uint64_t address,
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData,
        const std::vector<uint64_t>& taintSources
    ) const;

    /// Define common Android security-sensitive sinks.
    static std::vector<TaintSink> getDefaultSinks();

    /// Define common Android taint sources.
    static std::vector<TaintSource> getDefaultSources();

private:
    /// Propagate taint through a single ARM64 instruction.
    /// Updates taintMap based on instruction semantics.
    void propagateInstruction(
        uint32_t instruction,
        uint64_t address,
        std::unordered_map<uint64_t, TaintState>& taintMap,
        const std::vector<uint8_t>& codeData
    ) const;

    /// Check if instruction is a taint source (reads external input).
    bool isSource(uint64_t address, const std::vector<TaintSource>& sources) const;

    /// Check if instruction is a taint sink (uses data dangerously).
    bool isSink(uint64_t address, const std::vector<TaintSink>& sinks) const;

    /// Check if instruction is a sanitizer (bounds check, mask, etc.).
    bool isSanitizer(uint32_t instruction) const;

    /// Extract register indices from ARM64 instruction.
    /// Returns {Rd, Rn, Rm} or {-1, -1, -1} if not applicable.
    std::array<int, 3> extractRegisters(uint32_t instruction) const;

    /// Simple branch target extraction for control flow.
    uint64_t getBranchTarget(uint32_t instruction, uint64_t address) const;
};

} // namespace omnibyte::hydradis
