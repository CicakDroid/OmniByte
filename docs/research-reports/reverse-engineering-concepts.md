# Research Report: Custom Anti-Detection Specification & Architecture

**Date:** 2026-09-15
**Status:** Research Complete — Pending Implementation Specification

---

## Table of Contents

1. [Q1: Anti-Detection Maintenance Impact](#q1-anti-detection-maintenance-impact)
2. [Q2: ZigZag + ARTHook/PLT_GOTHook Pipeline](#q2-zigzag--arthookplt_gothook-pipeline)
3. [Q3: Sophisticated Memory Patching (KittyMemoryEx)](#q3-sophisticated-memory-patching-kittymemoryex)
4. [Q4: Root Requirement Analysis](#q4-root-requirement-analysis)
5. [Q5: ARTHook Anti-Detection Methods](#q5-arthook-anti-detection-methods)
6. [Q6: Non-Root /proc/pid/maps Bypass Mechanism](#q6-non-root-procpidmaps-bypass-mechanism)
7. [Next-To-Do: Specification & Schema](#next-to-do-specification--schema)

---

## Q1: Anti-Detection Maintenance Impact

### Answer: Low Maintenance Risk

Using adapter pattern, anti-detection layer is **isolated from shadowhook internals**:

```
ShadowhookAdapter (no changes) → AntiDetectionWrapper → IHookBackend
```

When shadowhook updates (e.g. v2.1.0):
- **ShadowhookAdapter** → No changes needed (only version number in header)
- **AntiDetectionWrapper** → No changes needed (no dependency on shadowhook internals)
- Only `getVersion()` return value changes

**Risk Level:** Low — anti-detection layer is fully decoupled.

---

## Q2: ZigZag + ARTHook/PLT_GOTHook Pipeline

### Architecture Diagram

```
┌─────────────────────────────────────────────────────┐
│                    HPTManager                       │
│         (inisialisasi & orchestration)              │
└──────────┬──────────────────────┬───────────────────┘
           │                      │
     ┌─────▼─────┐          ┌────▼──────┐
     │  Hooking   │          │ Memory    │
     │ Subsystem  │          │ Editing   │
     └─────┬──────┘          └───────────┘
           │
    ┌──────┴──────────────────────────────┐
    │          AntiDetection Layer         │
    │   (ZigZagEngine → patching rampung)  │
    └──┬──────────┬──────────┬────────────┘
       │          │          │
  ┌────▼───┐ ┌───▼────┐ ┌──▼──────┐
  │ ART    │ │ PLT/   │ │ Inline  │
  │ Hook   │ │ GOT    │ │ Hook    │
  │(Alba.) │ │(Bhook) │ │(Shadow.)│
  └────────┘ └────────┘ └─────────┘
```

### Backend Compatibility Matrix

| Backend | Anti-Detection Applicable? | Reasoning |
|---------|---------------------------|-----------|
| **ART Hook (Albatross)** | ⚠️ Partial | ART hook modifies method entry points. Maps hiding & permission switching useful, but trampoline encryption less relevant (ART uses different mechanism). |
| **PLT/GOT Hook (Bhook)** | ✅ Full | PLT/GOT hook modifies GOT entries. Anti-detection: hide GOT modification from memory scan, encrypt hook entries. |
| **Inline Hook (Shadowhook)** | ✅ Full (Most needed) | Inline hook is most vulnerable (modified prologue + RWX trampoline). All 3 techniques applicable. |

### Pipeline Flow

1. **HPTManager** → init ZigZag (load config stealth level)
2. **Hooking Subsystem** → select backend (ART / PLT-GOT / Inline)
3. **Before hook applied** → ZigZagEngine anti-detection layer:
   - Encrypt trampoline bytes
   - Patch `/proc/pid/maps` parsing
   - Setup permission switching
4. **Backend** → apply hook with protected trampoline
5. **After hook** → ZigZag monitor & maintain stealth state

---

## Q3: Sophisticated Memory Patching (KittyMemoryEx)

### Comparison: KittyMemory vs KittyMemoryEx

| Feature | KittyMemory (Basic) | KittyMemoryEx (Advanced) |
|---------|---------------------|--------------------------|
| **Read/Write** | ✅ Direct via `/proc/pid/mem` | ✅ + Multi-path fallback |
| **Permission** | Manual `mprotect` | ✅ Auto permission switching (RW→execute→restore) |
| **Integrity check** | ❌ | ✅ Detect memory scanning, re-patch if overwritten |
| **Encrypted patch** | ❌ | ✅ Patch bytes encrypted, decrypt only at runtime |
| **Backup & restore** | ❌ | ✅ Auto backup original bytes, restore on unhook |
| **Hook detection evasion** | ❌ | ✅ Scan patch locations, re-apply if anti-cheat removes them |

### Usage Example

```cpp
// Basic: direct byte write
kitty_memory_write(addr, patch_bytes, size);

// Sophisticated: write + encrypt + monitor
kittymemoryex_protect_write(addr, patch_bytes, size);  // auto permission
kittymemoryex_encrypt_inplace(addr, size);              // encrypt patch
kittymemoryex_monitor_start(addr, interval_ms);         // re-patch if overwritten
```

---

## Q4: Root Requirement Analysis

### All 3 Techniques Can Run Without Root

| Technique | Root Required? | Details |
|-----------|---------------|---------|
| **Encrypt trampoline** | ❌ No | In-process memory operations. User-space only. Works in non-root process. |
| **Dynamic permission switching** (RW→RX→RW) | ❌ No | `mprotect()` syscall. Process can change own memory permissions without root. |
| **Hook `/proc/pid/maps` parsing** | ⚠️ Depends | See Q6 for detailed explanation. |

**Summary:** All 3 techniques can run **without root** for basic defense. Root only needed for aggressive defense (kernel-level hiding).

---

## Q5: ARTHook Anti-Detection Methods

### How Albatross/ART Hooking Works

Based on AlbatrossAdapter source code analysis:

1. **Mechanism:** Albatross hooks via `java.lang.reflect.Method` objects, NOT inline instruction patching
2. **Core API:** `Albatross.backupAndHook(target, hook, backup)` (line 114-116 of AlbatrossAdapter.h)
3. **JNI Bridge:** AlbatrossAdapter translates address-based `IHookBackend` calls to method-based Albatross calls
4. **Method Resolution:** `resolveMethodFromAddress()` converts native address → `art::ArtMethod*` → JNI Method object

### ARTHook Detection Vectors

| Vector | Description | Detection Method |
|--------|-------------|------------------|
| **Entry point modification** | ART runtime modifies method entry point to redirect to hook | Scan `ArtMethod::entry_point_from_quick_compiled_code_` |
| **Method metadata modification** | Albatross may modify `access_flags_` or other ArtMethod fields | Compare method metadata snapshots |
| **JNI method registration** | Hook functions registered via JNI `RegisterNatives` | Scan JNI registration tables |
| **Thread state manipulation** | Some hooks need thread suspension for safe patching | Detect abnormal thread suspension patterns |

### ARTHook Anti-Detection Methods

#### Method 1: Entry Point Obfuscation (Recommended)

**Concept:** After Albatross redirects entry point, immediately obfuscate the redirect address.

```
Normal:    entry_point → compiled_code
Hooked:    entry_point → hook_function
Obfuscated: entry_point → encrypted_stub → hook_function
```

**Implementation:**
```cpp
// After Albatross backupAndHook():
uintptr_t entryPoint = getArtMethodEntryPoint(targetMethod);
uintptr_t hookAddr = getHookFunctionAddress();

// XOR encrypt the hook address with process-specific key
uintptr_t encryptedStub = encryptAddress(hookAddr, processKey);
setEncryptedTrampoline(entryPoint, encryptedStub);
```

**Advantage:** Anti-cheat scanning entry points sees encrypted value, not actual hook address.

#### Method 2: ArtMethod Metadata Restore

**Concept:** After hooking, restore original ArtMethod metadata to look unhooked.

```cpp
// Before hook: save original metadata
ArtMethodBackup saved = snapshotArtMethod(targetMethod);

// Apply hook via Albatross
albatross.backupAndHook(target, hook, backup);

// Restore visible metadata (keep entry_point redirect)
restoreArtMethodMetadata(targetMethod, saved);
// entry_point_ still points to hook, but other fields look original
```

**Advantage:** Memory scanners see "clean" ArtMethod structure.

#### Method 3: Hook Table Randomization

**Concept:** Randomize hook table layout to prevent pattern-based detection.

```cpp
// Instead of contiguous hook table:
// [hook1][hook2][hook3]  ← detectable pattern

// Use randomized layout:
// [hook3][padding][hook1][padding][hook2]  ← no pattern
```

**Advantage:** Anti-cheat can't scan for hook table patterns.

#### Method 4: JNI Registration Cloaking

**Concept:** Hide JNI native method registrations from detection.

```cpp
// Register hook as generic JNI method, not suspicious name
// Instead of: Java_com_omnibyte_hook_detour
// Use: Java_com_android_internal_os_Zygote_forkAndSpecialize
```

**Advantage:** JNI registration looks like legitimate Android framework method.

### Recommended ARTHook Anti-Detection Stack

| Priority | Method | Complexity | Effectiveness |
|----------|--------|------------|---------------|
| 1 | Entry Point Obfuscation | Medium | High |
| 2 | ArtMethod Metadata Restore | Medium | High |
| 3 | JNI Registration Cloaking | Low | Medium |
| 4 | Hook Table Randomization | High | Medium |

---

## Q6: Non-Root /proc/pid/maps Bypass Mechanism

### Understanding /proc/pid/maps

`/proc/pid/maps` is a virtual file that shows memory mappings of a process. Anti-cheat tools read this to detect:

- **RWX regions** (trampoline memory)
- **Unusual library mappings** (injected code)
- **Modified library sections** (hooked functions)

### How Anti-Cheat Reads /proc/pid/maps

```c
// Standard anti-cheat approach:
FILE* fp = fopen("/proc/self/maps", "r");
char line[256];
while (fgets(line, sizeof(line), fp)) {
    // Parse line: "7f8a000000-7f8a001000 rwxp 00000000 00:00 0 [anon:trampoline]"
    if (strstr(line, "rwxp") && !strstr(line, "[vdso]")) {
        // Found suspicious RWX region!
        report_hook_detected();
    }
}
fclose(fp);
```

### Non-Root Bypass: Hooking fopen()/read() in Own Process

**Key Insight:** `/proc/pid/maps` is read via standard POSIX I/O (`fopen`, `fgets`, `read`). If we hook these functions **in our own process**, we control what anti-cheat sees.

#### Mechanism

```
Anti-cheat code calls fopen("/proc/self/maps")
    ↓
Our hook intercepts fopen()
    ↓
If path == "/proc/self/maps" OR "/proc/self/task/*/maps":
    → Return modified maps data (filtered)
Else:
    → Call original fopen()
```

#### Implementation Flow

```
┌─────────────────────────────────────────────┐
│           Anti-Cheat Detection              │
│    (reads /proc/self/maps for RWX regions)  │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────┐
│         fopen() Hook (in own process)       │
│  1. Intercept fopen("/proc/self/maps")      │
│  2. Redirect to custom maps_generator()     │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────┐
│         maps_generator()                    │
│  1. Read REAL /proc/self/maps               │
│  2. Filter out:                             │
│     - RWX regions (trampoline memory)       │
│     - [anon:shadowhook_*] regions           │
│     - Injected library mappings             │
│  3. Return modified maps string             │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────┐
│         Anti-Cheat Receives                 │
│    "Clean" maps without hook evidence       │
└─────────────────────────────────────────────┘
```

#### Code Example (Conceptual)

```c
// fopen hook - filter /proc/pid/maps
FILE* hooked_fopen(const char* path, const char* mode) {
    if (is_maps_path(path)) {
        // Generate fake maps content
        char* filtered_maps = generate_clean_maps();
        // Return as FILE* via tmpfile + write
        FILE* tmp = tmpfile();
        fwrite(filtered_maps, 1, strlen(filtered_maps), tmp);
        rewind(tmp);
        return tmp;
    }
    return original_fopen(path, mode);
}

// read hook - filter maps reads
ssize_t hooked_read(int fd, void* buf, size_t count) {
    if (fd_is_maps_fd(fd)) {
        // Return filtered content
        return generate_clean_maps_chunk(buf, count);
    }
    return original_read(fd, buf, count);
}
```

### Why This Works Without Root

1. **Process-internal hooking:** We're hooking functions **in our own process space**, not modifying other processes
2. **No kernel modification:** `fopen()`/`read()` are user-space POSIX functions
3. **Self-defense:** The process is protecting itself from its own anti-cheat code
4. **No privilege escalation:** Standard function hooking (can use PLT/GOT or inline hook on own libc)

### Limitations

| Limitation | Description | Mitigation |
|------------|-------------|------------|
| **Kernel-level reads** | Kernel can still see real maps via syscalls | Use kernel-level hook (requires root) |
| **Other processes** | Can't hide from other processes' reads | Only affects own process |
| **Timing attacks** | Anti-cheat could read maps before hook is applied | Hook maps reader early in initialization |

---

## Next-To-Do: Specification & Schema

### 1. ZigZag Anti-Detection Module Specification

**File:** `runtime/ZigZag/AntiDetection/`

```
runtime/ZigZag/AntiDetection/
├── AntiDetectionEngine.h        # Main orchestrator
├── AntiDetectionEngine.cpp
├── strategies/
│   ├── IAntiDetectionStrategy.h  # Strategy interface
│   ├── TrampolineEncryptor.h     # Encrypt trampoline bytes
│   ├── TrampolineEncryptor.cpp
│   ├── PermissionSwitcher.h      # Dynamic RW→RX→RW
│   ├── PermissionSwitcher.cpp
│   ├── MapsCloak.h              # /proc/pid/maps filter
│   ├── MapsCloak.cpp
│   └── ARTMetadataRestore.h     # ArtMethod metadata restore
│       ARTMetadataRestore.cpp
├── config/
│   ├── StealthConfig.h          # Stealth level configuration
│   └── StealthConfig.cpp
└── CMakeLists.txt
```

### 2. Stealth Levels

```cpp
enum class StealthLevel {
    OFF = 0,          // No anti-detection
    BASIC = 1,        // Permission switching only
    STANDARD = 2,     // Permission switching + maps cloaking
    AGGRESSIVE = 3,   // All techniques + ART metadata restore
    PARANOID = 4      // All + kernel-level hooks (requires root)
};
```

### 3. Backend Integration Schema

```cpp
// IHookBackend extension (optional anti-detection)
class IHookBackend {
public:
    // ... existing interface ...

    // Optional: enable anti-detection for this backend
    virtual bool enableAntiDetection(StealthLevel level) { return false; }

    // Optional: get anti-detection status
    virtual AntiDetectionStatus getAntiDetectionStatus() const {
        return AntiDetectionStatus::DISABLED;
    }
};
```

### 4. Maps Cloak Configuration

```cpp
struct MapsCloakConfig {
    bool hide_rwx_regions = true;
    bool hide_shadowhook_regions = true;
    bool hide_injected_libraries = true;
    bool hide_anonymous_regions = false;
    std::vector<std::string> custom_hide_patterns = {
        "[anon:shadowhook_*]",
        "[anon:trampoline]"
    };
};
```

### 5. Implementation Priority

| Phase | Component | Complexity | Status |
|-------|-----------|------------|--------|
| Phase 1 | `MapsCloak` (fopen/read hook) | Medium | **NEXT TODO** |
| Phase 2 | `PermissionSwitcher` (mprotect) | Low | **NEXT TODO** |
| Phase 3 | `TrampolineEncryptor` (XOR/AES) | Medium | **NEXT TODO** |
| Phase 4 | `ARTMetadataRestore` (Albatross) | High | **NEXT TODO** |
| Phase 5 | `StealthConfig` (runtime config) | Low | **NEXT TODO** |

### 6. Testing Strategy

| Test Case | Expected Result |
|-----------|-----------------|
| Hook shadowhook + MapsCloak | Anti-cheat sees clean maps |
| Hook ART method + MetadataRestore | Memory scanner sees clean ArtMethod |
| PermissionSwitcher during hook | No RWX region visible |
| All strategies combined | Full stealth mode |

---

## References

- [bytedance/android-inline-hook](https://github.com/bytedance/android-inline-hook) — Shadowhook library
- [AlbatrossHook/AlbatrossAndroid](https://github.com/AlbatrossHook/AlbatrossAndroid) — ART method hooking
- [KittyMemory](https://github.com/MarsCrafts/KittyMemory) — Memory patching library
- Android AOSP: `art/runtime/art_method.h` — ArtMethod structure
- Linux kernel: `/proc/pid/maps` documentation

---

**Document created:** 2026-09-15
**Author:** Sisyphus (OhMyOpenCode)
**Tags:** #anti-detection #zigzag #hooking #arthook #inlinehook #specification #todo
