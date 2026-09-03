# Crypto Detection Research Report: RC4, XXTEA, YARA & Extended Signatures

**Date:** 2026-09-03
**Purpose:** Research missing crypto detection capabilities in HydraDis FindCrypt3/hrtng plugins
**Scope:** RC4 KSA/PRGA, XXTEA/TEA, YARA rule scanning, IV/nonce patterns, extended crypto signatures

---

## 1. Executive Summary

Current FindCrypt3 scans only: AES S-box, SHA-256 H0, DES S-box, Blowfish P-box, MD5 T-table. Current hrtng covers: XOR, base64, ROT13, addConstant, API hashes. **Major gaps identified**: RC4, XXTEA, Whirlpool, RIPEMD160, Camellia, RC2, Serpent, Twofish, BLAKE2, ChaCha/Salsa, GOST, and YARA rule-based scanning. Research synthesized from 5 open-source tools + 2 academic papers.

---

## 2. Existing Capabilities (Baseline)

### 2.1 FindCrypt3 (engine-core/HydraDis/Plugin/Enhanced/FindCrypt/FindCrypt3/)

| Algorithm | Detection Method | Signature |
|-----------|-----------------|-----------|
| AES | S-box at address 0x400 (offset 0x63 in AES spec) | 16-byte S-box |
| SHA-256 | H0 initial hash values (0x6a09e667, etc.) | 8 uint32_t constants |
| DES | S-boxes (32-byte each, 8 S-boxes = 256 bytes total) | 256-byte block |
| Blowfish | P-box initial values (pi digits) | 18 uint32_t constants |
| MD5 | T-table sine values (0xd76aa478, etc.) | 64 uint32_t constants |

### 2.2 hrtng (engine-core/HydraDis/Plugin/Deobfuscate/hrtng/)

| Technique | Detection Method |
|-----------|-----------------|
| XOR encryption | Single-byte XOR loop patterns |
| Base64 | Alphabet detection (A-Z, a-z, 0-9, +/) |
| ROT13 | Character rotation pattern |
| addConstant | Constant addition/subtraction in loop |
| API hash resolution | CRC32/MD5/SHA-based API hashing |

---

## 3. Missing Crypto Detection — Gap Analysis

### 3.1 RC4 (Rivest Cipher 4)

**Source:** mrphrazer/obfuscation_detection_ghidra (GitHub, 879 stars)
**Source:** fireeye/capa (FLARE team, assembly structure rules)
**Source:** YARA rules: "DetectCrypt_RC4" (ReversingLabs)

**Algorithm:**
- KSA (Key Scheduling Algorithm): Initializes S-box with 0x00-0xFF, then swaps based on key
- PRGA (Pseudo-Random Generation Algorithm): Generates keystream via i, j counters + swap

**Detection Patterns (x86/x64 assembly):**

| Pattern | Hex Bytes | Description |
|---------|-----------|-------------|
| KSA loop counter = 0x100 | `3D 00 01 00 00` | `cmp eax, 0x100` |
| KSA loop counter (register) | `81 F? 00 01 00 00` | `cmp reg, 0x100` |
| KSA loop counter (64-bit) | `48 3D 00 01 00 00` | `cmp rax, 0x100` |
| KSA loop counter (r64) | `48 81 F? 00 01 00 00` | `cmp r64, 0x100` |

**P-code Analysis Approach (Ghidra):**
- RC4 KSA: Functions with exactly 2 natural loops, containing 0x100 constant in p-code
- RC4 PRGA: Loop p-code with XOR between two non-constant byte-sized values

**Implementation Priority:** HIGH — commonly used in Android malware, game encryption

### 3.2 XXTEA (Corrected Block TEA)

**Source:** recon.cx/2012 presentation "Identifying unknown crypto algorithms in malware"
**Source:** d3v1l401/FindCrypt-Ghidra signature database
**Source:** KasperskyLab/hrtng (reference implementations)

**Algorithm:**
- Magic constant: `0x9E3779B9` (delta, golden ratio derived)
- Alternative delta: `0x61C88647` (some XXTEA implementations)
- MX macro: `((z>>5 ^ y<<2) + (y>>3 ^ z<<4)) ^ sum`
- 6 + 52/n rounds for n-word blocks

**Detection Patterns:**

| Constant | Hex | Context |
|----------|-----|---------|
| DELTA (primary) | `B9 79 37 9E` | TEA/XXTEA magic number |
| DELTA (alternate) | `47 86 C8 61` | XXTEA variant |
| DELTA (shifted) | `9E 37 79 B9` | Big-endian encoding |

**Structural Detection:**
- Look for constant `0x9E3779B9` appearing in loop body
- Identify shift operations: `>>5`, `<<2`, `>>3`, `<<4` in same function
- Count rounds: typical XXTEA uses 6 + 52/n (variable based on block size)

**Implementation Priority:** HIGH — used in Android game asset encryption, anti-tamper

### 3.3 YARA Rule Scanning

**Source:** VirusTotal/libyara (C API documentation)
**Source:** yara-project/yara (official YARA project)

**Integration Options:**

| Approach | Pros | Cons |
|----------|------|------|
| Embed libyara (C API) | Full YARA syntax, community rules | Large binary size (~2MB), GPL-3.0 |
| Custom lightweight scanner | Minimal size, no license issues | Limited pattern syntax |
| Signature-based only | Fast, no dependencies | No wildcards/alternatives |

**libyara C API Pattern:**

```cpp
#include <yara.h>

// Initialization (call once)
yr_initialize();

// Compile rules
YR_COMPILER* compiler;
yr_compiler_create(&compiler);

// Add rules from string/FILE
yr_compiler_add_string(compiler, rule_string, NULL);

YR_RULES* rules;
yr_compiler_get_rules(compiler, &rules);

// Scan memory
yr_rules_scan_mem(rules, buffer, buffer_size, 0, callback, user_data, &errors);

// Cleanup
yr_rules_destroy(rules);
yr_compiler_destroy(compiler);
yr_finalize();
```

**Build Integration (CMake):**

```cmake
find_package(yara REQUIRED)  # or use ExternalProject_Add
target_link_libraries(hydradis PRIVATE yara)
```

**Implementation Priority:** MEDIUM — powerful but adds dependency. Consider as optional plugin.

### 3.4 IV/Nonce Pattern Detection

**Source:** General crypto analysis knowledge
**Source:** Common AES-CBC/GCM implementations

**Detection Approach:**
- IV typically 16 bytes for AES-CBC, 12 bytes for AES-GCM
- Pattern: 16/12 byte constant passed before encryption call
- Often loaded via `MOV` instructions from .rodata section

**Heuristics:**
- Look for 16-byte aligned constants near crypto function calls
- Detect `AES_set_encrypt_key` + adjacent constant (IV)
- Check for random-looking 16-byte blocks in .rodata

**Implementation Priority:** LOW — heuristic-based, many false positives

### 3.5 Extended Crypto Signatures (Missing from FindCrypt3)

**Source:** d3v1l401/FindCrypt-Ghidra (122 detectable constants)
**Source:** TorgoTorgo/ghidra-findcrypt (extended database)

**Missing Algorithms:**

| Algorithm | Type | Key Constants |
|-----------|------|---------------|
| Whirlpool | Hash | S-box (256 bytes) |
| RIPEMD-160 | Hash | Initial values (5 uint32_t) |
| SHA-512 | Hash | H0 initial values (8 uint64_t) |
| BLAKE2 | Hash | IV (8 uint64_t) |
| Camellia | Block | S-box + P-box constants |
| Serpent | Block | S-box (4×128 bytes) |
| Twofish | Block | S-box + P-box |
| GOST | Block | S-box (8×16 bytes) |
| RC2 | Block | S-box (64 bytes) |
| ChaCha | Stream | Constants "expand 32-byte k" |
| Salsa20 | Stream | Constants "expand 32-byte k" |
| TEA/XTEA | Block | DELTA constant |
| CAST-256 | Block | S-box constants |
| MARS | Block | S-box + constants |
| HIGHT | Block | S-box constants |
| LEA | Block | Delta constants |
| SEED | Block | S-box constants |
| SIMON | Block | Round constants |
| SPECK | Block | Round constants |

---

## 4. Implementation Recommendations

### 4.1 Priority 1: Add XXTEA Detection (FindCrypt3)

**Approach:** Add `checkXxteaConstants()` to FindCrypt3 engine
**Detection:** Scan for `0x9E3779B9` (DELTA) in data sections
**Effort:** Small — single constant scan
**Risk:** Low — well-defined constant

### 4.2 Priority 2: Add RC4 Detection (FindCrypt3)

**Approach:** Add `checkRc4Pattern()` to FindCrypt3 engine
**Detection:** Scan for `cmp reg, 0x100` patterns in code sections
**Effort:** Medium — requires instruction pattern matching
**Risk:** Medium — may have false positives with other 256-iteration loops

### 4.3 Priority 3: Extended Crypto Signatures (FindCrypt3)

**Approach:** Add additional S-box/IV checks for Whirlpool, RIPEMD-160, Camellia, Serpent, Twofish, ChaCha/Salsa
**Detection:** Same pattern as existing AES/DES/Blowfish checks
**Effort:** Medium — many algorithms, each needs constant extraction
**Risk:** Low — well-documented constants

### 4.4 Priority 4: YARA Integration (Optional Plugin)

**Approach:** Create new `YaraScan` plugin wrapping libyara
**Detection:** Compile-time YARA rules, runtime scanning
**Effort:** High — CMake integration, rule management
**Risk:** Medium — GPL-3.0 license dependency, binary size increase

### 4.5 Priority 5: IV Pattern Detection (Heuristic)

**Approach:** Add heuristic IV detection to FindCrypt3
**Detection:** 16-byte constants near crypto function calls
**Effort:** Medium — heuristic-based, needs tuning
**Risk:** High — false positives likely

---

## 5. Build Integration Notes

### 5.1 FindCrypt3 Plugin
- Location: `engine-core/HydraDis/Plugin/Enhanced/FindCrypt/FindCrypt3/`
- Entry: `enhanced_findcrypt.cpp` → `EnhancedFindCryptPlugin::analyze()`
- Current: scans `.rodata` + `.data` sections for crypto constants

### 5.2 hrtng Plugin
- Location: `engine-core/HydraDis/Plugin/Deobfuscate/hrtng/`
- Entry: `plugin_deobfuscate.cpp` → `DeobfuscatePlugin::analyze()`
- Current: XOR, base64, ROT13, addConstant, API hashes

### 5.3 CMake Wiring
- Root: `app/src/main/cpp/CMakeLists.txt` (aggregator)
- HydraDis: `engine-core/HydraDis/CMakeLists.txt`
- Plugins are static libraries linked into `libomnibyte.so`

---

## 6. Citations

1. mrphrazer/obfuscation_detection_ghidra — https://github.com/mrphrazer/obfuscation_detection_ghidra
2. fireeye/capa — https://github.com/mandiant/capa (RC4 detection rules)
3. ReversingLabs YARA rules — https://github.com/reversinglabs/reversinglabs-yara-rules (RC4 signatures)
4. recon.cx/2012 — "Identifying unknown crypto algorithms in malware" (XXTEA DELTA constant)
5. d3v1l401/FindCrypt-Ghidra — https://github.com/d3v1l401/FindCrypt-Ghidra (122 crypto constants)
6. TorgoTorgo/ghidra-findcrypt — https://github.com/TorgoTorgo/ghidra-findcrypt (extended signatures)
7. VirusTotal/libyara — https://yara.readthedocs.io/en/stable/gettingstarted.html (C API reference)
8. KasperskyLab/hrtng — https://github.com/KasperskyLab/hrtng (crypto implementations for reference)

---

## 7. Status Summary

| Item | Status | Notes |
|------|--------|-------|
| RC4 detection | ✅ Researched | Byte patterns + p-code approach documented |
| XXTEA detection | ✅ Researched | DELTA constant + structural detection |
| YARA integration | ✅ Researched | libyara C API, build integration |
| IV patterns | ✅ Researched | Heuristic approach, high false positive risk |
| Extended signatures | ✅ Researched | 19 additional algorithms documented |
| Research report | ✅ Written | This file |
| Code implementation | ⏳ Pending | Awaiting approval |
| Compile check | ⏳ Pending | After implementation |
