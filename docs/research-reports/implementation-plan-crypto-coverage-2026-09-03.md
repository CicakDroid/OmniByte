# Implementation Plan: Crypto Detection Coverage Update
**Date**: 2026-09-03
**Status**: P1-P10 Complete, Coverage Gap Analysis

## Current Coverage (32/49 algorithms = 65%)

### Implemented Algorithms
| # | Algorithm | Detection Method | Status |
|---|-----------|-----------------|--------|
| 1 | AES | S-box (256 bytes) | ✅ |
| 2 | DES | S-box 1 (64 bytes) | ✅ |
| 3 | SHA-256 | H0 (32 bytes) | ✅ |
| 4 | MD5 | T constants (256 bytes) | ✅ |
| 5 | Blowfish | S-box (4168 bytes) | ✅ |
| 6 | XXTEA/TEA | Delta (4 bytes) | ✅ |
| 7 | RC4 | KSA pattern | ✅ |
| 8 | Whirlpool | IV constant (64 bytes) | ✅ |
| 9 | RIPEMD-160 | H0 (20 bytes) | ✅ |
| 10 | Camellia | S-box (1024 bytes) | ✅ |
| 11 | Serpent | S-box (128 bytes) | ✅ |
| 12 | Twofish | P-box (2048 bytes) | ✅ |
| 13 | GOST | S-box (128 bytes) | ✅ |
| 14 | RC2 | S-box (256 bytes) | ✅ |
| 15 | ChaCha/Salsa20 | Constants (32 bytes) | ✅ |
| 16 | SHA-1 | H0 (20 bytes) | ✅ |
| 17 | SHA-384 | H0 (48 bytes) | ✅ |
| 18 | SHA-512 | H0 (64 bytes) | ✅ |
| 19 | MD2 | PI_SUBST (256 bytes) | ✅ |
| 20 | MD4 | T constants (64 bytes) | ✅ |
| 21 | SEED | KC (64 bytes) | ✅ |
| 22 | LEA | Delta (32 bytes) | ✅ |
| 23 | Tiger | IV (24 bytes) | ✅ |
| 24 | HAVAL | IV (32 bytes) | ✅ |
| 25 | BLAKE2s | IV (32 bytes) | ✅ |
| 26 | Keccak/SHA-3 | Round constants (192 bytes) | ✅ |
| 27 | SIMON-64/128 | z3 constant (8 bytes) | ✅ |
| 28 | Skipjack | F-table (256 bytes) | ✅ |
| 29 | Square | S-box (32 bytes) | ✅ |
| 30 | SHARK | S-box (32 bytes) | ✅ |
| 31 | Curve25519 | Field prime (32 bytes) | ✅ |
| 32 | RC5/RC6 | Constant (8 bytes) | ✅ |

---

## Missing Algorithms (17 remaining)

### Tier 1: Easy to Add (< 256 bytes)
| # | Algorithm | Size | Difficulty | Notes |
|---|-----------|------|------------|-------|
| 1 | MD5MAC | 48 bytes | Easy | MD5 with different IV |
| 2 | SHACAL2 | 256 bytes | Easy | SHA-2 based block cipher |
| 3 | WAKE | 40 bytes | Easy | Stream cipher T-table |
| 4 | HIGHT | 640 bytes | Easy | Korean lightweight cipher |

### Tier 2: Medium (256-2048 bytes)
| # | Algorithm | Size | Difficulty | Notes |
|---|-----------|------|------------|-------|
| 5 | CAST-128 | 8192 bytes | Medium | S-box (4×256 bytes) |
| 6 | CAST-256 | 1536 bytes | Medium | T tables |
| 7 | SAFER | 512 bytes | Medium | EXP/LOG tables |
| 8 | MARS | 2048 bytes | Medium | S-box |
| 9 | RawDES | 2048 bytes | Medium | Spbox (extended DES) |

### Tier 3: Large (2048+ bytes)
| # | Algorithm | Size | Difficulty | Notes |
|---|-----------|------|------------|-------|
| 10 | Rijndael | 10240 bytes | Medium | Te/Td tables (AES extended) |
| 11 | Kalyna | 34816 bytes | Hard | Ukrainian standard, very large |
| 12 | ge25519 | 36936 bytes | Hard | Curve25519 point tables |
| 13 | modm | 224 bytes | Easy | Modular multiplication |

### Tier 4: Duplicate/Overlapping
| # | Algorithm | Size | Difficulty | Notes |
|---|-----------|------|------------|-------|
| 14 | BLAKE2b | 64 bytes | Skip | Same as SHA-512 H0 |
| 15 | PKCS Digest | 127 bytes | Skip | Not crypto constants |
| 16 | CRC32 | 1024 bytes | Skip | Not crypto |
| 17 | zdeflate/zinflate | 1496 bytes | Skip | Compression, not crypto |

---

## Recommended Implementation Order

### Phase 11: Quick Wins (Tier 1)
1. **MD5MAC** — 48 bytes, different IV from MD5
2. **SHACAL2** — 256 bytes, SHA-256 based block cipher
3. **WAKE** — 40 bytes, stream cipher T-table
4. **HIGHT** — 640 bytes, Korean lightweight cipher

### Phase 12: Medium Ciphers (Tier 2)
5. **CAST-128** — 8192 bytes, S-box
6. **CAST-256** — 1536 bytes, T tables
7. **SAFER** — 512 bytes, EXP/LOG tables
8. **MARS** — 2048 bytes, IBM S-box

### Phase 13: Extended Tables (Tier 3)
9. **Rijndael Te/Td** — 10240 bytes, AES extended tables
10. **modm** — 224 bytes, modular multiplication

---

## Android Considerations

### Size Limits
- **Recommended**: < 4096 bytes per detection
- **Maximum**: 16384 bytes (with streaming verification)
- **Skip**: > 32KB (Kalyna, ge25519) — too large for efficient scanning

### False Positive Risk
- **Low**: IV/H0 constants (unique, 20+ bytes)
- **Medium**: S-boxes (may appear in non-crypto contexts)
- **High**: Small constants (< 8 bytes) — requires additional verification

### Performance Impact
- **17 new algorithms**: ~15KB additional constants
- **Scan time**: < 1ms additional per 1MB binary
- **Memory**: ~50KB static const arrays

---

## Final Coverage Target

| Phase | Algorithms | Cumulative | % of Database |
|-------|-----------|------------|---------------|
| P1-P10 | 32 | 32 | 65% |
| P11 | 4 | 36 | 73% |
| P12 | 4 | 40 | 82% |
| P13 | 2 | 42 | 86% |
| **Total** | **42** | **42** | **86%** |

**Remaining 7 algorithms** (14%): Excluded due to size (> 32KB), duplication, or non-crypto nature.

---

## References
1. FindCrypt-Ghidra database: `https://github.com/TorgoTorgo/ghidra-findcrypt/blob/main/FindCrypt/data/database.json`
2. IDA FindCrypt constants: `https://github.com/you0708/ida/blob/master/idapython_tools/findcrypt/consts.py`
3. YARA rules: `https://github.com/polymorf/findcrypt-yara/blob/master/findcrypt3.rules`
