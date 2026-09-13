# Tahap 0 — IPlugin.h Contract Design

**Date**: 2026-09-02
**Status**: DRAFT — awaiting user review before implementation

---

## 1. Current State Analysis

### Existing Interfaces (reference pattern)
- `IDisassembler.h` — `DisassemblyResult` with `Instruction[]`
- `IParser.h` — `ParsedBinary` with `BinaryHeader`, `SectionInfo[]`, `SymbolInfo[]`
- `IDecompiler.h` — `DecompiledFunction` with `pseudocode` string

### Existing Plugin Files (current status)

| Subfolder | File | Status | Notes |
|-----------|------|--------|-------|
| Deobfuscate | `plugin_deobfuscate.cpp` | Placeholder | `extern "C"` stub only |
| Deobfuscate/DexKit | `DexKitAdapter.cpp/.h` | **Written** | Real code, JNI bridge stubs |
| Deobfuscate/hrtng | `HrtngDeob.cpp/.h` | **Written** | Real code, has bug: line 120 `HrtngEncrypt` → `HrtngDecrypt` |
| ScriptHooks/Loader | `plugin_scripthooks_loader.cpp` | Placeholder | `extern "C"` stub only |
| ScriptHooks/Runner | `plugin_scripthooks_runner.cpp` | Placeholder | `extern "C"` stub only |
| Enhanced/AST | `enhanced_ast.cpp` | Placeholder | `extern "C"` stub only |
| Enhanced/CFG | `enhanced_cfg.cpp` | Placeholder | `extern "C"` stub only |
| Enhanced/FunctionResolver | `enhanced_functionresolver.cpp` | Placeholder | `extern "C"` stub only |
| Enhanced/RTTI | `enhanced_rtti.cpp` | Placeholder | `extern "C"` stub only |
| Enhanced/FindCrypt | `enhanced_findcrypt.cpp` | Placeholder | `extern "C"` stub only |
| Enhanced/FindCrypt/FindCrypt3 | `FindCrypt3.cpp/.h` | **Written** | Real code, crypto constant scanner |
| Enhanced/Emulation | `enhanced_emulation.cpp` | Placeholder | `extern "C"` stub only |
| Enhanced/SymbolicExecution | (solvers, triton) | **Written** | Already fixed in prior commits |

---

## 2. Bug Found During Audit

**File**: `Deobfuscate/hrtng/HrtngDeob.cpp`, line 120
**Bug**: `HrtngEncrypt::encodeBase64` — class `HrtngEncrypt` does not exist. Should be `HrtngDecrypt::encodeBase64`.
**Impact**: Compile error when building hrtng module.
**Fix**: Change `HrtngEncrypt` → `HrtngDecrypt` on line 120.

---

## 3. IPlugin.h Draft Design

### Design Principles (matching existing interfaces)
1. **Minimal**: Only essential lifecycle methods — no anti-patterns
2. **Context-passing**: PluginContext carries data from all three backends (parser, disassembler, decompiler) so plugins can consume any combination
3. **Return-code pattern**: Same `success`/`errorMessage` pattern as `DisassemblyResult`, `ParsedBinary`, `DecompiledFunction`
4. **No ownership transfer**: Context holds non-owning pointers/references; plugins do not take ownership

### PluginContext Design

```cpp
struct PluginContext {
    // From IParser (binary metadata)
    const ParsedBinary* binary = nullptr;      // nullable: plugin may not need parser output

    // From IDisassembler (disassembly)
    const DisassemblyResult* disassembly = nullptr;  // nullable

    // From IDecompiler (decompilation)
    const DecompiledFunction* decompiled = nullptr;   // nullable

    // Target function address (common reference point for all plugins)
    uint64_t functionAddress = 0;

    // Output path for any generated artifacts
    std::string outputDir;
};
```

**Why nullable pointers?** Not every plugin needs all three backend outputs:
- `CFG` needs `disassembly` only
- `AST` needs `decompiled` only
- `FunctionResolver` needs `disassembly` + `binary` (symbols)
- `RTTI` needs `binary` (symbols) + `disassembly`
- `Deobfuscate` needs `decompiled` or `disassembly` depending on technique

### PluginResult Design

```cpp
struct PluginResult {
    bool success = false;
    std::string errorMessage;

    // Plugin-specific structured output (opaque to caller)
    // Each plugin defines its own output type internally
    void* pluginData = nullptr;

    // Human-readable summary for logging
    std::string summary;
};
```

### IPlugin Interface

```cpp
class IPlugin {
public:
    virtual ~IPlugin() = default;

    // Plugin identity
    virtual std::string name() const = 0;
    virtual std::string version() const = 0;

    // Lifecycle
    virtual bool onLoad() = 0;           // one-time init, return false if unavailable
    virtual PluginResult onRun(const PluginContext& ctx) = 0;
    virtual void onUnload() = 0;
};
```

### Why this design?

| Decision | Rationale |
|----------|-----------|
| `onLoad()` returns bool | Some plugins need external deps (Unicorn, DexKit JNI). `onLoad()` lets them fail gracefully if unavailable. |
| `onRun()` takes const ref | Plugins read context, don't modify it. Multiple plugins can run on same context. |
| `onUnload()` is void | Cleanup always succeeds (or we crash — same as destructor). |
| `pluginData` is `void*` | Avoids template complexity. Each plugin casts to its own output type. Caller uses `static_cast` only when it knows the plugin type. |
| No `enabled`/`priority` flags | Over-engineering. Caller decides which plugins to run. |
| No registration system | Plugins are instantiated directly via factory functions. No global registry magic. |

---

## 4. Files to Create/Modify

### New files
- `engine-core/HydraDis/Plugin/IPlugin.h` — Interface + PluginContext + PluginResult

### Files to modify (implement IPlugin)
- `Deobfuscate/plugin_deobfuscate.cpp` — Full rewrite implementing IPlugin
- `ScriptHooks/Loader/plugin_scripthooks_loader.cpp` — Full rewrite implementing IPlugin
- `ScriptHooks/Runner/plugin_scripthooks_runner.cpp` — Full rewrite implementing IPlugin
- `Enhanced/AST/enhanced_ast.cpp` — Full rewrite implementing IPlugin
- `Enhanced/CFG/enhanced_cfg.cpp` — Full rewrite implementing IPlugin
- `Enhanced/FunctionResolver/enhanced_functionresolver.cpp` — Full rewrite implementing IPlugin
- `Enhanced/RTTI/enhanced_rtti.cpp` — Full rewrite implementing IPlugin
- `Enhanced/FindCrypt/enhanced_findcrypt.cpp` — Full rewrite implementing IPlugin
- `Enhanced/Emulation/enhanced_emulation.cpp` — Full rewrite implementing IPlugin

### Files to fix (bug)
- `Deobfuscate/hrtng/HrtngDeob.cpp` line 120: `HrtngEncrypt` → `HrtngDecrypt`

---

## 5. Open Questions for User Review

1. **`void* pluginData`** — Is this acceptable, or do you prefer a type-erased wrapper (`std::any`)? `void*` is simpler for C++17 but less safe.

2. **`onLoad()` on every plugin** — Some plugins (CFG, AST) are pure logic with no external deps. Should they still implement `onLoad()` returning true, or should `onLoad()` be optional with a default implementation?

3. **Plugin discovery** — Currently I'm planning direct instantiation (no registry). If you want runtime plugin discovery later, we'd add a `REGISTER_PLUGIN()` macro. But that's YAGNI for now. Correct?

4. **HrtngDeob bug** — I'll fix the `HrtngEncrypt` → `HrtngDecrypt` typo as part of this batch. OK?
