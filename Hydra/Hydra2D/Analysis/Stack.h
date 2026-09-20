#pragma once
// Stack — Stack/expression analysis (§54: Expression Simplification).
// Implements constant folding, algebraic simplification using GMP BigInt
// for arbitrary-precision evaluation of constant expressions in binaries.
//
// Integrations:
//   - GMP: BigInt for constant folding with unlimited precision
//   - Taskflow: parallel expression evaluation
//
// Usage:
//   Stack analyzer;
//   auto result = analyzer.constantFold(exprBytes);
//   auto simp = analyzer.algebraicSimplify(exprBytes);

#include "IAnalysis.h"
#include <string>
#include <vector>

namespace omnibyte::hydradis {

/// Stack/expression analysis: constant folding, algebraic simplification.
/// Based on research doc §54 (Expression Simplification).
class Stack : public IAnalysis {
public:
    Stack() = default;
    ~Stack() override = default;

    std::string name() const override { return "stack"; }

    // ── IAnalysis interface ─────────────────────────────────────────

    StackResult constantFold(
        const std::vector<uint8_t>& expressionBytes
    ) const override;

    StackResult algebraicSimplify(
        const std::vector<uint8_t>& expressionBytes
    ) const override;

    // ── Stack-specific methods ──────────────────────────────────────

    /// Common subexpression elimination: detect duplicate expressions.
    /// Returns number of duplicate expressions found.
    size_t eliminateCommonSubexpressions(
        const std::vector<uint8_t>& expressionBytes
    ) const;

    /// Evaluate a constant arithmetic expression encoded as raw bytes.
    /// Interprets pairs of 8-byte values with 1-byte opcodes between them.
    /// Opcodes: 0x01=+, 0x02=-, 0x03=*, 0x04=/, 0x05=&, 0x06=|, 0x07=^, 0x08=%
    /// @return true if expression is fully constant and was folded.
    bool evaluateConstantExpression(
        const uint8_t* data, size_t dataSize,
        int64_t& outResult
    ) const;

    /// Detect algebraic identities: x*0→0, x+0→x, x&0→0, x|0→x, x^x→0, x&x→x.
    /// Returns simplified byte sequence.
    std::vector<uint8_t> simplifyAlgebraicIdentities(
        const uint8_t* data, size_t dataSize
    ) const;

private:
    /// Decode a single opcode + operand pair from byte stream.
    /// @return number of bytes consumed, 0 on error.
    size_t decodeOperation(
        const uint8_t* data, size_t dataSize,
        uint8_t& outOpcode, int64_t& outOperand
    ) const;
};

} // namespace omnibyte::hydradis
