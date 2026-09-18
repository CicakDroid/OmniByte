<p align="center">
  <img src="assets/icon.svg" width="128" alt="OmniByte Icon"/>
</p>

<h1 align="center">OmniByte</h1>

<p align="center">
  <strong>Android Reverse Engineering Toolkit</strong><br/>
  Decompiler &bull; Editor &bull; Dumper &bull; Hooking &bull; Memory Editor &bull; Network Monitoring
</p>

<p align="center">
  <a href="README.md">🇮🇩 Bahasa Indonesia</a> &bull;
  <strong>🇬🇧 English</strong> &bull;
  <a href="README_PT.md">🇧🇷 Portugu&ecirc;s</a> &bull;
  <a href="README_RU.md">🇷🇺 Русский</a> &bull;
  <a href="README_ZH.md">🇨🇳 中文</a>
</p>

---

## About

OmniByte is an Android reverse engineering toolkit built on **Kotlin + C++ Native**, supporting multi-architecture (**ARMv7** & **ARMv8a**) and multi-version Android (**6.0 to the latest release**).

The toolkit is designed for static & dynamic analysis, decompilation, binary editing, function hooking, memory editing, and network monitoring, capture & editing — all in a single integrated platform. **Without lifting to LLVM**.

**Special Features:**
- ✅ **Runs with root AND without root** — Supports root access (KernelSU, Magisk, SukiSU) and non-root mode via `/proc/pid/mem`
- 🔍 **Universal Dumper for Android Native Library** — Dumps all `.so` libraries from Android processes (libil2cpp.so, libtamarin.so, libunity.so, etc.)
- ⚡ **HPT Orchestrator** — Orchestrates hooking & memory editing via Hooking/MemoryEditing subsystems
- 🔄 **Taskflow Adapter** — Adapts taskflow for scheduling & parallelization

## Key Features

| Feature | Description |
|---------|-------------|
| 🔍 **APK Decompiler** | Decompile APK to source code (Java/Smali/DEX) with multi-engine support |
| ✏️ **APK Editor** | Edit manifest, resources, smali, and rebuild APK |
| 📊 **Binary Analysis** | Static & Dynamic binary analysis (ELF/PE) with disassembler & decompiler |
| 🔧 **Binary Editor** | Direct binary editing with hex editor & patching |
| 📦 **Universal Dumper** | Dump universal Android native library manually or automatically during Live PID (Bonus: 7 games engines: Unity, Unreal, Godot, etc.) |
| 🪝 **Hooking** | Hook native functions & ART methods with 6 backends (Albatross, Bhook, Vector, KittyMemory, SQLhook, NPhook) |
| 🧠 **Memory Editor** | Read & write live process memory via KittyMemory/KittyMemoryEx |
| 🗄️ **Database Hooking** | Hook SQLite database functions (sqlite3_exec, sqlite3_prepare_v2, etc.) via SQLhook |
| 🌐 **Network Hooking** | Hook libc network functions (send, recv, connect, socket) via NPhook (dlsym + inline hook) |
| 📡 **Network Monitoring, Capture & Editing** | Monitor, capture, and edit network packets in real-time |

## Platform Support

| Component | Support |
|-----------|---------|
| **Android** | 6.0 (API 23) — Latest |
| **Architecture** | ARMv7 (32-bit), ARMv8a (64-bit) |
| **Languages** | Kotlin (UI), C++17 (Native Core) |
| **Build System** | Gradle + CMake |
| **STL** | c++_shared |

## Project Structure

```
OmniByte/
├── app/                          # Android Application (Kotlin)
│   └── src/main/
│       ├── java/com/omnibyte/    # Kotlin source
│       ├── cpp/                  # Native aggregator (CMake)
│       └── res/                  # Android resources
├── Hydra/                        # Static & Dynamic Analysis Engine
│   ├── Hydra2D/                  # Core analysis engine
│   │   ├── Disassembler/         # Binary disassembly
│   │   ├── Decompiler/           # Code decompilation
│   │   ├── Parser/               # Binary format parsing
│   │   ├── Orchestrator/         # Analysis orchestration
│   │   ├── Factory/              # Component factory
│   │   ├── Plugin/               # Analysis plugins
│   │   │   ├── Enhanced/         # Advanced plugins
│   │   │   │   ├── AST/          # Abstract Syntax Tree
│   │   │   │   ├── CFG/          # Control Flow Graph
│   │   │   │   ├── Crypt/        # Crypto detection
│   │   │   │   ├── Deobfuscate/  # Deobfuscation
│   │   │   │   ├── Emulation/    # Binary emulation
│   │   │   │   ├── FunctionResolver/
│   │   │   │   ├── RTTI/         # Runtime Type Info
│   │   │   │   ├── Signatures/   # Pattern signatures
│   │   │   │   └── SymbolicExecution/
│   │   │   └── ScriptHooks/      # Script-based hooks
│   │   └── docs/
│   └── Shared/                   # Shared metadata
├── modules/
│   ├── Dumper/                   # Universal Dumper Module
│   │   ├── DumperCore/           # Core dumper logic
│   │   │   ├── Detector/         # Engine detection
│   │   │   ├── EngineRegistry/   # Engine registration
│   │   │   ├── ResultNormalizer/ # Output normalization
│   │   │   ├── SignatureBypass/  # APK signature bypass
│   │   │   ├── SharedUtils/      # Utilities
│   │   │   └── WorkingModes/     # Manual & Live modes
│   │   ├── Engines/              # 7 Dumper Engines
│   │   │   ├── UnityIL2CPP/      # Unity IL2CPP
│   │   │   ├── UnityMono/        # Unity Mono
│   │   │   ├── UnrealEngine/     # Unreal Engine
│   │   │   ├── Godot/            # Godot Engine
│   │   │   ├── Cocos2d/          # Cocos2d
│   │   │   ├── GameMaker/        # GameMaker
│   │   │   └── Source2/          # Source 2
│   │   └── Export/               # Output writers
│   ├── Hooker/                   # Function Hooking Module
│   │   ├── HookEngine/           # Hook techniques
│   │   │   ├── ARTHook/          # ART method hooking
│   │   │   ├── InlineHook/       # Inline function hooking
│   │   │   ├── PLT_GOTHook/      # PLT/GOT hooking
│   │   │   └── TracelessHook/    # Anti-detection hooking
│   │   └── backends/             # Hook backends
│   │       ├── Albatross/        # ART hook backend
│   │       ├── Bhook/            # PLT/GOT backend
│   │       ├── Inlinehook/       # Inline hook backend
│   │       ├── KittyMemory/      # Memory patching
│   │       ├── KittyMemoryEx/    # Extended memory
│   │       └── Vector/           # Traceless backend
│   └── HPT/                      # HPT Orchestrator Module
│       ├── Hooker/               # Hooking subsystem (IHookBackend)
│       │   ├── Albatross/        # ART method hook (pls/plthook + shadowhook)
│       │   ├── Bhook/            # PLT/GOT hook (PLTHook + shadowhook)
│       │   ├── Inlinehook/       # Inline hook (shadowhook)
│       │   ├── Vectorhook/       # Traceless hook (pltinline)
│       │   ├── SQLhook/          # Database hook (sqlite3 hooking)
│       │   └── NPhook/           # Network packet hook (dlsym + shadowhook)
│       └── MemoryEditor/         # Memory editing subsystem (IMemoryEditor)
├── runtime/                      # Live Device Runtime
│   ├── Bridges/                  # Bridge loaders
│   │   ├── FreedomServiceBridge/ # Root service bridge
│   │   └── ModuleBridge/         # Module bridge
│   ├── ProcessManager/           # Process management
│   ├── MemoryIO/                 # Memory read/write
│   ├── SymbolResolver/           # Symbol resolution (xdl)
│   ├── ZigZag/                   # Stealth/bypass engine
│   ├── ZigZagManager/            # Stealth management
│   ├── HPTManager/               # HPT lifecycle management
│   └── FreedomService/           # Root access service
│       ├── KernelSU-Next/        # KernelSU support
│       ├── SUI/                  # Magisk SUI support
│       ├── SukiSU-Ultra/         # SukiSU support
│       └── RootThread/           # Root thread management
├── common/                       # Shared utilities
│   ├── Math/                     # Math utilities
│   ├── Json/                     # Serialization
│   └── Taskflow/                 # Taskflow adapter (FetchContent v3.8.0)
├── toolchain/                    # Build toolchain
│   ├── rizin-android/            # Rizin disassembler
│   └── stub-headers/             # Stub headers
├── scripts/                      # Build scripts
├── docs/                         # Documentation
└── test/                         # Test fixtures
```

## Working Pipeline

<details open>
<summary><strong>Working Pipeline (click to hide/unhide)</strong></summary>

```mermaid
flowchart TB
    subgraph INPUT["Input"]
        APK["APK File"]
        BIN["Binary (ELF/PE)"]
        PID["Live Process (PID)"]
    end

    subgraph HYDRA["Hydra - Static Analysis"]
        direction TB
        H_DIS["Disassembler\n(Capstone / Rizin)"]
        H_DEC["Decompiler\n(Rizin / rz-ghidra)"]
        H_PAR["Parser\n(LIEF)"]
        H_PLG["Plugins\n(AST, CFG, Crypt,\nDeobfuscate, Signatures,\nSymbolicExecution)"]
        H_ORC["Orchestrator"]
        H_DIS --> H_ORC
        H_DEC --> H_ORC
        H_PAR --> H_ORC
        H_PLG --> H_ORC
    end

    subgraph DUMPER["Universal Dumper - Android Native Library"]
        direction TB
        D_DET["Detector\n(Magic Bytes,\nVersion Check)"]
        D_ENG["Engines\n(UnityIL2CPP, UnityMono,\nUnrealEngine, Godot,\nCocos2d, GameMaker, Source2)"]
        D_RES["Resolver\n(Registration, Symbol)"]
        D_EXP["Export\n(CSharpWriter, StructGen)"]
        D_DET --> D_ENG
        D_ENG --> D_RES
        D_RES --> D_EXP
    end

    subgraph HPT["HPT Orchestrator"]
        direction TB
        HK_MOD["Hooking Subsystem\n(IHookBackend)"]
        ME_MOD["MemoryEditing Subsystem\n(IMemoryEditor)"]
        HK_MOD --> HK_ART["ART Hook\n(Albatross)"]
        HK_MOD --> HK_INL["Inline Hook\n(android-inline-hook)"]
        HK_MOD --> HK_PLT["PLT/GOT Hook\n(Bhook)"]
        HK_MOD --> HK_TLS["Traceless Hook\n(Vector)"]
        HK_MOD --> HK_DB["Database Hook\n(SQLhook)"]
        HK_MOD --> HK_NP["Network Hook\n(NPhook)"]
        HK_ART --> HK_MEM["Memory Patching\n(KittyMemory)"]
        HK_INL --> HK_MEM
        HK_PLT --> HK_MEM
        HK_TLS --> HK_MEM
        HK_DB --> HK_MEM
        HK_NP --> HK_MEM
        ME_MOD --> ME_KIT["KittyMemory"]
        ME_MOD --> ME_KITX["KittyMemoryEx"]
    end

    subgraph RUNTIME["Runtime - Live Device"]
        direction TB
        R_BRG["Bridges\n(FreedomService, Module)"]
        R_PM["ProcessManager"]
        R_MIO["MemoryIO"]
        R_SYM["SymbolResolver\n(xdl)"]
        R_ZZ["ZigZag\n(Stealth/Bypass)"]
        R_HPT["HPTManager\n(Lifecycle)"]
        R_FS["FreedomService\n(KernelSU, SUI, SukiSU)"]
        R_BRG --> R_PM
        R_PM --> R_MIO
        R_PM --> R_SYM
        R_ZZ --> R_FS
        R_HPT --> HK_MOD
        R_HPT --> ME_MOD
    end

    subgraph TASKFLOW["Taskflow - Parallel Scheduling"]
        direction TB
        TF["TaskflowAdapter\n(FetchContent v3.8.0)"]
    end

    subgraph OUTPUT["Output"]
        O_CS["dump.cs\n(C# Definitions)"]
        O_ST["struct_dump.cs\n(Struct Layout)"]
        O_SF["static_fields.txt\n(Static Offsets)"]
        O_SM["Smali Source"]
        O_PT["Patch Result"]
    end

    APK --> H_PAR
    BIN --> H_DIS
    BIN --> D_DET
    PID --> R_PM

    H_ORC --> D_DET
    D_EXP --> O_CS
    D_EXP --> O_ST
    D_EXP --> O_SF
    H_DEC --> O_SM
    HK_MEM --> O_PT
    R_MIO --> HK_MEM
    R_SYM --> D_RES
    TF --> HK_MOD
    TF --> ME_MOD
```

</details>

## Module Architecture

<details open>
<summary><strong>Module Architecture (click to hide/unhide)</strong></summary>

```mermaid
flowchart LR
    subgraph CORE["Core Layer"]
        COMMON["common/\n(Math, Json, Taskflow)"]
        TOOLCHAIN["toolchain/\n(Rizin, Headers)"]
    end

    subgraph NATIVE["Native Layer (C++17)"]
        HYDRA["Hydra / Hydra2D\nStatic Analysis"]
        DUMPER["Universal Dumper\nAndroid Native Dumping"]
        HPT["HPT Orchestrator\nHooking + MemoryEditing"]
        RUNTIME["Runtime\nLive Device"]
    end

    subgraph HOOKS["Hooking Backends"]
        HK_ART["Albatross\n(ART Hook)"]
        HK_BHK["Bhook\n(PLT/GOT)"]
        HK_VEC["Vector\n(Traceless)"]
        HK_KIT["KittyMemory\n(Memory Patching)"]
        HK_SQL["SQLhook\n(Database Hook)"]
        HK_NP["NPhook\n(Network Hook)"]
    end

    subgraph UI["UI Layer (Kotlin)"]
        APP["app/\nAndroid Application"]
    end

    CORE --> HYDRA
    CORE --> DUMPER
    CORE --> HPT
    CORE --> RUNTIME
    TOOLCHAIN --> HYDRA

    HYDRA --> DUMPER
    RUNTIME --> DUMPER
    RUNTIME --> HPT
    HPT --> HK_ART
    HPT --> HK_BHK
    HPT --> HK_VEC
    HPT --> HK_KIT
    HPT --> HK_SQL
    HPT --> HK_NP

    HYDRA -.-> APP
    DUMPER -.-> APP
    HPT -.-> APP
    RUNTIME -.-> APP
```

</details>

## Build

```bash
# Clone
git clone https://github.com/CicakDroid/OmniByte.git
cd OmniByte

# Build APK
./gradlew assembleDebug

# Or build native only
./scripts/build-native.sh

# Build with specific options
./scripts/build-native.sh --abi arm64-v8a --api 23
./scripts/build-native.sh --abi armeabi-v7a --api 21
./scripts/build-native.sh --clean
```

> **Note:** Native build requires Android SDK & NDK. Run `./scripts/build-native.sh --help` for all options.

## License

This project is part of the OmniByte Research Platform.

---

<p align="center">
  <sub>Built with ❤️ by <a href="https://github.com/CicakDroid">CicakDroid</a></sub>
</p>
