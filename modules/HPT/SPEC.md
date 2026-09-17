# HPT — Hooking Platform Toolkit Specification

## Overview

HPT (Hooking Platform Toolkit) is the orchestrator module for function hooking and memory editing in OmniByte. It provides a unified API over multiple hooking backends and memory editors, selecting the best available tool for each operation.

**Namespace:** `omnibyte::runtime`
**Location:** `modules/HPT/`

---

## Architecture

```
HPT (Orchestrator)
├── Hooking Subsystem
│   └── IHookBackend (interface)
│       ├── AlbatrossAdapter    — ART method hooking (Albatross v3.6.0)
│       ├── BhookAdapter        — PLT/GOT hooking (bytedance/bhook v1.1.2)
│       ├── ShadowhookAdapter   — Inline hooking (bytedance/android-inline-hook v2.0.1)
│       ├── VectorhookAdapter   — Traceless hooking (stealth-core kpmhook v0.1.0)
│       ├── SQLhookAdapter      — Database function hooking (SQLite)
│       └── NetworkPacketHookAdapter — Network function hooking (dlsym + inline)
└── MemoryEditing Subsystem
    └── IMemoryEditor (interface)
        ├── KittyMemoryAdapter  — In-process memory (KittyMemory)
        └── KittyMemoryExAdapter — Cross-process memory (KittyMemoryEx)
```

---

## Core Interfaces

### IHookBackend

```cpp
namespace omnibyte::runtime::backends {

class IHookBackend {
public:
    virtual ~IHookBackend() = default;
    virtual std::string name() const = 0;
    virtual bool isAvailable() const = 0;
    virtual bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) = 0;
    virtual bool unhook(uintptr_t addr) = 0;
    virtual bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) = 0;
};

} // namespace omnibyte::runtime::backends
```

### IMemoryEditor

```cpp
namespace omnibyte::runtime::backends {

class IMemoryEditor {
public:
    virtual ~IMemoryEditor() = default;
    virtual const char* name() const = 0;
    virtual bool isAvailable() const = 0;
    virtual bool readMemory(uintptr_t addr, void* buffer, size_t size) = 0;
    virtual bool writeMemory(uintptr_t addr, const void* data, size_t size) = 0;
    virtual bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) = 0;
    virtual bool dumpMemory(uintptr_t addr, size_t size, const std::string& destPath) = 0;
};

} // namespace omnibyte::runtime::backends
```

---

## Backend Reference

### 1. Albatross (ART Method Hook)

| Property | Value |
|----------|-------|
| **Adapter** | `AlbatrossAdapter` |
| **Source** | https://github.com/AlbatrossHook/AlbatrossAndroid |
| **License** | Apache-2.0 |
| **Version** | 3.6.0 |
| **Mechanism** | ART method replacement via JNI |
| **Android** | 7.0–16 |
| **Architectures** | ARM, ARM64, x86, x86_64 |

**Capabilities:**
- Hook/Unhook `java.lang.reflect.Method` and `Constructor`
- No trampoline management needed
- Delegates `patchMemory` to KittyMemory fallback

**Additional API:**
```cpp
bool init(JNIEnv* env);
bool loadLibrary(JNIEnv* env, const char* libName);
bool hookMethod(JNIEnv* env, jobject targetMethod, jobject hookMethod, jobject backupMethod);
bool unhookMethod(JNIEnv* env, jobject targetMethod, jobject hookMethod, jobject backupMethod);
```

---

### 2. Bhook (PLT/GOT Hook)

| Property | Value |
|----------|-------|
| **Adapter** | `BhookAdapter` |
| **Source** | https://github.com/bytedance/bhook |
| **License** | MIT |
| **Version** | 1.1.2 |
| **Mechanism** | Dynamic linker PLT/GOT interception |
| **Android** | 4.1–17 |
| **Architectures** | ARM, ARM64, x86, x86_64 |

**Capabilities:**
- Hook all callers' imports of a shared library function
- Hook single caller-specific imports
- Automatic or manual mode
- Delegates `patchMemory` to KittyMemory fallback

**Additional API:**
```cpp
bool init(int mode = 0, bool debug = false);  // mode: 0=automatic, 1=manual
bytehook_stub_t hookSingle(const char* callerPath, const char* calleePath,
                           const char* symName, void* newFunc, ...);
bytehook_stub_t hookAll(const char* calleePath, const char* symName,
                        void* newFunc, ...);
```

---

### 3. Shadowhook (Inline Hook)

| Property | Value |
|----------|-------|
| **Adapter** | `ShadowhookAdapter` |
| **Source** | https://github.com/bytedance/android-inline-hook |
| **License** | MIT |
| **Version** | 2.0.1 |
| **Mechanism** | Function prologue rewrite with trampoline |
| **Android** | 4.1–17 |
| **Architectures** | ARM, ARM64, x86, x86_64 |

**Capabilities:**
- True inline hooking at instruction level
- Trampoline allocation handled internally
- Three modes: SHARED, UNIQUE, MULTI
- Delegates `patchMemory` to KittyMemory fallback

**Additional API:**
```cpp
bool init(shadowhook_mode_t mode = SHADOWHOOK_MODE_SHARED, bool debug = false);
void* hookByAddr(void* funcAddr, void* newAddr, void** origAddr);
void* hookByName(const char* libName, const char* symName, void* newAddr, void** origAddr);
```

---

### 4. Vectorhook (Traceless Hook)

| Property | Value |
|----------|-------|
| **Adapter** | `VectorhookAdapter` |
| **Source** | https://github.com/1013503897/stealth-core |
| **License** | MIT (stealth-core layer) |
| **Version** | 0.1.0 |
| **Mechanism** | kpmhook API (KPM-based inline hook) |
| **Android** | ARM64 only |
| **Architectures** | ARM64 |

**Capabilities:**
- Traceless inline hooking via KPM
- Anti-detection resistant
- Delegates `patchMemory` to KittyMemory fallback

---

### 5. SQLhook (Database Hook)

| Property | Value |
|----------|-------|
| **Adapter** | `SQLhookAdapter` |
| **Location** | `HookEngines/DatabaseHook/` |
| **Mechanism** | SQLite function interception |

**Capabilities:**
- Hook `sqlite3_exec`, `sqlite3_prepare_v2`, `sqlite3_step`, `sqlite3_close`
- Monitor/modify database queries at runtime
- Delegates `patchMemory` to KittyMemory fallback

---

### 6. NetworkPacketHook (Network Hook)

| Property | Value |
|----------|-------|
| **Adapter** | `NetworkPacketHookAdapter` |
| **Location** | `HookEngines/NPHook/` |
| **Mechanism** | dlsym resolution + inline hook |

**Capabilities:**
- Hook `send`, `recv`, `connect`, `socket` via dlsym
- Monitor/modify network packets at runtime
- Delegates `patchMemory` to KittyMemory fallback

---

### 7. KittyMemory (In-Process Memory)

| Property | Value |
|----------|-------|
| **Adapter** | `KittyMemoryAdapter` |
| **Source** | https://github.com/MJx0/KittyMemory |
| **License** | MIT |
| **Scope** | Same-process memory |

**Capabilities:**
- `memRead`, `memWrite`, `memExecWrite`, `memProtect`
- Process memory map enumeration
- Module base address lookup
- Memory dump to disk

**Additional API:**
```cpp
std::vector<std::string> getAllMaps();
uintptr_t getModuleBase(const std::string& moduleName);
```

---

### 8. KittyMemoryEx (Cross-Process Memory)

| Property | Value |
|----------|-------|
| **Adapter** | `KittyMemoryExAdapter` |
| **Scope** | Cross-process memory (by PID) |

**Capabilities:**
- Read/write memory of other processes
- Requires root or ptrace capability
- Used by `MemoryEditing::setTargetPid(pid_t pid)`

---

## Usage Guide

### Basic Hook Installation

```cpp
#include "modules/HPT/HPT.h"

using namespace omnibyte::runtime;

HPT hpt;

// Register backends
hpt.registerBackend(std::make_shared<backends::AlbatrossAdapter>());
hpt.registerBackend(std::make_shared<backends::BhookAdapter>());
hpt.registerBackend(std::make_shared<backends::ShadowhookAdapter>());

// Select best available
std::vector<std::string> priority = {"Shadowhook", "Bhook", "Albatross"};
hpt.selectBackend(priority);

// Hook a function
void* original = nullptr;
bool ok = hpt.hookFunction(targetAddr, myDetour, &original);

// Unhook
hpt.unhook(targetAddr);
```

### Memory Editing

```cpp
// Register memory editors
hpt.registerEditor(std::make_shared<backends::KittyMemoryAdapter>());
hpt.registerEditor(std::make_shared<backends::KittyMemoryExAdapter>());

// Select editor
hpt.selectEditor({"KittyMemory", "KittyMemoryEx"});

// Read memory
uint32_t value = 0;
hpt.readMemory(addr, &value, sizeof(value));

// Patch memory
uint8_t nop[] = {0x00, 0x00, 0x00, 0x00};
hpt.patchMemory(addr, nop, sizeof(nop));

// Dump to file
hpt.dumpMemory(addr, 4096, "/sdcard/dump.bin");
```

### Cross-Process Memory (KittyMemoryEx)

```cpp
auto* editor = std::make_shared<backends::KittyMemoryExAdapter>();
hpt.registerEditor(editor);
hpt.selectEditor({"KittyMemoryEx"});

// Set target process
editor->setTargetPid(targetPid);

// Read/write remote memory
uint32_t value = 0;
hpt.readMemory(remoteAddr, &value, sizeof(value));
```

---

## Backend Selection Strategy

1. **Register** all available backends via `registerBackend()`
2. **Select** using priority list via `selectBackend(priority)`
3. First available backend matching the priority wins
4. If no backend available, operations return false

**Recommended Priority Order:**

| Use Case | Recommended Priority |
|----------|---------------------|
| ART Method Hook | Albatross → Shadowhook |
| PLT/GOT Hook | Bhook → Shadowhook |
| Inline Hook | Shadowhook → Bhook |
| Traceless Hook | Vectorhook → Shadowhook |
| Database Hook | SQLhook → Bhook |
| Network Hook | NPhook → Bhook |

---

## Lifecycle

```
1. HPT hpt;                           // Create orchestrator
2. hpt.registerBackend(...);          // Register backends
3. hpt.selectBackend(priority);       // Select best available
4. hpt.hookFunction(addr, fn, &orig); // Install hooks
5. // ... runtime operations ...
6. hpt.unhook(addr);                  // Remove hooks
7. hpt.release();                     // Release all backends
```

---

## Integration with Other Modules

- **Runtime/HPTManager** — Manages HPT lifecycle (init, use, destroy)
- **Runtime/ZigZag** — Stealth backends can hide hooking activity
- **Runtime/RootFileAccess** — Root-based memory access for non-rooted apps
- **Runtime/BinaryInstaller** — Deploys hook libraries into target apps
- **modules/Dumper** — Uses HPT for dynamic analysis during dump operations
