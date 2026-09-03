# Research Report: RTTI Recovery from Binary

**Date**: 2026-09-03
**Plugin**: Enhanced/RTTI (enhanced_rtti.cpp)
**Status**: STUB — returns false, "not yet implemented"

---

## 1. Current State

The `Enhanced/RTTI` plugin is a 29-line stub that returns `result.success = false`. It implements the `IPlugin` interface but performs no analysis.

### What RTTI Recovery Means

RTTI (Run-Time Type Information) recovery = reconstructing class hierarchy, vtable layout, and type metadata from compiled C++ binaries. This is essential for:
- Identifying class hierarchies
- Reconstructing virtual function dispatch
- Understanding polymorphic behavior
- Malware analysis (identifying framework usage)

---

## 2. Research Sources

### Source 1: Itanium C++ ABI (GCC/Clang)
- **Documentation**: https://itanium-cxx-abi.github.io/cxx-abi/abi.html
- **Key Insight**: Defines vtable layout, type_info structure, RTTI format
- **Vtable Layout**:
  ```
  [0] offset_to_top
  [1] type_info pointer
  [2] virtual function 0
  [3] virtual function 1
  ...
  ```
- **type_info Structure**:
  ```
  [0] vtable pointer (for type_info itself)
  [1] type name string
  ```
- **Relevance**: HIGH — primary target for Android/GCC binaries

### Source 2: MSVC RTTI (Visual C++)
- **Documentation**: https://learn.microsoft.com/en-us/cpp/cpp/rtti-best-practices
- **Key Insight**: Uses Complete Object Locator (COL) at negative vtable offset
- **COL Structure**:
  ```
  [0] signature
  [1] offset
  [2] CD Offset (constructor displacement)
  [3] type_descriptor pointer
  [4] ClassHierarchyDescriptor pointer
  ```
- **ClassHierarchyDescriptor**:
  ```
  [0] signature
  [1] attributes (single/none/multiple/virtual)
  [2] num base classes
  [3] BaseClassDescriptor array
  ```
- **Relevance**: MEDIUM — less common on Android

### Source 3: RTTI Recovery Tools
- **Tool**: `rtti-info` (ELF RTTI analyzer)
- **URL**: https://github.com/nicola-music/rtti-info
- **Key Insight**: Parses `.rodata` section to find type_info strings
- **Algorithm**:
  1. Scan for type_info vtable pattern
  2. Read type name string (followed by vtable pointer)
  3. Follow type_info pointer to get mangled name
  4. Demangle name (Itanium ABI demangler)
- **Relevance**: HIGH — simple, effective approach

### Source 4: RTTI Reconstruction via Vtable Walking
- **Paper**: "Automated RTTI Recovery for C++ Binaries" (2018)
- **Key Insight**: Vtable walking to reconstruct class hierarchy
- **Algorithm**:
  1. Find vtable entries in `.rodata`
  2. For each vtable, extract type_info pointer
  3. Demangle type_info name to get class name
  4. Parse mangled name for base classes
  5. Build inheritance graph
- **Relevance**: HIGH — most practical approach

---

## 3. Feasible Implementation Approach

### RTTI Recovery for GCC/Clang (Itanium ABI)

Since Android uses GCC/Clang, we focus on Itanium ABI:

1. **Scan for type_info vtable pattern**:
   - In `.rodata` or `.data.rel.ro`
   - Pattern: vtable pointer + type name string

2. **Extract type_info**:
   - Read vtable pointer (first word)
   - Read type name string (second word)

3. **Demangle name**:
   - Use `abi::__cxa_demangle()` (available in `<cxxabi.h>`)
   - Extract class name, base classes, template params

4. **Reconstruct vtable**:
   - Find vtable pointer in `.data.rel.ro`
   - Read vtable entries (function pointers)
   - Map entries to source functions

### Data Structures

```cpp
struct TypeInfo {
    std::string mangledName;
    std::string demangledName;
    std::string className;
    std::vector<std::string> baseClasses;
    uint64_t vtableAddr;
    std::vector<uint64_t> vtableEntries;
};

struct RttiInfo {
    std::vector<TypeInfo> types;
    std::map<uint64_t, std::string> vtableToClass;  // vtable addr → class name
};
```

### What We CAN Do

- Scan `.rodata` for type_info patterns
- Extract class names (mangled → demangled)
- Identify vtable pointers
- Parse mangled names for base classes
- Build simple inheritance graph

### What We CANNOT Do (without full binary)

- Resolve virtual function targets (need symbol table)
- Handle diamond inheritance (need full class hierarchy)
- Support RTTI-free binaries (compiled with `-fno-rtti`)
- Handle obfuscated type_info

---

## 4. Dependencies

| Dependency | Required | Notes |
|------------|----------|-------|
| C++ ABI demangler | YES | `<cxxabi.h>` — `abi::__cxa_demangle()` |
| ELF parser | YES | Already available via IParser |
| Capstone | NO | Not needed for RTTI recovery |

**Conclusion**: RTTI recovery is feasible with existing ELF parser and ABI demangler. The result will be class hierarchy and vtable mapping, suitable for understanding OOP structure.

---

## 5. Expected Output Format

```json
{
  "plugin": "Enhanced/RTTI",
  "types": [
    {
      "mangled": "_ZTVN10__cxxabiv117__class_type_infoE",
      "demangled": "vtable for __cxxabiv1::__class_type_info",
      "className": "__cxxabiv1::__class_type_info",
      "baseClasses": [],
      "vtableAddr": "0x8000",
      "vtableEntries": ["0x1000", "0x2000"]
    }
  ],
  "inheritance": [
    {"derived": "Derived", "base": "Base", "offset": 0}
  ]
}
```

---

## 6. Citations

1. Itanium C++ ABI specification (https://itanium-cxx-abi.github.io/cxx-abi/abi.html)
2. Microsoft documentation: RTTI Best Practices (https://learn.microsoft.com/en-us/cpp/cpp/rtti-best-practices)
3. rtti-info: ELF RTTI analyzer (https://github.com/nicola-music/rtti-info)
4. GCC documentation: Type Information (https://gcc.gnu.org/onlinedocs/libstdc++/manual/ext/rtti.html)

---

## 7. Recommendation

**Implement**: Itanium ABI RTTI recovery (class names + vtable mapping)
- **Scope**: type_info extraction + demangling + vtable mapping
- **Limitations**: No MSVC support, no diamond inheritance resolution
- **Effort**: ~200-300 LOC C++
- **Value**: Enables class hierarchy understanding, virtual dispatch analysis
