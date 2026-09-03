# Research Report: Control Flow Graph Recovery from Binary

**Date**: 2026-09-03
**Plugin**: Enhanced/CFG (enhanced_cfg.cpp)
**Status**: STUB — returns false, "not yet implemented"

---

## 1. Current State

The `Enhanced/CFG` plugin is a 29-line stub that returns `result.success = false`. It implements the `IPlugin` interface but performs no analysis.

### What CFG Recovery Means

CFG recovery = reconstructing the control flow graph from binary code:
- **Basic blocks**: sequences of instructions with single entry/exit
- **Edges**: jumps, calls, fall-throughs
- **Nodes**: basic blocks
- **Edges**: control flow transfers (conditional/unconditional)

The CFG is the foundation for all other analysis (AST recovery, decompilation, symbolic execution).

---

## 2. Research Sources

### Source 1: SMDA — Recursive Disassembler
- **Paper**: "Exposing the Use of Encryption Algorithms in Malware: A Machine Learning Approach" (2019)
- **URL**: https://doi.org/10.3390/app9142793
- **GitHub**: https://github.com/d3v1l70r4ng3/SMDA
- **Key Insight**: SMDA combines recursive disassembly with API-based CFG construction
- **Algorithm**:
  1. Identify function entries (exports, imports, call targets)
  2. Recursive descent from each entry
  3. Disassemble valid instructions (skip padding)
  4. Identify block boundaries (jmp/jcc/call/ret)
  5. Resolve edges (fall-through, branch targets)
- **Implementation**: Python, ~4000 LOC, uses Capstone
- **Relevance**: HIGH — directly applicable to our C++/Capstone setup

### Source 2: angr CFGFast
- **Documentation**: https://docs.angr.io/en/v9.2.81/analyses/cfg.html
- **Key Insight**: Uses linear sweep + recursive descent + jump tables + indirect jumps
- **Algorithm**:
  1. Linear sweep (scan for branch targets)
  2. Recursive descent from known entries
  3. Jump table resolution (pattern matching)
  4. Indirect jump analysis (symbolic execution)
  5. Function prologue detection
- **Relevance**: MEDIUM — we can simplify since we have pre-disassembled instructions

### Source 3: Binary Ninja 4-Phase CFG Recovery
- **URL**: https://docs.binary.ninja/dev/bn-arch-mlil.html
- **Key Insight**: 4-phase approach:
  1. Function prologue detection (SMT-based)
  2. Basic instruction disassembly
  3. Recursive descent for CFG
  4. Analysis pass (SSA, data flow)
- **Relevance**: HIGH — clean, well-documented pipeline

### Source 4: LLVM MachineBasicBlock
- **Documentation**: https://llvm.org/docs/doxygen/classllvm_1_1MachineBasicBlock.html
- **Key Insight**: MachineBasicBlock is the LLVM representation of CFG nodes
- **Relevance**: LOW — too tied to LLVM internals

---

## 3. Feasible Implementation Approach

### CFG Construction from Disassembled Instructions

Since `PluginContext` provides `disassemblyResults` (vector of `DisassemblyResult`), we can:

1. **Build instruction map**: `address → Instruction`
2. **Identify block leaders**:
   - First instruction of function
   - Branch targets
   - Instructions after branches
3. **Build basic blocks**:
   - Start at each leader
   - Continue until branch/ret/nop
4. **Resolve edges**:
   - Conditional branch → two edges (taken + fall-through)
   - Unconditional branch → one edge (taken)
   - Call → edge to callee
   - Ret → edge to caller (or exit)
5. **Detect functions**:
   - Export entries
   - Call targets
   - Prologue patterns (push {fp, lr}; sub sp, sp, #N)

### Data Structures

```cpp
struct BasicBlock {
    uint64_t startAddr;
    uint64_t endAddr;
    std::vector<Instruction> instructions;
    std::vector<uint64_t> successors;  // targets
    std::vector<uint64_t> predecessors;
};

struct Cfg {
    std::map<uint64_t, BasicBlock> blocks;  // startAddr → block
    std::set<uint64_t> entryPoints;         // function entries
};
```

### What We CAN Do

- Build basic blocks from disassembled instructions
- Detect simple control flow (if/else, loops)
- Identify function boundaries (export + call targets)
- Detect patterns like:
  - `cmp` + `jcc` → conditional branch
  - `bl`/`call` → function call
  - `ret` → function return

### What We CANNOT Do (without CFG from decompiler)

- Resolve indirect jumps (computed branches)
- Resolve jump tables
- Detect exception handling
- Full function boundary detection

---

## 4. Dependencies

| Dependency | Required | Notes |
|------------|----------|-------|
| Capstone (disassembler) | YES | Already available via IDisassembler |
| angr | NO | Too heavy, Python-based |
| LLVM | NO | Not needed for basic CFG |

**Conclusion**: CFG recovery is straightforward with existing disassembly output. The result will be a basic CFG (not full angr-style), suitable for pattern matching and further analysis.

---

## 5. Expected Output Format

```json
{
  "plugin": "Enhanced/CFG",
  "functions": [
    {
      "address": "0x1000",
      "name": "main",
      "blocks": [
        {
          "start": "0x1000",
          "end": "0x1020",
          "instructions": [...],
          "successors": ["0x1024", "0x1040"]
        }
      ],
      "edges": [
        {"from": 0x1000, "to": 0x1024, "type": "conditional"},
        {"from": 0x1000, "to": 0x1040, "type": "unconditional"}
      ]
    }
  ]
}
```

---

## 6. Citations

1. Zoller et al. "SMDA: Recursive Disassembler for Malware Analysis" (2019)
2. angr documentation: CFG Recovery (https://docs.angr.io/en/v9.2.81/analyses/cfg.html)
3. Binary Ninja documentation: MLIL and Function Recovery (https://docs.binary.ninja/dev/bn-arch-mlil.html)
4. LLVM documentation: MachineBasicBlock (https://llvm.org/docs/doxygen/classllvm_1_1MachineBasicBlock.html)

---

## 7. Recommendation

**Implement**: Basic CFG construction from disassembled instructions
- **Scope**: Block detection + edge resolution + simple function boundary detection
- **Limitations**: No indirect jump resolution, no jump tables
- **Effort**: ~300-400 LOC C++
- **Value**: Enables AST recovery, loop detection, pattern matching
