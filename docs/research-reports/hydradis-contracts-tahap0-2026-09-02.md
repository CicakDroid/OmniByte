# Tahap 0 — Kontrak Dasar HydraDis (IDisassembler, IParser, IDecompiler)

**Tanggal:** 2026-09-02
**Status:** Draft untuk review
**Scope:** 3 contract headers yang menjadi prasyarat untuk semua backend adapter

---

## Context

HydraDis membutuhkan 7 backend adapter (Capstone, rizin-native, rz-ghidra, LIEF, Triton, Z3, CVC5). Semua backend ini butuh kontrak interface yang konsisten supaya:
1. Backend bisa di-swap tanpa mengubah caller
2. Factory pattern bisa worked dengan统一 interface
3. Testing bisa pakai mock implementations

Tahap 0 mendefinisikan 3 kontrak dasar sebelum backend ditulis di atasnya.

---

## Contract 1: IDisassembler.h

**Path:** `engine-core/HydraDis/Disassembler/IDisassembler.h`
**Namespace:** `omnibyte::hydradis`

### Data Types

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
    virtual DisassemblyResult disassemble(
        const uint8_t* code, size_t codeSize,
        uint64_t baseAddr, size_t count = 0
    ) const = 0;
};
```

### Design Decisions

1. **Arch-agnostic:** `mnemonic` dan `opStr` adalah string, bukan enum Capstone-specific. Backend Capstone yang convert `CS_INS_MOV` → `"mov"`, caller tidak perlu tahu.

2. **`bytes` field disimpan:** Untuk hex dump / re-encode. Optional (bisa kosong kalau overhead terlalu besar).

3. **`count` parameter default 0 (unlimited):** Useful untuk truncation di large binaries.

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
| `addConstraint(SMT-LIB2 string)` | ✅ native | ✅ native | None |
| `check(timeoutMs)` | ✅ | ✅ | None |
| `getModel()` | ✅ | ✅ | None |
| `push()/pop()` | ✅ | ✅ | None |

**No gaps found** — existing interface is sufficient for both Z3 and CVC5.

---

## References

- **IEngineProfile.h** — pola interface yang diikuti (`modules/Dumper/DumperCore/IEngineProfile.h`)
- **IDumperEngine.h** — pola interface yang diikuti (`modules/Dumper/DumperCore/IDumperEngine.h`)
- **ISolverBackend.h** — kontrak existing untuk solver backend (`engine-core/HydraDis/Plugin/Enhanced/SymbolicExecution/ISolverBackend.h`)

---

## Status

- [x] IDisassembler.h — written
- [x] IParser.h — written
- [x] IDecompiler.h — written
- [ ] Awaiting user review & approval
