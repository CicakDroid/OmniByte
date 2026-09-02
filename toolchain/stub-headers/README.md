# Stub Headers — Local Syntax Verification Only

> **These headers are NOT a production replacement.**  
> They exist solely so `g++ -fsyntax-only` can type-check the four `.cpp` adapter files  
> without installing the full Z3 / CVC5 / Triton / Rizin libraries.

## Purpose

- Enable fast compile-check in CI or local dev before the real libraries are wired via `FetchContent`.
- Catch typos, missing method declarations, namespace mismatches early.

## Do NOT use for production

- **Do NOT add `-Itoolchain/stub-headers` to `CMakeLists.txt`** for production builds.
- **Do NOT link against these stubs** — they define no implementations.
- Production builds must use the real headers via `FetchContent` / `find_package`.

## Included libraries

| Library | Stub path | Official source | Version target |
|---------|-----------|----------------|----------------|
| Z3 | `z3++.h` | [Z3Prover/z3](https://github.com/Z3Prover/z3) | 4.13.0 |
| CVC5 | `cvc5/cvc5.h` | [cvc5/cvc5](https://github.com/cvc5/cvc5) | 1.2.0 |
| Triton | `triton/api.hpp`, `triton/archEnums.hpp`, `triton/cpuSize.hpp`, `triton/dllexport.hpp`, `triton/x8664Specifications.hpp` | [JonathanSalwan/Triton](https://github.com/JonathanSalwan/Triton) | 1.0 |
| Rizin | `rz_core.h`, `rz_analysis.h`, `rz_ghidra.h` | [rizinorg/rizin](https://github.com/rizinorg/rizin) | 0.7 |

## How to run the syntax check

```bash
g++ -std=c++17 -fsyntax-only \
    -Iengine-core/HydraDis \
    -Itoolchain/stub-headers \
    engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/solvers/z3-adapter/solvers_z3.cpp \
    engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/solvers/cvc5-adapter/solvers_cvc5.cpp \
    engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/engine/triton-adapter/symbolicexec_triton.cpp \
    engine-core/HydraDis/Decompiler/backends/rz-ghidra-adapter/decompiler_rz_ghidra.cpp
```

## Signature source policy

Every declaration in these stubs cites the official header file where the real signature lives.  
Signatures that were inferred from usage (not verified against the official header) are marked:

```cpp
// unverified signature, inferred from usage
```

Do not trust those blindly — verify against the real library when it becomes available.

## Unverified signatures

| File | Symbol | Status |
|------|--------|--------|
| `cvc5/cvc5.h` | `Solver::getInputFormula` | ⚠️ unverified — no public equivalent found in cvc5 1.2.0; may use internal API |
| `cvc5/cvc5.h` | `Solver::setTimeLimit` | ⚠️ unverified — real cvc5 may use `setOption("tlimit", ...)` instead |
| `rz_core.h` | `rz_core_block_read` | ⚠️ unverified — .cpp calls with 1 arg; official signature may differ |
| `triton/api.hpp` | `triton::MODE` enum | ⚠️ unverified — exact enum values unknown; stub provides `ALIGNED_MEMORY = 0` |
| `triton/x8664Specifications.hpp` | *(empty placeholder)* | ⚠️ header does not exist in official Triton; .cpp include path is incorrect |
