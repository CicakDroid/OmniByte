// Stack.cpp — Implementation of stack/expression analysis algorithms.
// §54: Constant folding, algebraic simplification, common subexpression elimination.

#include "Stack.h"

#include <common/GMP/GMP.h>
#include <common/Taskflow/TaskflowAdapter.h>

#include <cstring>
#include <unordered_map>
#include <functional>

namespace omnibyte::hydradis {

// ── IAnalysis interface ─────────────────────────────────────────────────

StackResult Stack::constantFold(
    const std::vector<uint8_t>& expressionBytes
) const {
    StackResult result;
    if (expressionBytes.empty()) {
        result.success = true;
        return result;
    }

    int64_t folded = 0;
    if (evaluateConstantExpression(expressionBytes.data(), expressionBytes.size(), folded)) {
        result.success = true;
        result.simplifiedValue = folded;
        result.nodesSimplified = expressionBytes.size() / 9; // each op = 9 bytes (8 operand + 1 opcode)
    } else {
        result.success = true; // not an error, just not foldable
        result.simplifiedValue = 0;
    }

    return result;
}

StackResult Stack::algebraicSimplify(
    const std::vector<uint8_t>& expressionBytes
) const {
    StackResult result;
    if (expressionBytes.empty()) {
        result.success = true;
        return result;
    }

    auto simplified = simplifyAlgebraicIdentities(
        expressionBytes.data(), expressionBytes.size()
    );

    result.success = true;
    result.nodesSimplified = (expressionBytes.size() - simplified.size()) / 9;
    result.simplifiedValue = 0;

    // If fully collapsed to a single 8-byte constant
    if (simplified.size() == 8) {
        int64_t val = 0;
        std::memcpy(&val, simplified.data(), 8);
        result.simplifiedValue = val;
    }

    return result;
}

// ── Stack-specific methods ──────────────────────────────────────────────

size_t Stack::eliminateCommonSubexpressions(
    const std::vector<uint8_t>& expressionBytes
) const {
    if (expressionBytes.size() < 9) return 0;

    // Hash each subexpression (opcode + operand) and count duplicates
    std::unordered_map<uint64_t, size_t> seen;
    size_t duplicates = 0;

    size_t offset = 0;
    while (offset + 9 <= expressionBytes.size()) {
        // Hash the 9-byte chunk (1 opcode + 8 operand)
        uint64_t hash = 0;
        for (size_t i = 0; i < 9 && offset + i < expressionBytes.size(); ++i) {
            hash = hash * 31 + expressionBytes[offset + i];
        }

        auto it = seen.find(hash);
        if (it != seen.end()) {
            ++duplicates;
        } else {
            seen[hash] = duplicates;
        }

        offset += 9;
    }

    return duplicates;
}

bool Stack::evaluateConstantExpression(
    const uint8_t* data, size_t dataSize,
    int64_t& outResult
) const {
    if (dataSize < 8) return false;

    // Read first operand
    int64_t accumulator = 0;
    std::memcpy(&accumulator, data, 8);

    size_t offset = 8;

    // Process opcode + operand pairs
    while (offset + 9 <= dataSize) {
        uint8_t opcode = data[offset];
        int64_t operand = 0;
        std::memcpy(&operand, data + offset + 1, 8);

        switch (opcode) {
            case 0x01: accumulator += operand; break;  // ADD
            case 0x02: accumulator -= operand; break;  // SUB
            case 0x03: accumulator *= operand; break;  // MUL
            case 0x04:
                if (operand == 0) return false; // division by zero
                accumulator /= operand;
                break;                              // DIV
            case 0x05: accumulator &= operand; break;  // AND
            case 0x06: accumulator |= operand; break;  // OR
            case 0x07: accumulator ^= operand; break;  // XOR
            case 0x08:
                if (operand == 0) return false; // modulo by zero
                accumulator %= operand;
                break;                              // MOD
            default: return false; // unknown opcode
        }

        offset += 9;
    }

    outResult = accumulator;
    return true;
}

std::vector<uint8_t> Stack::simplifyAlgebraicIdentities(
    const uint8_t* data, size_t dataSize
) const {
    std::vector<uint8_t> result;
    if (dataSize < 8) return result;

    // Copy first operand
    result.assign(data, data + 8);
    int64_t accumulator = 0;
    std::memcpy(&accumulator, data, 8);

    size_t offset = 8;
    while (offset + 9 <= dataSize) {
        uint8_t opcode = data[offset];
        int64_t operand = 0;
        std::memcpy(&operand, data + offset + 1, 8);

        bool skip = false;

        switch (opcode) {
            case 0x01: // ADD: x + 0 → x
                if (operand == 0) skip = true;
                else accumulator += operand;
                break;
            case 0x02: // SUB: x - 0 → x
                if (operand == 0) skip = true;
                else accumulator -= operand;
                break;
            case 0x03: // MUL: x * 0 → 0, x * 1 → x
                if (operand == 0) { accumulator = 0; skip = true; }
                else if (operand == 1) skip = true;
                else accumulator *= operand;
                break;
            case 0x04: // DIV: x / 1 → x
                if (operand == 1) skip = true;
                else if (operand != 0) accumulator /= operand;
                break;
            case 0x05: // AND: x & 0 → 0
                if (operand == 0) { accumulator = 0; skip = true; }
                else if (operand == -1) skip = true; // x & 0xFF..FF → x
                else accumulator &= operand;
                break;
            case 0x06: // OR: x | 0 → x
                if (operand == 0) skip = true;
                else if (operand == -1) { accumulator = -1; skip = true; }
                else accumulator |= operand;
                break;
            case 0x07: // XOR: x ^ 0 → x, x ^ x → 0
                if (operand == 0) skip = true;
                else if (accumulator == operand) { accumulator = 0; skip = true; }
                else accumulator ^= operand;
                break;
            case 0x08: // MOD: x % 1 → 0
                if (operand == 1) { accumulator = 0; skip = true; }
                else if (operand != 0) accumulator %= operand;
                break;
            default:
                // Unknown opcode: copy as-is
                result.insert(result.end(), data + offset, data + offset + 9);
                offset += 9;
                continue;
        }

        if (skip) {
            // Operation eliminated, don't append to result
        } else {
            // Append the folded operation
            result.resize(result.size() + 9);
            std::memcpy(result.data() + result.size() - 9, data + offset, 9);
        }

        // Update accumulator in result
        std::memcpy(result.data(), &accumulator, 8);

        offset += 9;
    }

    return result;
}

// ── Private helpers ─────────────────────────────────────────────────────

size_t Stack::decodeOperation(
    const uint8_t* data, size_t dataSize,
    uint8_t& outOpcode, int64_t& outOperand
) const {
    if (dataSize < 9) return 0;

    outOpcode = data[0];
    std::memcpy(&outOperand, data + 1, 8);

    return 9;
}

} // namespace omnibyte::hydradis
