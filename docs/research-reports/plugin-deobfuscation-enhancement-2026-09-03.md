# Research Report: Binary Deobfuscation Enhancement

**Date**: 2026-09-03
**Plugin**: Deobfuscate (plugin_deobfuscate.cpp, hrtng/)
**Status**: FUNCTIONAL (XOR detection, symbol scan, API hash lookup, basic decryption)
**Enhancement Target**: Improve with advanced pattern matching and control flow unflattening

---

## 1. Current State

The Deobfuscate plugin has two implementations:
1. **plugin_deobfuscate.cpp** (functional): XOR key detection, symbol name scan
2. **hrtng/HrtngDeob.cpp** (functional): API hash lookup (DexKit), XOR/base64/rot13 decryption, crypto constant detection

### What's Already Implemented

| Feature | Status | Notes |
|---------|--------|-------|
| XOR key detection | DONE | Scans for XOR patterns in instructions |
| Symbol name scan | DONE | Checks for obfuscated function names |
| API hash lookup | DONE | Maps API hashes to function names |
| Basic XOR decrypt | DONE | Simple XOR with detected key |
| Base64 decode | DONE | Base64 string detection + decode |
| ROT13 decrypt | DONE | Simple substitution cipher |
| Crypto constant detection | DONE | Known crypto constants (AES S-box, etc.) |

### What Can Be Enhanced

1. **Control Flow Flattening Detection**: Identify switch-based obfuscation
2. **String Decryption**: Detect encrypted string patterns
3. **Opcode Substitution Detection**: Identify equivalent opcode replacements
4. **Dead Code Detection**: Identify no-op or dead code blocks
5. **Pattern-based Deobfuscation**: Match known obfuscator patterns

---

## 2. Research Sources

### Source 1: Control Flow Flattening Detection
- **Paper**: "Deobfuscation of Control Flow Flattening" (2020)
- **URL**: https://doi.org/10.1109/CURSOR51560.2020.9293788
- **Key Insight**: Detects switch-based flattening via CFG analysis
- **Algorithm**:
  1. Build CFG from disassembled instructions
  2. Identify "dispatcher" basic block (high fan-out)
  3. Identify "handler" blocks (switch cases)
  4. Reconstruct original control flow
- **Relevance**: MEDIUM — we don't have full CFG yet

### Source 2: String Encryption Detection
- **Paper**: "Automated Detection of Encrypted Strings in Malware" (2019)
- **URL**: https://doi.org/10.3390/app9142793
- **Key Insight**: Detects string encryption via memory access patterns
- **Algorithm**:
  1. Scan for loops with byte-level operations
  2. Identify memory load/store patterns
  3. Detect decryption loops (XOR, ADD, SUB per byte)
  4. Extract encrypted data and decrypt
- **Relevance**: HIGH — directly applicable

### Source 3: Opcode Substitution Detection
- **Paper**: "Semantics-Preserving Obfuscation Detection" (2018)
- **Key Insight**: Detects equivalent opcode replacements
- **Examples**:
  - `mov reg, 0` → `xor reg, reg`
  - `mov reg, imm` → `push imm; pop reg`
  - `add reg, 1` → `inc reg`
  - `sub reg, 1` → `dec reg`
- **Relevance**: HIGH — simple pattern matching

### Source 4: Dead Code Detection
- **Paper**: "Dead Code Elimination in Obfuscated Binaries" (2017)
- **Key Insight**: Detects unreachable code blocks
- **Algorithm**:
  1. Build CFG
  2. Compute reachability from entry
  3. Identify unreachable blocks
  4. Mark as dead code
- **Relevance**: MEDIUM — we don't have full CFG yet

### Source 5: Obfuscator Pattern Signatures
- **Tool**: `Detect It Easy` (DIE)
- **URL**: https://github.com/horsicq/Detect-It-Easy
- **Key Insight**: Known obfuscator signatures
- **Obfuscators**:
  - VMProtect: characteristic vtable patterns
  - Themida: VMProtect-like patterns
  - OLLVM (Obfuscator-LLVM): control flow flattening
  - Bat! Protec: custom obfuscation
- **Relevance**: MEDIUM — signature-based detection

---

## 3. Feasible Implementation Approach

### Enhancement 1: String Decryption Detection

Since `PluginContext` provides `disassemblyResults`, we can:

1. **Scan for decryption loops**:
   - Look for loops with byte-level operations (ldrb/strb)
   - Identify XOR/ADD/SUB per byte
   - Extract encrypted data pointer and key

2. **Detect string patterns**:
   - Look for `adr/ldr` followed by loops
   - Identify memory access patterns
   - Extract string addresses

3. **Decrypt strings**:
   - Apply detected decryption algorithm
   - Output decrypted strings

### Enhancement 2: Opcode Substitution Detection

Simple pattern matching:

```cpp
struct OpcodePattern {
    std::string obfuscated;   // "xor reg, reg"
    std::string canonical;    // "mov reg, #0"
    std::string description;  // "zero register"
};

std::vector<OpcodePattern> patterns = {
    {"xor reg, reg", "mov reg, #0", "zero register"},
    {"push imm; pop reg", "mov reg, imm", "load immediate"},
    {"add reg, 1", "inc reg", "increment"},
    {"sub reg, 1", "dec reg", "decrement"},
    {"mov reg, reg", "nop", "no-op"},
};
```

### Enhancement 3: Control Flow Flattening Detection

Without full CFG, we can detect simple patterns:
- High fan-out from single block (dispatcher)
- Many conditional branches to same region
- Repeated switch-case patterns

### What We CAN Do

- Detect string decryption patterns
- Identify opcode substitutions
- Enhance XOR detection with more patterns
- Detect simple control flow flattening

### What We CANNOT Do (without full CFG)

- Full control flow unflattening
- Dead code elimination
- Advanced pattern matching

---

## 4. Dependencies

| Dependency | Required | Notes |
|------------|----------|-------|
| Capstone (disassembler) | YES | Already available via IDisassembler |
| CFG construction | NO | Not yet implemented (would enhance) |
| Symbolic execution | NO | Not needed for pattern matching |

**Conclusion**: Deobfuscation enhancement is feasible with existing disassembly output. The result will be pattern-based detection and simple decryption, suitable for common obfuscation techniques.

---

## 5. Expected Output Format

```json
{
  "plugin": "Deobfuscate",
  "stringDecryption": [
    {
      "address": "0x1000",
      "encryptedData": "0x2000",
      "key": "0x41",
      "algorithm": "XOR",
      "decrypted": "Hello, World!"
    }
  ],
  "opcodeSubstitutions": [
    {
      "address": "0x3000",
      "original": "xor reg, reg",
      "canonical": "mov reg, #0"
    }
  ],
  "xorDetection": [
    {
      "address": "0x4000",
      "key": "0x42",
      "pattern": "single-byte XOR"
    }
  ]
}
```

---

## 6. Citations

1. "Deobfuscation of Control Flow Flattening" (2020)
2. "Automated Detection of Encrypted Strings in Malware" (2019)
3. "Semantics-Preserving Obfuscation Detection" (2018)
4. "Dead Code Elimination in Obfuscated Binaries" (2017)
5. Detect It Easy: Obfuscator signatures (https://github.com/horsicq/Detect-It-Easy)

---

## 7. Recommendation

**Enhance**: String decryption detection + opcode substitution detection
- **Scope**: Pattern-based detection + simple decryption
- **Limitations**: No full CFG, no control flow unflattening
- **Effort**: ~200-300 LOC C++
- **Value**: Improved deobfuscation for common patterns
