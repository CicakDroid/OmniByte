# Tahap 0 — Kontrak Dasar HydraDis (IDisassembler, IParser, IDecompiler)

**Tanggal:** 2026-09-02
**Status:** Draft untuk review
**Scope:** 3 contract headers yang menjadi prasyarat untuk semua backend adapter

---

## Context

HydraDis membutuhkan 7 backend adapter (Capstone, rizin-native, rz-ghidra, LIEF, Triton, Z3, CVC5). Semua backend ini butuh kontrak interface yang konsisten supaya:
1. Backend bisa di-swap tanpa mengubah caller
2. Factory pattern bisa jalan dengan interface yang seragam
3. Testing bisa pakai mock implementations

Tahap 0 mendefinisikan 3 kontrak dasar sebelum backend ditulis di atasnya.

---

## Contract 1: IDisassembler.h

**Path:** `engine-core/HydraDis/Disassembler/IDisassembler.h`
**Namespace:** `omnibyte::hydradis`

### Data Types

#### `DisassemblerArch` enum
```cpp
enum class DisassemblerArch {
    ARM,        // ARM 32-bit (ARM + Thumb mode)
    ARM64,      // AArch64
    x86,        // Intel x86 32-bit
    x86_64,     // Intel x86 64-bit
    MIPS,       // MIPS
    PPC,        // PowerPC
    SPARC,      // SPARC
    SystemZ,    // IBM System/z
    XCore,      // XCore
    M68K,       // Motorola 68000
    TMS320C64X, // TMS320C64x
    M680X,      // Motorola 68000 family
    EVM,        // Ethereum Virtual Machine
};
```

#### `Instruction`
```cpp
struct Instruction {
    uint64_t address = 0;       // virtual address
    uint16_t size = 0;          // panjang dalam byte
    std::string mnemonic;       // "mov", "bl", "ret"
    std::string opStr;          // "x0, #0x10"
    std::vector<uint8_t> bytes; // raw bytes (hex dump)
};
```

#### `DisassemblyResult`
```cpp
struct DisassemblyResult {
    bool success = false;
    std::string errorMessage;
    std::vector<Instruction> instructions;
    size_t totalBytes = 0;
};
```

### Interface

```cpp
class IDisassembler {
public:
    virtual ~IDisassembler() = default;
    virtual std::string name() const = 0;

    // Architecture yang di-disassemble oleh instance ini.
    // Dipilih saat construction, tidak berubah sepanjang lifetime.
    virtual DisassemblerArch arch() const = 0;

    virtual DisassemblyResult disassemble(
        const uint8_t* code, size_t codeSize,
        uint64_t baseAddr, size_t count = 0
    ) const = 0;
};
```

### Design Decisions

1. **Instance-per-arch (Opsi A):** Satu instance IDisassembler = satu arsitektur target. Caller tentukan arch saat construct backend via factory, bukan saat panggil `disassemble()`.

   Alasan:
   - Caller (Detector/Analyzer) sudah tahu arch dari ELF `e_machine` header via IParser SEBELUM panggil `disassemble()`
   - Backend Capstone butuh arch+mode saat `cs_open()`, tidak bisa diubah setelah
   - Cleaner API: tidak perlu pass arch di setiap panggilan `disassemble()`

2. **Arch-agnostic:** `mnemonic` dan `opStr` adalah string, bukan enum Capstone-specific. Backend Capstone yang convert `CS_INS_MOV` → `"mov"`, caller tidak perlu tahu.

3. **`bytes` field disimpan:** Untuk hex dump / re-encode. Optional (bisa kosong kalau overhead terlalu besar).

4. **`count` parameter default 0 (unlimited):** Useful untuk truncation di large binaries.

---

## Contract 2: IParser.h

**Path:** `engine-core/HydraDis/Parser/IParser.h`
**Namespace:** `omnibyte::hydradis`

### Data Types

#### `BinaryFormat` enum
```cpp
enum class BinaryFormat { ELF, PE, MachO, Unknown };
```

#### `SectionInfo`
```cpp
struct SectionInfo {
    std::string name;
    uint64_t virtualAddress = 0;
    uint64_t fileOffset = 0;
    uint64_t size = 0;
    uint32_t flags = 0;  // READ, WRITE, EXECUTE
};
```

#### `SymbolInfo`
```cpp
struct SymbolInfo {
    std::string name;
    uint64_t value = 0;
    uint64_t size = 0;
    uint32_t type = 0;    // FUNC, OBJECT, SECTION
    uint32_t binding = 0; // LOCAL, GLOBAL, WEAK
    int sectionIndex = -1;
};
```

#### `BinaryHeader`
```cpp
struct BinaryHeader {
    BinaryFormat format = BinaryFormat::Unknown;
    uint16_t machine = 0;
    uint64_t entryPoint = 0;
    uint64_t imageBase = 0;
    bool is64Bit = true;
    bool isEndianLittle = true;
};
```

#### `ParsedBinary`
```cpp
struct ParsedBinary {
    bool success = false;
    std::string errorMessage;
    BinaryHeader header;
    std::vector<SectionInfo> sections;
    std::vector<SymbolInfo> symbols;
};
```

### Interface

```cpp
class IParser {
public:
    virtual ~IParser() = default;
    virtual std::string name() const = 0;
    virtual ParsedBinary parseFile(const std::string& filePath) const = 0;
    virtual ParsedBinary parseBuffer(const uint8_t* data, size_t dataSize) const = 0;
};
```

### Design Decisions

1. **Format-agnostic:** `flags`, `type`, `binding` adalah `uint32_t` — biar bisa menampung ELF `SHF_WRITE`, PE `IMAGE_SCN_MEM_READ`, dll tanpa perlu enum gabungan.

2. **`parseFile` + `parseBuffer`:** Caller yang punya file pakai `parseFile`, caller yang sudah load bytes pakai `parseBuffer`.

---

## Contract 3: IDecompiler.h

**Path:** `engine-core/HydraDis/Decompiler/IDecompiler.h`
**Namespace:** `omnibyte::hydradis`

### Data Types

#### `DecompilerCapability` enum
```cpp
enum class DecompilerCapability {
    Light,  // cepat, ringan, auto-run ok (ESIL-based)
    Heavy   // lambat, berat, opt-in required (Ghidra-based)
};
```

#### `DecryptedFunction`
```cpp
struct DecompiledFunction {
    bool success = false;
    std::string errorMessage;
    uint64_t address = 0;
    std::string name;           // opsional, kosong kalau belum diketahui
    std::string pseudocode;     // C-like hasil decompilation
};
```

### Interface

```cpp
class IDecompiler {
public:
    virtual ~IDecompiler() = default;
    virtual std::string name() const = 0;
    virtual DecompilerCapability capability() const = 0;
    virtual DecompiledFunction decompile(
        const uint8_t* code, size_t codeSize, uint64_t baseAddr
    ) const = 0;
};
```

### Trade-off Decision: `vector<Instruction>` vs `raw bytes`

**Decided: raw bytes + baseAddr**

| Backend | Needs raw bytes? | Can use Instruction[]? |
|---------|-----------------|----------------------|
| rizin-native | ✅ Yes (ESIL re-disassembles) | ❌ No |
| rz-ghidra-adapter | ✅ Yes (feeds Ghidra decompiler) | ❌ No |

Both real backends require raw bytes. The "reuse IDisassembler output" optimization is currently impossible. If a future backend can use pre-disassembled instructions, we add an overload then (YAGNI).

---

## ISolverBackend.h Gap Analysis (untuk Z3 + CVC5)

Interface yang sudah ada di `engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/ISolverBackend.h`:

| Method | Z3 support | CVC5 support | Gap? |
|--------|-----------|-------------|------|
| `addConstraint(SMT-LIB2 string)` | ✅ native (`from_string()`) | ✅ via `assertFormula()` + parse | None |
| `check(timeoutMs)` | ✅ `check()` | ✅ `checkSat()` | None |
| `getModel()` | ✅ `get_model()` | ✅ `getModel()` | None |
| `push()/pop()` | ✅ `push()` / `pop()` | ✅ `push()` / `pop()` | None |

### Z3 C++ API Mapping

Source: https://z3prover.github.io/api/html/classz3_1_1solver.html

| ISolverBackend method | Z3 C++ API | Notes |
|----------------------|------------|-------|
| `addConstraint(smtLib2)` | `z3::solver::from_string(const char* s)` | Parses SMT-LIB2 string directly |
| `check(timeoutMs)` | `z3::solver::check()` | Returns `z3::check_result` (sat/unsat/unknown) |
| `getModel()` | `z3::solver::get_model()` | Returns `z3::model` |
| `push()` | `z3::solver::push()` | Creates backtracking point |
| `pop()` | `z3::solver::pop(unsigned n=1)` | Backtracks n levels |
| `reset()` | `z3::solver::reset()` | Removes all assertions |

### CVC5 C++ API Mapping

Source: https://cvc5.github.io/docs/cvc5-1.3.0/api/cpp/cpp.html
Source: https://cvc5.github.io/docs/cvc5-1.0.9/api/cpp/solver.html

| ISolverBackend method | CVC5 C++ API | Notes |
|----------------------|--------------|-------|
| `addConstraint(smtLib2)` | `cvc5::Solver::assertFormula(const Term&)` | Requires parsing SMT-LIB2 to Term first |
| `check(timeoutMs)` | `cvc5::Solver::checkSat()` | Returns `cvc5::Result` (sat/unsat/unknown) |
| `getModel()` | `cvc5::Solver::getModel(const vector<Sort>&, const vector<Term>&)` | Returns string representation |
| `push()` | `cvc5::Solver::push(uint32_t nscopes = 1)` | Pushes scope level |
| `pop()` | `cvc5::Solver::pop(uint32_t nscopes = 1)` | Pops scope level |
| `reset()` | `cvc5::Solver::resetAssertions()` | Removes all assertions |

### Conclusion

**No gaps found** — existing interface is sufficient for both Z3 and CVC5. Both solvers support:
- SMT-LIB2 constraint input (Z3 natively, CVC5 via Term parsing)
- Satisfiability checking with timeout support
- Model extraction after SAT result
- Push/pop scope management for path-sensitive analysis

---

## References

- **IEngineProfile.h** — pola interface yang diikuti (`modules/Dumper/DumperCore/IEngineProfile.h`)
- **IDumperEngine.h** — pola interface yang diikuti (`modules/Dumper/DumperCore/IDumperEngine.h`)
- **ISolverBackend.h** — kontrak existing untuk solver backend (`engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/ISolverBackend.h`)
- **Z3 C++ API** — https://z3prover.github.io/api/html/classz3_1_1solver.html
- **CVC5 C++ API** — https://cvc5.github.io/docs/cvc5-1.3.0/api/cpp/cpp.html
- **CVC5 Solver API** — https://cvc5.github.io/docs/cvc5-1.0.9/api/cpp/solver.html

---

## Status

- [x] IDisassembler.h — written (revised: added DisassemblerArch enum + arch() method)
- [x] IParser.h — written
- [x] IDecompiler.h — written
- [x] ISolverBackend.h Gap Analysis — verified with Z3/CVC5 documentation citations
- [ ] Awaiting user review & approval
