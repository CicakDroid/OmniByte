# DumperCore Component Logic — Research Report
**Date**: 2026-09-01
**Batch**: A (DumperCore)
**Status**: Research complete, ready for implementation

---

## 1. SharedUtils (`dumper_core_sharedutils.cpp`)

### Contract (from codebase)
No header file exists — this is a utility `.cpp` providing free functions to all DumperCore/Engine modules. Current stub: `dumper_core_sharedutils_placeholder_init()`.

### Required Functions (derived from engine usage patterns)
| Function | Purpose | Used By |
|----------|---------|---------|
| `readFileBytes(path)` | Read entire file into `vector<uint8_t>` | All Analyzers (file targets) |
| `readProcessMemory(pid, addr, out, size)` | Read `/proc/pid/mem` at address | UnityMonoResolver (line 38-49), all Resolvers |
| `hexEncode(data, len)` | Convert bytes → hex string | StringEntry.address formatting |
| `hexDecode(hexStr)` | Convert hex string → bytes | AOB pattern parsing |
| `base64Encode(data, len)` | Standard base64 | Metadata export |
| `base64Decode(str)` | Standard base64 decode | — (future use) |
| `trim(str)` | Trim whitespace | Version string cleanup |
| `startsWith(str, prefix)` | Prefix check | Engine detection (library names) |
| `scanPattern(base, size, pattern, mask)` | AOB pattern scanner | UE Resolver (GNames/GObjects/GWorld) |
| `isPageAligned(addr)` | Check page alignment | MemoryIO validation |

### Open-Source References
- **Pattern scanning**: IDA/Ghidra-style AOB scan with mask — standard RE technique
- **`/proc/pid/mem` read**: Standard Linux process memory read (used by frida, reclass, etc.)
- **Base64**: RFC 4648 standard implementation
- No Android-specific dependencies needed — all pure C++17 + POSIX

### Implementation Notes
- File I/O: use `std::ifstream` binary mode + `std::istreambuf_iterator`
- Memory read: `fopen("/proc/pid/mem", "rb")` + `fseek` + `fread` (same pattern as UnityMonoResolver line 38-49)
- Pattern scan: simple byte-by-byte comparison with mask — no need for SIMD on ARM64 for dump-sized data

---

## 2. Detector (`dumper_core_detector.cpp`)

### Contract (from codebase)
No header file — this is the orchestrator that ties together EngineRegistry. The actual `detect()` logic lives in each engine's `*Engine.h` (all currently return empty `DetectionResult{}`). The Detector's job is to:
1. Accept an `AnalysisTarget`
2. Call `EngineRegistry::instance().detectBestMatch(target)`
3. Return the `MatchResult`

### Detection Signatures Per Engine (from `*Engine.h` comments)

| Engine | Detection Signal | Confidence Source |
|--------|-----------------|-------------------|
| **UnrealEngine** | `.pak` magic `0x5A6F12E1`, `"UE4"`/`"UE5"` string in binary, `GEngineVersion` struct | High if magic matches; medium if string-only |
| **UnityIL2CPP** | `libil2cpp.so` in `lib/<abi>/`, `global-metadata.dat` magic `0xAF1BB1FA`, version field in metadata header | High if both lib + metadata; medium if lib-only |
| **UnityMono** | `libmono.so` / `libmonobdwgc-2.0.so`, `Assembly-CSharp.dll`, symbol `mono_get_root_domain` | High if lib + dll; medium if lib-only |
| **Godot** | `.pck` footer magic `"GDPC"`, `.godot` project file, exe name `godot_linux`/`godot4` | High if magic; medium if project file |
| **GameMaker** | `data.win` magic `"YYYG"` (GM2.3+) or `"FORM"` (GMS1), `yoyorun` library | High if magic; medium if library |
| **Source2** | `.vpk_c` resource compiler format (different from Source 1 `.vpk`) | High if magic |
| **Cocos2d** | `libcocos2dlua.so` / `libcocos2djs.so` / `libgame.so` / `libcocos.so`, `org.cocos2dx.*` DEX classes | High if lib name match; medium if DEX classes |

### File vs Process Detection
- **File targets**: Check file extension, read magic bytes from file header
- **Process targets**: Check loaded modules from `/proc/pid/maps`, read memory at known offsets

### Implementation Notes
- The `detect()` implementations are in each `*Engine.h` — the Detector `.cpp` provides the orchestration + file reading helpers
- Each engine's detect() returns `{matched, confidence, detectedVersion}` — Detector doesn't need engine-specific logic
- Confidence thresholds: `>= 0.8` = strong match, `0.5-0.8` = possible match, `< 0.5` = weak/unknown

---

## 3. EngineRegistry (`dumper_core_engineregistry.cpp`)

### Contract (from `EngineRegistry.h`)
```cpp
class EngineRegistry {
public:
    static EngineRegistry& instance();                          // Singleton
    void registerEngine(shared_ptr<IDumperEngine> engine);      // Register engine
    optional<MatchResult> detectBestMatch(const AnalysisTarget&) const;  // Detect best
    vector<shared_ptr<IDumperEngine>> allEngines() const;       // List all
private:
    vector<shared_ptr<IDumperEngine>> engines_;
};
```

### `detectBestMatch()` Algorithm
1. Iterate all registered engines, call `engine->detect(target)` on each
2. Filter to engines where `detection.matched == true`
3. Sort by `detection.confidence` descending
4. If top engine has confidence ≥ 0.1 above second-best → return as `best`
5. If top 2+ engines within 0.1 confidence → return all as `allCandidates` (UI can prompt user)
6. If no engines matched → return `nullopt`

### Singleton Pattern
```cpp
EngineRegistry& EngineRegistry::instance() {
    static EngineRegistry reg;
    return reg;
}
```

### Implementation Notes
- Simple vector storage — no map needed (7 engines max)
- Registration happens at app startup (or first dump request)
- The `registerEngine()` call is idempotent — same engine type shouldn't be registered twice

---

## 4. ResultNormalizer (`dumper_core_resultnormalizer.cpp`)

### Contract (no header — free functions)
Merges and normalizes `DumpResult` from one or more engines into a single canonical result.

### Required Functions
| Function | Purpose |
|----------|---------|
| `mergeResults(vector<DumpResult>)` | Merge multiple engine results into one |
| `normalizeTypes(vector<TypeEntry>)` | Dedup types by name, merge interfaces |
| `normalizeMethods(vector<MethodEntry>)` | Dedup methods by (declaringType + name) |
| `normalizeFields(vector<FieldEntry>)` | Dedup fields by (declaringType + name) |
| `normalizeStrings(vector<StringEntry>)` | Dedup strings by value |
| `canonicalTypeName(string)` | Map engine-specific names to canonical (e.g., `"FString"` → `"string"`) |

### Normalization Rules
1. **Type dedup**: Two `TypeEntry` with same `name` → keep the one with more data (non-zero address/size wins)
2. **Method dedup**: Same `(declaringType, name)` → keep the one with resolved address
3. **Field dedup**: Same `(declaringType, name)` → keep the one with non-zero offset
4. **String dedup**: Same `value` → keep first occurrence
5. **Type name canonicalization**: Engine-specific → generic (optional, for cross-engine export)

### Implementation Notes
- Uses `std::unordered_set` or `std::unordered_map` for O(1) dedup
- No external dependencies — pure C++17 standard library

---

## Summary: Implementation Order

| Order | File | Lines (est.) | Complexity | Dependencies |
|-------|------|-------------|------------|--------------|
| 1 | `dumper_core_sharedutils.cpp` | ~120 | Low | None (pure utils) |
| 2 | `dumper_core_engineregistry.cpp` | ~60 | Low | IDumperEngine.h, AnalysisTarget.h |
| 3 | `dumper_core_detector.cpp` | ~50 | Low | EngineRegistry.h (calls detectBestMatch) |
| 4 | `dumper_core_resultnormalizer.cpp` | ~100 | Medium | DumpResult.h (dedup logic) |

Total estimated: ~330 lines across 4 files.
