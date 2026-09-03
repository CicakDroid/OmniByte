# Implementation Plan: Crypto Detection P1-P4

**Date:** 2026-09-03
**Scope:** XXTEA (P1), RC4 (P2), Extended signatures (P3), YARA plugin (P4)

---

## P1: XXTEA Detection — FindCrypt3

**File:** `engine-core/HydraDis/Plugin/Enhanced/FindCrypt/FindCrypt3/FindCrypt3.cpp`

**Add constant:**
```cpp
static const uint32_t XXTEA_DELTA = 0x9E3779B9;
static const uint32_t XXTEA_DELTA_ALT = 0x61C88647;
```

**Add method:** `bool checkXxtea(const uint8_t* data, size_t size, size_t offset) const`
- Scan for 4-byte match of `XXTEA_DELTA` (little-endian: `B9 79 37 9E`) or `XXTEA_DELTA_ALT`
- Confidence: 0.85 for primary, 0.70 for alternate (more ambiguous)

**Wire into scanRegion:** Loop over data checking every 4-byte alignment.

---

## P2: RC4 Detection — FindCrypt3

**File:** `engine-core/HydraDis/Plugin/Enhanced/FindCrypt/FindCrypt3/FindCrypt3.cpp`

**Add method:** `bool checkRc4Ksa(const uint8_t* data, size_t size, size_t offset) const`
- Pattern: `3D 00 01 00 00` (cmp eax, 0x100)
- Pattern: `81 F? 00 01 00 00` (cmp reg, 0x100)
- Pattern: `48 3D 00 01 00 00` (cmp rax, 0x100)
- Pattern: `48 81 F? 00 01 00 00` (cmp r64, 0x100)
- Confidence: 0.75 (may have false positives with other 256-iteration loops)

**Wire into scanRegion:** Scan data as code section for byte patterns.

---

## P3: Extended Crypto Signatures — FindCrypt3

**File:** `engine-core/HydraDis/Plugin/Enhanced/FindCrypt/FindCrypt3/FindCrypt3.h`
**File:** `engine-core/HydraDis/Plugin/Enhanced/FindCrypt/FindCrypt3/FindCrypt3.cpp`

**Add constants (from FindCrypt-Ghidra database):**

| Algorithm | Constant | Size | Hex |
|-----------|----------|------|-----|
| Whirlpool | S-box (first 32 bytes) | 32 | `0x18 0x23 0xC6 0xE8 ...` |
| RIPEMD-160 | H0 (5 × uint32) | 20 | `0x67452301, 0xEFCDAB89, ...` |
| SHA-512 | H0 (8 × uint64) | 64 | `0x6A09E667F3BCC908, ...` |
| Camellia | S-box (first 32 bytes) | 32 | Specific to Camellia |
| Serpent | S-box (first 32 bytes) | 32 | Specific to Serpent |
| Twofish | S-box (first 32 bytes) | 32 | Specific to Twofish |
| GOST | S-box (first 16 bytes) | 16 | Specific to GOST |
| RC2 | S-box (first 16 bytes) | 16 | Specific to RC2 |
| ChaCha | Constants "expand 32-byte k" | 16 | `0x65, 0x78, 0x70, 0x61, ...` |
| Salsa20 | Constants "expand 32-byte k" | 16 | Same as ChaCha |

**Add methods:** `checkWhirlpool()`, `checkRipemd160()`, `checkSha512()`, `checkCamellia()`, `checkSerpent()`, `checkTwofish()`, `checkGost()`, `checkRc2()`, `checkChacha()`

**Wire into scanRegion:** Add loops for each new constant.

---

## P4: YARA Plugin (Optional)

**New directory:** `engine-core/HydraDis/Plugin/Enhanced/YaraScan/`

**Files:**
- `YaraScan.h` — `YaraScanEngine` class
- `YaraScan.cpp` — libyara wrapper implementation
- `plugin_yarascan.cpp` — plugin entry point
- `CMakeLists.txt` — build config (conditional on libyara)

**Approach:**
- Wrap libyara C API: `yr_initialize()`, `yr_compiler_create()`, `yr_rules_scan_mem()`
- Compile embedded YARA rules at init time
- Scan binary sections for matches
- Make optional via `#ifdef HAS_YARA` or CMake option

**CMake wiring:**
```cmake
option(ENABLE_YARA "Enable YARA rule scanning" OFF)
if(ENABLE_YARA)
    find_package(yara REQUIRED)
    target_sources(hydradis PRIVATE YaraScan/plugin_yarascan.cpp)
    target_link_libraries(hydradis PRIVATE yara)
endif()
```

---

## Bug Fix: enhanced_findcrypt.cpp

Current code has `hits` vector always empty because `scanRegion()` is never called on the actual instruction bytes. Fix:
- Collect all instruction bytes into a buffer
- Call `engine.scanRegion(buffer.data(), buffer.size())`
- Return hits in JSON output

---

## Status

| Item | Status |
|------|--------|
| Implementation plan | ✅ Written |
| P1: XXTEA | ⏳ Pending |
| P2: RC4 | ⏳ Pending |
| P3: Extended signatures | ⏳ Pending |
| P4: YARA plugin | ⏳ Pending |
| Bug fix: enhanced_findcrypt.cpp | ⏳ Pending |
| Compile check | ⏳ Pending |
