# Tahap 1-4 — 7 Backend Adapters HydraDis

**Tanggal:** 2026-09-02
**Status:** Implementation complete
**Scope:** CMake wiring + wrapper implementations untuk 7 backend adapter

---

## Summary

| Backend | Type | Library | Version | CMake Target |
|---------|------|---------|---------|--------------|
| Capstone | Disassembler | capstone-engine/capstone | v5.0.9 | capstone-static |
| rizin-native | Decompiler | subprocess (no link) | - | - |
| rz-ghidra | Decompiler | rizinorg/rz-ghidra | v0.9.0 | rz_ghidra |
| LIEF | Parser | lief-project/LIEF | v1.0.0 | LIEF::LIEF |
| Triton | SymbolicExec | JonathanSalwan/Triton | v0.9 | triton |
| Z3 | Solver | Z3Prover/z3 | z3-4.13.0 | libz3 |
| CVC5 | Solver | cvc5/cvc5 | cvc5-1.3.4 | cvc5 |

---

## Backend 1: Capstone Adapter

**Path:** `engine-core/HydraDis/Disassembler/backends/capstone-adapter/`
**Implements:** `IDisassembler.h`

### CMakeLists.txt
- FetchContent capstone v5.0.9
- Static build, all architectures enabled (ARM, ARM64, x86, MIPS, PPC, SPARC, SystemZ, XCore, M68K, TMS320C64X, M680X, EVM)

### Implementation
- `CapstoneDisassembler` class implementing `IDisassembler`
- Constructor takes `DisassemblerArch`, maps to `cs_arch` + `cs_mode`
- `cs_open()` in constructor, `cs_close()` in destructor
- `disassemble()` calls `cs_disasm()`, maps results to `Instruction` struct

---

## Backend 2: Rizin Native (Subprocess)

**Path:** `engine-core/HydraDis/Decompiler/backends/rizin-native/`
**Implements:** `IDecompiler.h`

### CMakeLists.txt
- No library linkage
- Subprocess approach via `popen("rizin -j")`
- Rizin binary must be available in PATH at runtime

### Implementation
- `RizinNativeDecompiler` class implementing `IDecompiler`
- `capability()` returns `Light` (fast, ESIL-based)
- `decompile()` executes `rizin -q -c "pd {count} @ {addr}"` via popen
- Captures stdout as decompilation output

---

## Backend 3: rz-ghidra Adapter

**Path:** `engine-core/HydraDis/Decompiler/backends/rz-ghidra-adapter/`
**Implements:** `IDecompiler.h`

### CMakeLists.txt
- FetchContent rizin v0.9.1 + rz-ghidra v0.9.0
- Builds rizin as library, then rz-ghidra linking against it

### Implementation
- `RzGhidraDecompiler` class implementing `IDecompiler`
- `capability()` returns `Heavy` (slow, Ghidra-based, opt-in required)
- Uses rizin API to load binary and invoke Ghidra decompiler

---

## Backend 4: LIEF Parser

**Path:** `engine-core/HydraDis/Parser/backends/lief-adapter/`
**Implements:** `IParser.h`

### CMakeLists.txt
- FetchContent LIEF v1.0.0
- ELF-only build (PE/MachO disabled)

### Implementation
- `LiefParser` class implementing `IParser`
- `parseFile()` uses `LIEF::Parser::parse(filePath)`
- `parseBuffer()` uses `LIEF::Parser::parse(vector<uint8_t>)`
- Maps ELF sections to `SectionInfo`, symbols to `SymbolInfo`

---

## Backend 5: Triton Adapter

**Path:** `engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/engine/triton-adapter/`
**Implements:** Symbolic execution engine

### CMakeLists.txt
- FetchContent Triton v0.9
- Python bindings disabled

### Implementation
- `TritonSymbolicExec` class wrapping `triton::API`
- `liftInstruction()` processes bytes through Triton's symbolic engine
- `getFullSMTLib2()` exports all symbolic expressions as SMT-LIB2
- Delegates solving to ISolverBackend (Z3 or CVC5)

---

## Backend 6: Z3 Solver

**Path:** `engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/solvers/z3-adapter/`
**Implements:** `ISolverBackend.h`

### CMakeLists.txt
- FetchContent Z3 z3-4.13.0
- Python/Java bindings disabled

### Implementation
- `Z3Solver` class implementing `ISolverBackend`
- `addConstraint()` uses `z3::solver::from_string()` for SMT-LIB2
- `check()` calls `z3::solver::check()` with timeout
- `getModel()` extracts assignments from `z3::model`
- `push()/pop()` for scope management

---

## Backend 7: CVC5 Solver

**Path:** `engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/solvers/cvc5-adapter/`
**Implements:** `ISolverBackend.h`

### CMakeLists.txt
- FetchContent CVC5 cvc5-1.3.4
- Python/Java bindings disabled

### Implementation
- `CVC5Solver` class implementing `ISolverBackend`
- `addConstraint()` parses SMT-LIB2 via `solver_.getInputFormula()`
- `check()` calls `solver_.checkSat()` with timeout
- `getModel()` extracts model via `solver_.getModel()`
- `push()/pop()` for scope management

---

## File Manifest

| File | Status |
|------|--------|
| `engine-core/HydraDis/Disassembler/backends/capstone-adapter/CMakeLists.txt` | ✅ Written |
| `engine-core/HydraDis/Disassembler/backends/capstone-adapter/disassembler_capstone.cpp` | ✅ Written |
| `engine-core/HydraDis/Decompiler/backends/rizin-native/CMakeLists.txt` | ✅ Written |
| `engine-core/HydraDis/Decompiler/backends/rizin-native/decompiler_rizin_native.cpp` | ✅ Written |
| `engine-core/HydraDis/Decompiler/backends/rz-ghidra-adapter/CMakeLists.txt` | ✅ Written |
| `engine-core/HydraDis/Decompiler/backends/rz-ghidra-adapter/decompiler_rz_ghidra.cpp` | ✅ Written |
| `engine-core/HydraDis/Parser/backends/lief-adapter/CMakeLists.txt` | ✅ Written |
| `engine-core/HydraDis/Parser/backends/lief-adapter/parser_lief.cpp` | ✅ Written |
| `engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/engine/triton-adapter/CMakeLists.txt` | ✅ Written |
| `engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/engine/triton-adapter/symbolicexec_triton.cpp` | ✅ Written |
| `engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/solvers/z3-adapter/CMakeLists.txt` | ✅ Written |
| `engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/solvers/z3-adapter/solvers_z3.cpp` | ✅ Written |
| `engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/solvers/cvc5-adapter/CMakeLists.txt` | ✅ Written |
| `engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/solvers/cvc5-adapter/solvers_cvc5.cpp` | ✅ Written |
| `docs/research-reports/hydradis-7-backends-tahap1-4-2026-09-02.md` | ✅ Written |
