# Research Report: Function Resolution and Boundary Detection

**Date**: 2026-09-03
**Plugin**: Enhanced/FunctionResolver (enhanced_functionresolver.cpp)
**Status**: STUB — returns false, "not yet implemented"

---

## 1. Current State

The `Enhanced/FunctionResolver` plugin is a 29-line stub that returns `result.success = false`. It implements the `IPlugin` interface but performs no analysis.

### What Function Resolution Means

Function resolution = identifying where functions begin and end, their names, calling conventions, parameters, and relationships. This is fundamental for:
- Disassembly (only disassemble known code regions)
- Decompilation (need function boundaries for CFG construction)
- Symbolic execution (need function entry/exit points)
- Code review (understand function structure)

---

## 2. Research Sources

### Source 1: Binary Ninja 4-Phase CFG Recovery
- **URL**: https://docs.binary.ninja/dev/bn-arch-mlil.html
- **Key Insight**: Uses SMT-based function prologue detection
- **Algorithm**:
  1. Identify function entries via exports, imports, call targets
  2. Detect prologues (push {fp, lr}; sub sp, sp, #N)
  3. Recursive descent from each entry
  4. Stop at function epilogue (pop {fp, pc}; ret)
- **Relevance**: HIGH — clean, well-documented pipeline

### Source 2: Ghidra Function ID
- **Documentation**: https://ghidra-sre.org/HelpTopic/ghidra/core/pluginmgr/pluginFunctionId/
- **Key Insight**: Uses pattern matching + heuristics for function identification
- **Algorithm**:
  1. Scan for function prologues (known patterns per architecture)
  2. Validate via call targets (cross-references)
  3. Use FLIRT signatures for library detection
  4. Apply heuristics for function boundaries
- **Relevance**: MEDIUM — Ghidra is Java-based, but algorithms are applicable

### Source 3: IDA Pro Function Analysis
- **Documentation**: https://hex-rays.com/decompiler/
- **Key Insight**: Uses recursive descent + data flow analysis
- **Algorithm**:
  1. Linear sweep to find code regions
  2. Recursive descent from known entries
  3. Data flow analysis for parameter detection
  4. Type propagation for variable typing
- **Relevance**: MEDIUM — too complex for our use case

### Source 4: SMDA Function Discovery
- **Paper**: "Exposing the Use of Encryption Algorithms in Malware" (2019)
- **URL**: https://doi.org/10.3390/app9142793
- **Key Insight**: Combines recursive disassembly with API-based function discovery
- **Algorithm**:
  1. Identify function entries via imports, exports, call targets
  2. Recursive descent from each entry
  3. Detect function boundaries via prologue/epilogue patterns
  4. Validate via call graph consistency
- **Relevance**: HIGH — directly applicable to C++/Capstone

---

## 3. Feasible Implementation Approach

### Function Boundary Detection

Since `PluginContext` provides `disassemblyResults` and `binary` (with sections, symbols, imports, exports), we can:

1. **Identify function entries**:
   - Export symbols (function names in symbol table)
   - Import stubs (PLT entries)
   - Call targets (bl/call operands)
   - Function prologues (push {fp, lr}; sub sp, sp, #N)

2. **Detect function prologues**:
   - ARM64: `stp x29, x30, [sp, #-N]!` / `mov x29, sp`
   - ARM: `push {fp, lr}` / `sub sp, sp, #N`
   - x86: `push rbp` / `mov rbp, rsp` / `sub rsp, N`
   - x86_64: `push rbp` / `mov rbp, rsp`

3. **Detect function epilogues**:
   - `ret` / `pop {fp, pc}` / `bx lr`
   - `add sp, sp, #N` / `pop {..., pc}`

4. **Build function map**:
   - For each entry, scan forward until epilogue
   - Record function boundaries (start, end)
   - Build call graph (who calls whom)

### Data Structures

```cpp
struct FunctionInfo {
    uint64_t startAddr;
    uint64_t endAddr;
    std::string name;
    std::vector<uint64_t> callers;  // who calls this function
    std::vector<uint64_t> callees;  // what this function calls
    bool isExport;
    bool isImport;
};

struct FunctionResolverResult {
    std::map<uint64_t, FunctionInfo> functions;  // startAddr → info
    std::vector<uint64_t> unresolvedCalls;  // indirect calls
};
```

### What We CAN Do

- Detect function entries via exports, imports, call targets
- Identify prologues/epilogues per architecture (ARM64, x86)
- Build basic function map (start, end, name)
- Construct call graph (who calls whom)
- Detect recursive calls

### What We CANNOT Do (without full data flow analysis)

- Detect indirect calls (computed targets)
- Resolve virtual function dispatch (need vtable walking)
- Identify parameters and return types
- Handle function pointers in data structures
- Detect tail calls and thunks

---

## 4. Dependencies

| Dependency | Required | Notes |
|------------|----------|-------|
| ELF parser | YES | Already available via IParser |
| Capstone (disassembler) | YES | Already available via IDisassembler |
| Symbol table | YES | Available via ParsedBinary.symbols |

**Conclusion**: Function resolution is feasible with existing ELF parser and disassembly output. The result will be basic function boundaries and call graph, suitable for further analysis.

---

## 5. Expected Output Format

```json
{
  "plugin": "Enhanced/FunctionResolver",
  "functions": [
    {
      "startAddr": "0x1000",
      "endAddr": "0x1200",
      "name": "main",
      "callers": ["0x2000"],
      "callees": ["0x3000", "0x4000"],
      "isExport": true,
      "isImport": false
    }
  ],
  "unresolvedCalls": ["0x5000"]
}
```

---

## 6. Citations

1. Binary Ninja documentation: MLIL and Function Recovery (https://docs.binary.ninja/dev/bn-arch-mlil.html)
2. Ghidra documentation: Function ID plugin (https://ghidra-sre.org/HelpTopic/ghidra/core/pluginmgr/pluginFunctionId/)
3. IDA Pro documentation: Hex-Rays Decompiler (https://hex-rays.com/decompiler/)
4. Zoller et al. "Exposing the Use of Encryption Algorithms in Malware" (2019)

---

## 7. Recommendation

**Implement**: Basic function boundary detection + call graph construction
- **Scope**: Function entry/exit detection + caller/callee mapping
- **Limitations**: No indirect call resolution, no parameter detection
- **Effort**: ~300-400 LOC C++
- **Value**: Enables further analysis (AST, CFG, symbolic execution)
