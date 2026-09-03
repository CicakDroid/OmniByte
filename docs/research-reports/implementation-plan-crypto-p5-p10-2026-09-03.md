# Implementation Plan: Crypto Detection P5-P10
**Date**: 2026-09-03
**Reference**: `crypto-detection-p5-p10-hex-constants-2026-09-03.md`

## Approach
Add `check*()` methods to `FindCrypt3.cpp` following existing pattern from P1-P4. Each method returns `AlgorithmConst` struct with name, algo, byteArray, size, offset, section.

## Implementation Order

### P5: Hash Functions
1. `checkSHA1()` — 20 bytes: `67452301EFCDAB8998BADCFE10325476C3D2E1F0`
2. `checkSHA384()` — 48 bytes: `CBBB9D5DC1059ED8...47B5481DBEFA4FA4`
3. `checkSHA512()` — 64 bytes: `6A09E667F3BCC908...5BE0CD19137E2179`
4. `checkMD2()` — 256 bytes: PI_SUBST S-box from RFC 1319
5. `checkMD4()` — 64 bytes: T[1..64] from RFC 1320

### P6: Block Ciphers
6. `checkTripleDES()` — 24 bytes: `000000000000000000000000000000000000000000000000` (weak key)
7. `checkXTEA()` — 4 bytes: `9E3779B9` (delta)
8. `checkTEAN()` — 4 bytes: `9E3779B9` (same delta, different usage)
9. `checkSEED()` — 512 bytes: S0+S1 S-boxes
10. `checkLEA()` — 32 bytes: Delta constant

### P7: Extended Hash/MAC
11. `checkSerpent()` — 128 bytes: S0-S7 S-boxes
12. `checkTwofish()` — 2048 bytes: S0+S1 S-boxes (large but identifiable)
13. `checkWhirlpool()` — 64 bytes: IV constant
14. `checkRIPEMD160()` — 20 bytes: IV constant

### P8: Legacy Ciphers
15. `checkGOST()` — 128 bytes: S1-S8 S-boxes
16. `checkRC2()` — 256 bytes: S-box
17. `checkCAST256()` — 1024 bytes: S0-S3 S-boxes
18. `checkSkipjack()` — 256 bytes: F-table
19. `checkTiger()` — 24 bytes: IV constant
20. `checkHAVAL()` — 32 bytes: IV constant
21. `checkBLAKE2b()` — 64 bytes: IV constant (same as SHA-512 H0)
22. `checkBLAKE2s()` — 32 bytes: IV constant
23. `checkKeccak()` — 192 bytes: Round constants
24. `checkWAKE()` — 32 bytes: T-table fixed values

### P9: Modern Ciphers
25. `checkSquare()` — 256 bytes: S-box
26. `checkSHARK()` — 256 bytes: S-box
27. `checkSIMON64_128()` — 8 bytes: z3 constant `DBAC653E0048A734`
28. `checkKalyna()` — 1024 bytes: S-box
29. `checkDonna()` — 32 bytes: Field prime

### P10: Extended (Already Covered)
- Camellia: Implemented in P3
- ChaCha20: Implemented in P3

## Integration
Add all new `check*()` calls to `FindCrypt3::scanAll()` method.

## Verification
Compile check: `g++ -std=c++17 -fsyntax-only FindCrypt3.cpp`
