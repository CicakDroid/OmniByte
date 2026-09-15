<p align="center">
  <img src="assets/icon.svg" width="128" alt="OmniByte Icon"/>
</p>

<h1 align="center">OmniByte</h1>

<p align="center">
  <strong>Инструментарий обратного инженеринга Android</strong><br/>
  Декомпилятор &bull; Редактор &bull; Дампер &bull; Хукинг &bull; Редактор памяти &bull; Мониторинг сети
</p>

<p align="center">
  <a href="README.md">🇮🇩 Bahasa Indonesia</a> &bull;
  <a href="README_EN.md">🇬🇧 English</a> &bull;
  <a href="README_PT.md">🇧🇷 Portugu&ecirc;s</a> &bull;
  <strong>🇷🇺 Русский</strong> &bull;
  <a href="README_ZH.md">🇨🇳 中文</a>
</p>

---

## О проекте

OmniByte — это инструментарий обратного инженеринга Android, построенный на **Kotlin + C++ Native**, с поддержкой мульти-архитектуры (**ARMv7** & **ARMv8a**) и мульти-версий Android (**от 6.0 до последней версии**).

Инструментарий предназначен для статического и динамического анализа, декомпиляции, редактирования бинарников, хукинга функций, редактирования памяти, а также мониторинга, захвата и редактирования сети — всё в одной интегрированной платформе. **Без поднятия до LLVM**.

**Особые возможности:**
- ✅ **Работает с root И без root** — Поддерживает root-доступ (KernelSU, Magisk, SukiSU) и безroot-режим через `/proc/pid/mem`
- 🔍 **Universal Dumper для Android Native Library** — Дамп всех библиотек `.so` из процессов Android (libil2cpp.so, libtamarin.so, libunity.so и т.д.)
- ⚡ **HPT Orchestrator** — Оркестрация хукинга и редактирования памяти через подсистемы Hooking/MemoryEditing
- 🔄 **Taskflow Adapter** — Адаптация taskflow для планирования и параллелизации

## Основные возможности

| Возможность | Описание |
|-------------|----------|
| 🔍 **APK Decompiler** | Декомпиляция APK в исходный код (Java/Smali/DEX) с поддержкой нескольких движков |
| ✏️ **APK Editor** | Редактирование манифеста, ресурсов, smali и пересборка APK |
| 📊 **Анализ бинарников** | Статический анализ бинарников (ELF/PE) с дизассемблером и декомпилятором |
| 🔧 **Редактор бинарников** | Прямое редактирование бинарников с hex-редактором и патчингом |
| 📦 **Universal Dumper** | Дамп универсальных нативных библиотек Android (все движки: Unity, Unreal, Godot и т.д.) вручную или автоматически во время Live PID |
| 🪝 **Хукинг** | Хук нативных функций и методов ART с 6 бэкендами (Albatross, Bhook, Vector, KittyMemory, SQLhook, NPhook) |
| 🧠 **Редактор памяти** | Чтение и запись памяти активных процессов через KittyMemory/KittyMemoryEx |
| 🗄️ **Database Hooking** | Хук функций SQLite (sqlite3_exec, sqlite3_prepare_v2 и т.д.) через SQLhook |
| 🌐 **Network Hooking** | Хук сетевых функций libc (send, recv, connect, socket) через NPhook (dlsym + inline hook) |
| 📡 **Мониторинг, захват и редактирование сети** | Мониторинг, захват и редактирование сетевых пакетов в реальном времени |

## Поддержка платформы

| Компонент | Поддержка |
|-----------|-----------|
| **Android** | 6.0 (API 23) — последняя версия |
| **Архитектура** | ARMv7 (32 бита), ARMv8a (64 бита) |
| **Языки** | Kotlin (UI), C++17 (нативное ядро) |
| **Система сборки** | Gradle + CMake |
| **STL** | c++_shared |

## Структура проекта

```
OmniByte/
├── app/                          # Android приложение (Kotlin)
│   └── src/main/
│       ├── java/com/omnibyte/    # Исходный код Kotlin
│       ├── cpp/                  # Нативный агрегатор (CMake)
│       └── res/                  # Ресурсы Android
├── Hydra/                        # Движок статического анализа
│   ├── Hydra2D/                  # Основной движок анализа
│   │   ├── Disassembler/         # Дизассемблирование бинарников
│   │   ├── Decompiler/           # Декомпиляция кода
│   │   ├── Parser/               # Парсинг форматов бинарников
│   │   ├── Orchestrator/         # Оркестрация анализа
│   │   ├── Factory/              # Фабрика компонентов
│   │   ├── Plugin/               # Плагины анализа
│   │   │   ├── Enhanced/         # Расширенные плагины
│   │   │   │   ├── AST/          # Абстрактное синтаксическое дерево
│   │   │   │   ├── CFG/          # Граф потока управления
│   │   │   │   ├── Crypt/        # Обнаружение криптографии
│   │   │   │   ├── Deobfuscate/  # Деобфускация
│   │   │   │   ├── Emulation/    # Эмуляция бинарников
│   │   │   │   ├── FunctionResolver/
│   │   │   │   ├── RTTI/         # Информация о типах времени выполнения
│   │   │   │   ├── Signatures/   # Паттерн-подписи
│   │   │   │   └── SymbolicExecution/
│   │   │   └── ScriptHooks/      # Скриптовые хуки
│   │   └── docs/
│   └── Shared/                   # Общие метаданные
├── modules/
│   ├── Dumper/                   # Модуль Universal Dumper
│   │   ├── DumperCore/           # Основная логика дампера
│   │   │   ├── Detector/         # Обнаружение движка
│   │   │   ├── EngineRegistry/   # Регистрация движков
│   │   │   ├── ResultNormalizer/ # Нормализация вывода
│   │   │   ├── SignatureBypass/  # Обход подписи APK
│   │   │   ├── SharedUtils/      # Утилиты
│   │   │   └── WorkingModes/     # Ручной и Live режимы
│   │   ├── Engines/              # 7 движков дампера
│   │   │   ├── UnityIL2CPP/      # Unity IL2CPP
│   │   │   ├── UnityMono/        # Unity Mono
│   │   │   ├── UnrealEngine/     # Unreal Engine
│   │   │   ├── Godot/            # Godot Engine
│   │   │   ├── Cocos2d/          # Cocos2d
│   │   │   ├── GameMaker/        # GameMaker
│   │   │   └── Source2/          # Source 2
│   │   └── Export/               # Экспортные модули
│   ├── Hooker/                   # Модуль хукинга
│   │   ├── HookEngine/           # Техники хукинга
│   │   │   ├── ARTHook/          # Хук методов ART
│   │   │   ├── InlineHook/       # Инлайн-хук функций
│   │   │   ├── PLT_GOTHook/      # PLT/GOT хук
│   │   │   └── TracelessHook/    # Безследный хук
│   │   └── backends/             # Бэкенды хукинга
│   │       ├── Albatross/        # Бэкенд ART хука
│   │       ├── Bhook/            # Бэкенд PLT/GOT
│   │       ├── Inlinehook/       # Бэкенд инлайн-хука
│   │       ├── KittyMemory/      # Патчинг памяти
│   │       ├── KittyMemoryEx/    # Расширенная память
│   │       └── Vector/           # Безследный бэкенд
│   └── HPT/                      # Модуль HPT Orchestrator
│       ├── Hooker/               # Подсистема хукинга (IHookBackend)
│       │   ├── Albatross/        # ART method hook (pls/plthook + shadowhook)
│       │   ├── Bhook/            # PLT/GOT hook (PLTHook + shadowhook)
│       │   ├── Inlinehook/       # Inline hook (shadowhook)
│       │   ├── Vectorhook/       # Traceless hook (pltinline)
│       │   ├── SQLhook/          # Database hook (sqlite3 hooking)
│       │   └── NPhook/           # Network packet hook (dlsym + shadowhook)
│       └── MemoryEditor/         # Подсистема редактирования памяти (IMemoryEditor)
├── runtime/                      # Рантайм активного устройства
│   ├── Bridges/                  # Загрузчики мостов
│   │   ├── FreedomServiceBridge/ # Мост сервиса root
│   │   └── ModuleBridge/         # Мост модуля
│   ├── ProcessManager/           # Управление процессами
│   ├── MemoryIO/                 # Чтение/запись памяти
│   ├── SymbolResolver/           # Разрешение символов (xdl)
│   ├── ZigZag/                   # Движок скрытности/обхода
│   ├── ZigZagManager/            # Управление скрытностью
│   ├── HPTManager/               # Управление жизненным циклом HPT
│   └── FreedomService/           # Сервис root-доступа
│       ├── KernelSU-Next/        # Поддержка KernelSU
│       ├── SUI/                  # Поддержка Magisk SUI
│       ├── SukiSU-Ultra/         # Поддержка SukiSU
│       └── RootThread/           # Управление потоком root
├── common/                       # Общие утилиты
│   ├── Math/                     # Математические утилиты
│   ├── Json/                     # Сериализация
│   └── Taskflow/                 # Адаптер Taskflow (FetchContent v3.8.0)
├── toolchain/                    # Инструментальная цепочка
│   ├── rizin-android/            # Дизассемблер Rizin
│   └── stub-headers/             # Заглушки заголовков
├── scripts/                      # Скрипты сборки
├── docs/                         # Документация
└── test/                         # Тестовые фикстуры
```

## Рабочий пайплайн

<details open>
<summary><strong>Рабочий пайплайн (нажмите чтобы скрыть/показать)</strong></summary>

```mermaid
flowchart TB
    subgraph INPUT["Вход"]
        APK["Файл APK"]
        BIN["Бинарник (ELF/PE)"]
        PID["Активный процесс (PID)"]
    end

    subgraph HYDRA["Hydra - Статический анализ"]
        direction TB
        H_DIS["Дизассемблер\n(Capstone / Rizin)"]
        H_DEC["Декомпилятор\n(Rizin / rz-ghidra)"]
        H_PAR["Парсер\n(LIEF)"]
        H_PLG["Плагины\n(AST, CFG, Crypt,\nDeobfuscate, Signatures,\nSymbolicExecution)"]
        H_ORC["Оркестратор"]
        H_DIS --> H_ORC
        H_DEC --> H_ORC
        H_PAR --> H_ORC
        H_PLG --> H_ORC
    end

    subgraph DUMPER["Universal Dumper - Нативные библиотеки Android"]
        direction TB
        D_DET["Детектор\n(Magic Bytes,\nПроверка версии)"]
        D_ENG["Движки\n(UnityIL2CPP, UnityMono,\nUnrealEngine, Godot,\nCocos2d, GameMaker, Source2)"]
        D_RES["Резолвер\n(Регистрация, Символ)"]
        D_EXP["Экспорт\n(CSharpWriter, StructGen)"]
        D_DET --> D_ENG
        D_ENG --> D_RES
        D_RES --> D_EXP
    end

    subgraph HPT["HPT Orchestrator"]
        direction TB
        HK_MOD["Подсистема хукинга\n(IHookBackend)"]
        ME_MOD["Подсистема редактирования памяти\n(IMemoryEditor)"]
        HK_MOD --> HK_ART["ART Hook\n(Albatross)"]
        HK_MOD --> HK_INL["Inline Hook\n(android-inline-hook)"]
        HK_MOD --> HK_PLT["PLT/GOT Hook\n(Bhook)"]
        HK_MOD --> HK_TLS["Безследный Hook\n(Vector)"]
        HK_MOD --> HK_DB["Database Hook\n(SQLhook)"]
        HK_MOD --> HK_NP["Network Hook\n(NPhook)"]
        HK_ART --> HK_MEM["Патчинг памяти\n(KittyMemory)"]
        HK_INL --> HK_MEM
        HK_PLT --> HK_MEM
        HK_TLS --> HK_MEM
        HK_DB --> HK_MEM
        HK_NP --> HK_MEM
        ME_MOD --> ME_KIT["KittyMemory"]
        ME_MOD --> ME_KITX["KittyMemoryEx"]
    end

    subgraph RUNTIME["Рантайм - Активное устройство"]
        direction TB
        R_BRG["Мосты\n(FreedomService, Module)"]
        R_PM["ProcessManager"]
        R_MIO["MemoryIO"]
        R_SYM["SymbolResolver\n(xdl)"]
        R_ZZ["ZigZag\n(Скрытность/Обход)"]
        R_HPT["HPTManager\n(Жизненный цикл)"]
        R_FS["FreedomService\n(KernelSU, SUI, SukiSU)"]
        R_BRG --> R_PM
        R_PM --> R_MIO
        R_PM --> R_SYM
        R_ZZ --> R_FS
        R_HPT --> HK_MOD
        R_HPT --> ME_MOD
    end

    subgraph TASKFLOW["Taskflow - Параллельное планирование"]
        direction TB
        TF["TaskflowAdapter\n(FetchContent v3.8.0)"]
    end

    subgraph OUTPUT["Выход"]
        O_CS["dump.cs\n(Определения C#)"]
        O_ST["struct_dump.cs\n(Макет структуры)"]
        O_SF["static_fields.txt\n(Статические оффсеты)"]
        O_SM["Исходный код Smali"]
        O_PT["Результат патча"]
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

## Архитектура модулей

<details open>
<summary><strong>Архитектура модулей (нажмите чтобы скрыть/показать)</strong></summary>

```mermaid
flowchart LR
    subgraph CORE["Базовый слой"]
        COMMON["common/\n(Math, Json, Taskflow)"]
        TOOLCHAIN["toolchain/\n(Rizin, Headers)"]
    end

    subgraph NATIVE["Нативный слой (C++17)"]
        HYDRA["Hydra / Hydra2D\nСтатический анализ"]
        DUMPER["Universal Dumper\nНативный дамп Android"]
        HPT["HPT Orchestrator\nХукинг + MemoryEditing"]
        RUNTIME["Runtime\nАктивное устройство"]
    end

    subgraph HOOKS["Бэкенды хукинга"]
        HK_ART["Albatross\n(ART Hook)"]
        HK_BHK["Bhook\n(PLT/GOT)"]
        HK_VEC["Vector\n(Traceless)"]
        HK_KIT["KittyMemory\n(Патчинг памяти)"]
        HK_SQL["SQLhook\n(Database Hook)"]
        HK_NP["NPhook\n(Network Hook)"]
    end

    subgraph UI["UI слой (Kotlin)"]
        APP["app/\nAndroid приложение"]
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

## Сборка

```bash
# Клонировать
git clone https://github.com/CicakDroid/OmniByte.git
cd OmniByte

# Сборка APK
./gradlew assembleDebug

# Или сборка только нативного кода
./scripts/build-native.sh

# Сборка с определёнными опциями
./scripts/build-native.sh --abi arm64-v8a --api 23
./scripts/build-native.sh --abi armeabi-v7a --api 21
./scripts/build-native.sh --clean
```

> **Примечание:** Нативная сборка требует Android SDK и NDK. Запустите `./scripts/build-native.sh --help` для всех опций.

## Лицензия

Этот проект является частью OmniByte Research Platform.

---

<p align="center">
  <sub>Создано с ❤️ от <a href="https://github.com/CicakDroid">CicakDroid</a></sub>
</p>
