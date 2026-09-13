<p align="center">
  <img src="../assets/icon.svg" width="128" alt="OmniByte Icon"/>
</p>

<h1 align="center">OmniByte</h1>

<p align="center">
  <strong>Android 逆向工程工具包</strong><br/>
  反编译器 &bull; 编辑器 &bull; Dumper &bull; Hooking &bull; 内存编辑器 &bull; 网络监控
</p>

<p align="center">
  <a href="README.md">🇮🇩 Bahasa Indonesia</a> &bull;
  <a href="README_EN.md">🇬🇧 English</a> &bull;
  <a href="README_PT.md">🇧🇷 Portugu&ecirc;s</a> &bull;
  <a href="README_RU.md">🇷🇺 Русский</a> &bull;
  <strong>🇨🇳 中文</strong>
</p>

---

## 关于

OmniByte 是一个基于 **Kotlin + C++ Native** 构建的 Android 逆向工程工具包，支持多架构（**ARMv7** & **ARMv8a**）和多版本 Android（**6.0 到最新版本**）。

该工具包专为静态和动态分析、反编译、二进制编辑、函数 Hooking、内存编辑以及网络数据包监控和编辑而设计 — 全部集成在一个统一平台中。

## 主要功能

| 功能 | 描述 |
|------|------|
| 🔍 **APK 反编译器** | 将 APK 反编译为源代码（Java/Smali/DEX），支持多引擎 |
| ✏️ **APK 编辑器** | 编辑清单、资源、smali 并重新构建 APK |
| 📊 **二进制分析** | 静态二进制分析（ELF/PE），包含反汇编器和反编译器 |
| 🔧 **二进制编辑器** | 直接编辑二进制文件，支持十六进制编辑器和补丁 |
| 📦 **Binary Dumper** | 手动或在 Live PID 期间自动转储二进制结构 |
| 🪝 **Hooking** | 使用 4 种不同技术 Hook 原生函数和 ART 方法 |
| 🧠 **内存编辑器** | 读写活动进程内存 |
| 🌐 **网络数据包监控器** | 实时监控网络流量 |
| 📝 **网络数据包编辑器** | 实时拦截和编辑网络数据包 |

## 平台支持

| 组件 | 支持 |
|------|------|
| **Android** | 6.0 (API 23) — 最新版本 |
| **架构** | ARMv7（32位），ARMv8a（64位） |
| **语言** | Kotlin（UI），C++17（原生核心） |
| **构建系统** | Gradle + CMake |
| **STL** | c++_shared |

## 项目结构

```
OmniByte/
├── app/                          # Android 应用程序 (Kotlin)
│   └── src/main/
│       ├── java/com/omnibyte/    # Kotlin 源代码
│       ├── cpp/                  # 原生聚合器 (CMake)
│       └── res/                  # Android 资源
├── Hydra/                        # 静态分析引擎
│   ├── Hydra2D/                  # 核心分析引擎
│   │   ├── Disassembler/         # 二进制反汇编
│   │   ├── Decompiler/           # 代码反编译
│   │   ├── Parser/               # 二进制格式解析
│   │   ├── Orchestrator/         # 分析编排
│   │   ├── Factory/              # 组件工厂
│   │   ├── Plugin/               # 分析插件
│   │   │   ├── Enhanced/         # 高级插件
│   │   │   │   ├── AST/          # 抽象语法树
│   │   │   │   ├── CFG/          # 控制流图
│   │   │   │   ├── Crypt/        # 加密检测
│   │   │   │   ├── Deobfuscate/  # 反混淆
│   │   │   │   ├── Emulation/    # 二进制模拟
│   │   │   │   ├── FunctionResolver/
│   │   │   │   ├── RTTI/         # 运行时类型信息
│   │   │   │   ├── Signatures/   # 模式签名
│   │   │   │   └── SymbolicExecution/
│   │   │   └── ScriptHooks/      # 脚本 Hook
│   │   └── docs/
│   └── Shared/                   # 共享元数据
├── modules/
│   ├── Dumper/                   # Binary Dumper 模块
│   │   ├── DumperCore/           # 核心 dumper 逻辑
│   │   │   ├── Detector/         # 引擎检测
│   │   │   ├── EngineRegistry/   # 引擎注册
│   │   │   ├── ResultNormalizer/ # 输出规范化
│   │   │   ├── SignatureBypass/  # APK 签名绕过
│   │   │   ├── SharedUtils/      # 工具类
│   │   │   └── WorkingModes/     # 手动 & Live 模式
│   │   ├── Engines/              # 7 个 Dumper 引擎
│   │   │   ├── UnityIL2CPP/      # Unity IL2CPP
│   │   │   ├── UnityMono/        # Unity Mono
│   │   │   ├── UnrealEngine/     # Unreal Engine
│   │   │   ├── Godot/            # Godot Engine
│   │   │   ├── Cocos2d/          # Cocos2d
│   │   │   ├── GameMaker/        # GameMaker
│   │   │   └── Source2/          # Source 2
│   │   └── Export/               # 输出写入器
│   ├── Hooker/                   # 函数 Hooking 模块
│   │   ├── HookEngine/           # Hook 技术
│   │   │   ├── ARTHook/          # ART 方法 Hook
│   │   │   ├── InlineHook/       # 内联函数 Hook
│   │   │   ├── PLT_GOTHook/      # PLT/GOT Hook
│   │   │   └── TracelessHook/    # 反检测 Hook
│   │   └── backends/             # Hook 后端
│   │       ├── Albatross/        # ART Hook 后端
│   │       ├── Bhook/            # PLT/GOT 后端
│   │       ├── Inlinehook/       # 内联 Hook 后端
│   │       ├── KittyMemory/      # 内存补丁
│   │       ├── KittyMemoryEx/    # 扩展内存
│   │       └── Vector/           # 无痕后端
│   └── HPT/                      # HPT 模块
│       ├── Hooker/               # Hook 封装
│       └── MemoryEditor/         # 内存编辑器
├── runtime/                      # 活跃设备运行时
│   ├── Bridges/                  # 桥接加载器
│   │   ├── FreedomServiceBridge/ # Root 服务桥接
│   │   └── ModuleBridge/         # 模块桥接
│   ├── ProcessManager/           # 进程管理
│   ├── MemoryIO/                 # 内存读写
│   ├── SymbolResolver/           # 符号解析 (xdl)
│   ├── ZigZag/                   # 隐身/绕过引擎
│   ├── ZigZagManager/            # 隐身管理
│   └── FreedomService/           # Root 访问服务
│       ├── KernelSU-Next/        # KernelSU 支持
│       ├── SUI/                  # Magisk SUI 支持
│       ├── SukiSU-Ultra/         # SukiSU 支持
│       └── RootThread/           # Root 线程管理
├── common/                       # 共享工具类
│   ├── Math/                     # 数学工具
│   └── Serialization/            # 序列化
├── toolchain/                    # 构建工具链
│   ├── rizin-android/            # Rizin 反汇编器
│   └── stub-headers/             # 桩头文件
├── scripts/                      # 构建脚本
├── docs/                         # 文档
└── test/                         # 测试数据
```

## 工作流程

```mermaid
flowchart TB
    subgraph INPUT["输入"]
        APK["APK 文件"]
        BIN["二进制文件 (ELF/PE)"]
        PID["活动进程 (PID)"]
    end

    subgraph HYDRA["Hydra - 静态分析"]
        direction TB
        H_DIS["反汇编器\n(Capstone / Rizin)"]
        H_DEC["反编译器\n(Rizin / rz-ghidra)"]
        H_PAR["解析器\n(LIEF)"]
        H_PLG["插件\n(AST, CFG, Crypt,\nDeobfuscate, Signatures,\nSymbolicExecution)"]
        H_ORC["编排器"]
        H_DIS --> H_ORC
        H_DEC --> H_ORC
        H_PAR --> H_ORC
        H_PLG --> H_ORC
    end

    subgraph DUMPER["Dumper - 二进制转储"]
        direction TB
        D_DET["检测器\n(Magic Bytes,\n版本检查)"]
        D_ENG["引擎\n(UnityIL2CPP, UnityMono,\nUnrealEngine, Godot,\nCocos2d, GameMaker, Source2)"]
        D_RES["解析器\n(注册, 符号)"]
        D_EXP["导出\n(CSharpWriter, StructGen)"]
        D_DET --> D_ENG
        D_ENG --> D_RES
        D_RES --> D_EXP
    end

    subgraph HOOKER["Hooker - 函数 Hooking"]
        direction TB
        HK_ART["ART Hook\n(Albatross)"]
        HK_INL["Inline Hook\n(android-inline-hook)"]
        HK_PLT["PLT/GOT Hook\n(Bhook)"]
        HK_TLS["无痕 Hook\n(Vector)"]
        HK_ART --> HK_MEM["内存补丁\n(KittyMemory)"]
        HK_INL --> HK_MEM
        HK_PLT --> HK_MEM
        HK_TLS --> HK_MEM
    end

    subgraph RUNTIME["运行时 - 活跃设备"]
        direction TB
        R_BRG["桥接\n(FreedomService, Module)"]
        R_PM["ProcessManager"]
        R_MIO["MemoryIO"]
        R_SYM["SymbolResolver\n(xdl)"]
        R_ZZ["ZigZag\n(隐身/绕过)"]
        R_FS["FreedomService\n(KernelSU, SUI, SukiSU)"]
        R_BRG --> R_PM
        R_PM --> R_MIO
        R_PM --> R_SYM
        R_ZZ --> R_FS
    end

    subgraph OUTPUT["输出"]
        O_CS["dump.cs\n(C# 定义)"]
        O_ST["struct_dump.cs\n(结构体布局)"]
        O_SF["static_fields.txt\n(静态偏移)"]
        O_SM["Smali 源代码"]
        O_PT["补丁结果"]
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
```

## 模块架构

```mermaid
flowchart LR
    subgraph CORE["基础层"]
        COMMON["common/\n(Math, Serialization)"]
        TOOLCHAIN["toolchain/\n(Rizin, Headers)"]
    end

    subgraph NATIVE["原生层 (C++17)"]
        HYDRA["Hydra / Hydra2D\n静态分析"]
        DUMPER["Dumper\n二进制转储"]
        HOOKER["Hooker\n函数 Hooking"]
        RUNTIME["Runtime\n活跃设备"]
    end

    subgraph UI["UI 层 (Kotlin)"]
        APP["app/\nAndroid 应用程序"]
    end

    CORE --> HYDRA
    CORE --> DUMPER
    CORE --> HOOKER
    CORE --> RUNTIME
    TOOLCHAIN --> HYDRA

    HYDRA --> DUMPER
    RUNTIME --> DUMPER
    RUNTIME --> HOOKER

    HYDRA -.-> APP
    DUMPER -.-> APP
    HOOKER -.-> APP
    RUNTIME -.-> APP
```

## 构建

```bash
# 克隆
git clone https://github.com/CicakDroid/OmniByte.git
cd OmniByte

# 构建 APK
./gradlew assembleDebug

# 或仅构建原生代码
cd app/src/main/cpp
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-23 \
      ..
make
```

## 许可证

本项目是 OmniByte Research Platform 的一部分。

---

<p align="center">
  <sub>由 <a href="https://github.com/CicakDroid">CicakDroid</a> 用 ❤️ 构建</sub>
</p>
