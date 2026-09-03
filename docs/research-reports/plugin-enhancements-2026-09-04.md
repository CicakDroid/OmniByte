# Plugin Enhancement Documentation
**Date:** 2026-09-04  
**Version:** 1.0.0  
**Status:** Complete

## Overview
Documentation for enhanced HydraDis plugins with new detection capabilities.

---

## FunctionResolver Enhancement

### New Capabilities
1. **PLT/GOT Stub Detection**
   - Identifies PLT stubs: `ldr x16, [x16, ...]; br x16` pattern
   - Marks as `isPltStub: true` and `isImport: true`

2. **Itanium ABI Name Demangling**
   - Parses `_Z...` mangled names to human-readable
   - Example: `_ZN3foo3barEv` → `foo::bar`

3. **Symbol Name Mapping**
   - Resolves function addresses to symbol names
   - Adds `demangledName` field for C++ symbols

### JSON Output Changes
```json
{
  "startAddr": "0x3000",
  "name": "plt_3000",
  "demangledName": "printf",
  "isPltStub": true,
  "isImport": true
}
```

---

## CFG Enhancement

### New Capabilities
1. **Loop Detection**
   - Identifies back-edges (successor ≤ block start)
   - Sets `isLoopHeader: true` on target block

2. **Unresolved Edges**
   - Tracks branches to non-existent blocks
   - Useful for indirect jumps or data corruption

3. **Dominator Tree**
   - Computes dominators for each block
   - Entry block dominates itself

### JSON Output Changes
```json
{
  "start": "0x1000",
  "isLoopHeader": true,
  "dominators": ["0x1000", "0x1020"],
  "unresolvedEdges": [
    {"from": "0x1040", "to": "0x5000"}
  ]
}
```

---

## AST Enhancement

### New Capabilities
1. **Expression Recovery**
   - ARITH node for: add, sub, mul, sdiv, udiv
   - Identifies arithmetic operations

2. **Load/Store Classification**
   - LOAD node: ldr, ldrb, ldrh, ldp, ldrsb, ldrsh
   - STORE node: str, strb, strh, stp

3. **Instruction Type Detection**
   - ASSIGN node: mov, orr, and, eor
   - Improved AST node classification

### Node Types
| Type | Description |
|------|-------------|
| `arith` | Arithmetic operation (add, sub, mul) |
| `load` | Memory read operation |
| `store` | Memory write operation |
| `assign` | Register assignment |

---

## RTTI Enhancement

### New Capabilities
1. **Extended Section Scanning**
   - Scans `.data.rel.ro.local` in addition to `.rodata` and `.data.rel.ro`

2. **Type Deduplication**
   - Removes duplicate entries by mangled name

3. **vtableSize Field**
   - Counts entries per vtable
   - Estimates virtual function count

### JSON Output Changes
```json
{
  "mangled": "_ZTV3foo",
  "vtableSize": 5,
  "baseClasses": ["bar", "baz"]
}
```

---

## Deobfuscate Enhancement

### New Capabilities
1. **Junk Code Detection**
   - Identifies `nop` instructions
   - Detects self-mov patterns: `mov x0, x0`

2. **Dead Store Detection**
   - Flags `str` followed immediately by overwrite
   - Tracks register and overwritten address

3. **Confidence Field**
   - Per-detection confidence scores (0.0-1.0)
   - Higher = more certain detection

### Detection Types
| Type | Confidence | Description |
|------|------------|-------------|
| `junk_code` | 0.95 | nop or self-mov |
| `string_decryption` | 0.90 | ldr + eor pattern |
| `dead_store` | 0.85 | str followed by overwrite |
| `xor_obfuscation` | 0.80 | eor/xor instruction |
| `opcode_substitution` | 0.70 | table-driven pattern |

### JSON Output Changes
```json
{
  "type": "dead_store",
  "address": "0x1000",
  "instruction": "str x0, [sp]",
  "overwrittenBy": "0x1008",
  "confidence": 0.85
}
```

---

## Commit History

| Commit | Plugin | Enhancement |
|--------|--------|-------------|
| `4962aa9` | FunctionResolver | PLT/GOT, demangling, symbols |
| `70ac6cd` | CFG | loops, unresolved edges, dominators |
| `bd04a37` | AST | expression recovery, load/store |
| `a373de0` | RTTI | .data.rel.ro.local, dedup, vtableSize |
| `48df4e7` | Deobfuscate | junk code, dead store, confidence |

---

## Usage Notes

### FunctionResolver
- PLT stubs are marked as imports
- Demangled names available for `_Z` symbols
- Symbol mapping resolves addresses to function names

### CFG
- Loop headers identified by back-edges
- Dominator tree useful for optimization analysis
- Unresolved edges indicate indirect jumps

### AST
- ARITH nodes represent arithmetic expressions
- LOAD/STORE nodes classify memory operations
- Improved instruction type classification

### RTTI
- vtableSize estimates virtual function count
- Deduplication removes redundant type entries
- Extended section scanning catches more RTTI data

### Deobfuscate
- Confidence scores help prioritize analysis
- Dead store detection finds unused writes
- Junk code detection identifies obfuscation artifacts

---

## Future Enhancements

### FunctionResolver
- Indirect jump resolution
- PLT/GOT relocation analysis

### CFG
- Loop nesting depth
- Natural loop detection

### AST
- Expression tree recovery
- Array access detection

### RTTI
- MSVC RTTI support
- Type hierarchy visualization

### Deobfuscate
- Control flow unflattening
- Opaque predicate detection

---

## References

- Itanium C++ ABI: https://itanium-cxx-abi.github.io/cxx-abi/abi.html
- ARM64 Instruction Set: https://developer.arm.com/documentation/
- LLVM RTTI: https://llvm.org/docs/HowToSetUpLLVMStyleRTTI.html
