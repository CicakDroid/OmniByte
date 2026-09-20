#pragma once
// IAnalysis.h — Abstract interface for binary analysis algorithms.
// Maps to research doc §54 (Algoritma), §55 (Aritmatika), §56 (Struktur Data).
//
// Design principles:
//   - Format-agnostic: operates on raw binary data + parsed metadata
//   - name() is the only pure virtual — subclasses override what they need
//   - Integrates Boost (hash/CRC/pattern), GMP (big integer), Taskflow (parallel)
//
// Update: added PackersResult, ObfuscateResult, TaintResult for extended analysis.
//         Default stubs return empty results so subclasses only implement what fits.

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

// ── Result types ────────────────────────────────────────────────────────

/// Results from structure analysis (§56: Trie, BloomFilter, Union-Find).
struct StructureResult {
    bool success = false;
    std::string errorMessage;
    std::vector<uint8_t> bytePattern;
    uint64_t patternOffset = 0;
    size_t matchCount = 0;
};

/// Results from stack analysis (§54: expression simplification, constant folding).
struct StackResult {
    bool success = false;
    std::string errorMessage;
    int64_t simplifiedValue = 0;
    size_t expressionDepth = 0;
    size_t nodesSimplified = 0;
};

/// Results from list/traversal analysis (§54: DFS/BFS, SCC, liveness).
struct ListResult {
    bool success = false;
    std::string errorMessage;
    std::vector<uint64_t> visitedAddresses;
    size_t componentCount = 0;
    std::vector<std::vector<uint64_t>> components;
};

/// Results from tree/graph analysis (§54: dominator tree, alias analysis).
struct TreeResult {
    bool success = false;
    std::string errorMessage;
    std::vector<uint64_t> dominatorTree;
    size_t loopCount = 0;
    std::vector<std::vector<uint64_t>> loops;
};

/// Results from packer/compiler detection.
struct PackersResult {
    bool success = false;
    std::string errorMessage;
    std::string packerName;           // detected packer (UPX, ASPack, Themida, etc.)
    std::string compilerName;         // detected compiler (GCC, Clang, MSVC, etc.)
    double entropy = 0.0;            // section entropy (high = packed/encrypted)
    bool isPacked = false;
    std::vector<std::string> indicators; // detection evidence strings
};

/// Results from obfuscation analysis.
struct ObfuscateResult {
    bool success = false;
    std::string errorMessage;
    bool controlFlowFlattened = false;
    bool opaquePredicatesFound = false;
    bool junkCodeDetected = false;
    bool stringEncryptionDetected = false;
    size_t suspiciousPatternCount = 0;
    std::vector<uint64_t> suspiciousOffsets; // offsets of obfuscation artifacts
};

/// Results from taint analysis.
struct TaintResult {
    bool success = false;
    std::string errorMessage;
    size_t sourcesFound = 0;          // taint source count (recv, read, env)
    size_t sinksFound = 0;            // taint sink count (write, exec, system)
    size_t propagationPaths = 0;      // source→sink paths
    std::vector<uint64_t> taintedAddresses; // addresses carrying tainted data
};

/// Results from function naming analysis.
struct FunctionsResult {
    bool success = false;
    std::string errorMessage;
    std::vector<uint64_t> functionAddresses; // discovered function addresses
    size_t namedFunctions = 0;              // functions with assigned names
    std::unordered_map<uint64_t, std::string> functionNames; // addr → suggested name
};

/// Results from variable naming analysis.
struct VariablesResult {
    bool success = false;
    std::string errorMessage;
    std::vector<uint64_t> variableAddresses; // detected variable stack offsets
    size_t namedVariables = 0;              // variables with assigned names
    std::unordered_map<uint64_t, std::string> variableNames; // offset → suggested name
};

/// Results from parameter naming analysis.
struct ParametersResult {
    bool success = false;
    std::string errorMessage;
    std::vector<uint64_t> parameterRegisters; // register indices used as params
    size_t namedParameters = 0;              // parameters with assigned names
    std::unordered_map<uint64_t, std::string> parameterNames; // register → suggested name
};

/// Results from type inference analysis.
struct TypesResult {
    bool success = false;
    std::string errorMessage;
    std::vector<uint64_t> typedAddresses;   // addresses with inferred types
    size_t inferredTypes = 0;               // number of types inferred
    std::unordered_map<uint64_t, std::string> typeMap; // addr → type name (e.g. "int", "void*")
};

/// Results from confidence scoring analysis.
struct ConfidencesResult {
    bool success = false;
    std::string errorMessage;
    double overallConfidence = 0.0;         // average confidence across all results
    std::unordered_map<std::string, double> categoryConfidence; // category → confidence
};

/// Results from string classification analysis.
struct StringsResult {
    bool success = false;
    std::string errorMessage;
    std::vector<uint64_t> stringAddresses;  // addresses of classified strings
    size_t classifiedStrings = 0;           // strings with assigned categories
    std::unordered_map<uint64_t, std::string> stringClassifications; // addr → category
};

// ── Abstract interface ──────────────────────────────────────────────────

/// Abstract interface for binary analysis algorithms.
/// name() is pure virtual; all other methods have default stubs.
/// Subclasses override only what fits their domain.
class IAnalysis {
public:
    virtual ~IAnalysis() = default;

    /// Algorithm name for logging/diagnostic.
    virtual std::string name() const = 0;

    // ── Structure analysis (§56) — default stubs ─────────────────────

    virtual StructureResult buildTrie(
        const std::vector<std::string>& /*strings*/
    ) const { return {}; }

    virtual StructureResult buildBloomFilter(
        const uint8_t* /*data*/, size_t /*dataSize*/,
        size_t /*expectedElements*/, double /*falsePositiveRate*/
    ) const { return {}; }

    virtual StructureResult computeEquivalenceClasses(
        const std::vector<std::pair<uint64_t, uint64_t>>& /*pairs*/
    ) const { return {}; }

    // ── Stack / expression analysis (§54) — default stubs ────────────

    virtual StackResult constantFold(
        const std::vector<uint8_t>& /*expressionBytes*/
    ) const { return {}; }

    virtual StackResult algebraicSimplify(
        const std::vector<uint8_t>& /*expressionBytes*/
    ) const { return {}; }

    // ── List / traversal analysis (§54) — default stubs ──────────────

    virtual ListResult dfsTraversal(
        uint64_t /*entryAddress*/,
        const std::vector<uint8_t>& /*codeData*/
    ) const { return {}; }

    virtual ListResult bfsTraversal(
        uint64_t /*entryAddress*/,
        const std::vector<uint8_t>& /*codeData*/
    ) const { return {}; }

    virtual ListResult findStronglyConnectedComponents(
        const std::vector<uint8_t>& /*codeData*/
    ) const { return {}; }

    // ── Tree / graph analysis (§54) — default stubs ──────────────────

    virtual TreeResult computeDominatorTree(
        uint64_t /*entryAddress*/,
        const std::vector<uint8_t>& /*codeData*/
    ) const { return {}; }

    virtual TreeResult detectLoops(
        uint64_t /*entryAddress*/,
        const std::vector<uint8_t>& /*codeData*/
    ) const { return {}; }

    // ── Packer/compiler analysis — default stubs ─────────────────────

    virtual PackersResult detectPacker(
        const uint8_t* /*data*/, size_t /*dataSize*/
    ) const { return {}; }

    virtual PackersResult detectCompiler(
        const uint8_t* /*data*/, size_t /*dataSize*/
    ) const { return {}; }

    // ── Obfuscation analysis — default stubs ─────────────────────────

    virtual ObfuscateResult analyzeObfuscation(
        const uint8_t* /*data*/, size_t /*dataSize*/
    ) const { return {}; }

    // ── Taint analysis — default stubs ───────────────────────────────

    virtual TaintResult analyzeTaint(
        uint64_t /*entryAddress*/,
        const std::vector<uint8_t>& /*codeData*/,
        const std::vector<uint64_t>& /*taintSources*/
    ) const { return {}; }

    // ── Function naming — default stubs ──────────────────────────────

    virtual FunctionsResult analyzeFunctions(
        uint64_t /*entryAddress*/,
        const std::vector<uint8_t>& /*codeData*/
    ) const { return {}; }

    // ── Variable naming — default stubs ──────────────────────────────

    virtual VariablesResult analyzeVariables(
        uint64_t /*entryAddress*/,
        const std::vector<uint8_t>& /*codeData*/
    ) const { return {}; }

    // ── Parameter naming — default stubs ─────────────────────────────

    virtual ParametersResult analyzeParameters(
        uint64_t /*entryAddress*/,
        const std::vector<uint8_t>& /*codeData*/
    ) const { return {}; }

    // ── Type inference — default stubs ───────────────────────────────

    virtual TypesResult analyzeTypes(
        uint64_t /*entryAddress*/,
        const std::vector<uint8_t>& /*codeData*/
    ) const { return {}; }

    // ── Confidence scoring — default stubs ───────────────────────────

    virtual ConfidencesResult analyzeConfidences(
        const std::vector<uint8_t>& /*codeData*/
    ) const { return {}; }

    // ── String classification — default stubs ────────────────────────

    virtual StringsResult analyzeStrings(
        const uint8_t* /*data*/, size_t /*dataSize*/
    ) const { return {}; }
};

} // namespace omnibyte::hydradis
