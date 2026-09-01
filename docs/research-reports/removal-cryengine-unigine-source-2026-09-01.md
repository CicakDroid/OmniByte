# Engine Removal Report: CryEngine, UNIGINE, Source (v1)

**Date:** 2026-09-01
**Author:** Sisyphus (automated)
**Status:** Research complete, awaiting code changes

---

## Summary

Three game engines are being removed from OmniByte's Dumper module because they have **no meaningful Android game presence**:

| Engine | Reason for Removal |
|--------|-------------------|
| **CryEngine** | Mobile SDK never left beta. Only 1 obscure Android game (ASH 2014). No active Android development. |
| **UNIGINE** | Positioned for simulation/enterprise/industrial, not consumer games. No Android game titles. |
| **Source (v1)** | Valve never released Source 1 on Android. Only Source 2 has Android presence (Half-Life: Alyx mobile, Dota 2 Underlords). |

**Note:** Source2/ is RETAINED — it's a separate engine with Android presence.

---

## Rationale per Engine

### CryEngine

- **CryEngine Mobile SDK** was announced ~2014-2015 but never left "in development" status
- **ASH: Immortal** (2014) was the only notable Android title — an obscure RPG that is no longer available
- Crytek's focus shifted to CryEngine 5.x for PC/console; mobile was abandoned
- No active CryEngine Android games on Play Store as of 2026
- The engine's architecture (`.pak` containers, `CrySystem.dll/.so`) is PC/console-oriented

### UNIGINE

- UNIGINE is positioned as a **simulation/visualization/enterprise** engine, not a consumer game engine
- Used for: architectural visualization, industrial simulation, GIS, oil & gas
- No known Android game titles
- The engine's licensing model targets enterprise customers, not game developers
- UNIGINE 2.x has VR/AR capabilities but for enterprise headsets, not mobile

### Source (v1)

- Valve's **Source 1 engine** was never released for Android
- Major Source 1 titles: Half-Life 2, CS:GO, Dota 2, Portal 2 — all PC/console only
- **Source 2** is the only Valve engine with Android presence (Dota 2 Underlords, Half-Life: Alyx mobile)
- Source 1's architecture (VTF textures, BSP maps, VPK archives) is PC/console-oriented
- The `Source/` directory should be removed; `Source2/` is retained

---

## Files Affected

### Directories to Delete

```
modules/Dumper/Engines/CryEngine/     (5 entries: Analyzer/, Resolver/, Profiles/, CryEngineEngine.h, CMakeLists.txt)
modules/Dumper/Engines/UNIGINE/       (5 entries: Analyzer/, Resolver/, Profiles/, UnigineEngine.h, CMakeLists.txt)
modules/Dumper/Engines/Source/        (5 entries: Analyzer/, Resolver/, Profiles/, SourceEngine.h, CMakeLists.txt)
```

### Files to Modify

1. **`modules/Dumper/DumperCore/IDumperEngine.h`**
   - Remove from `EngineType` enum: `CryEngine`, `Unigine`, `Source`
   - Keep: `Source2` (different engine)

2. **`modules/Dumper/Engines/CMakeLists.txt`**
   - Remove: `add_subdirectory(Source)`, `add_subdirectory(CryEngine)`, `add_subdirectory(UNIGINE)`
   - Keep: `add_subdirectory(Source2)`

3. **No markdown docs reference these engines** (grep confirmed no matches in .md files)

---

## Verification Commands

```bash
# Confirm no references outside the 3 directories + IDumperEngine.h + CMakeLists.txt
grep -rn "CryEngine\|Unigine\|UNIGINE" --include="*.h" --include="*.cpp" --include="*.cmake" --include="CMakeLists.txt" --include="*.md" /root/OmniByte/ 2>/dev/null | grep -v "modules/Dumper/Engines/CryEngine/" | grep -v "modules/Dumper/Engines/UNIGINE/"

# Confirm Source1 references (excluding Source2)
grep -rn "EngineType::Source\b\|SourceEngine\|#include.*Source/" --include="*.h" --include="*.cpp" /root/OmniByte/ 2>/dev/null | grep -v Source2

# Confirm Source2 is NOT being touched
ls /root/OmniByte/modules/Dumper/Engines/Source2/
```

---

## Raw Verification Output

### Directory Listing (before removal)

```
modules/Dumper/Engines/
├── CMakeLists.txt
├── CryEngine/
│   ├── Analyzer/
│   ├── CMakeLists.txt
│   ├── CryEngineEngine.h
│   ├── Profiles/
│   └── Resolver/
├── GameMaker/
├── Godot/
├── Source/
│   ├── Analyzer/
│   ├── CMakeLists.txt
│   ├── Profiles/
│   ├── Resolver/
│   └── SourceEngine.h
├── Source2/
├── UNIGINE/
│   ├── Analyzer/
│   ├── CMakeLists.txt
│   ├── Profiles/
│   ├── Resolver/
│   └── UnigineEngine.h
├── UnityIL2CPP/
├── UnityMono/
└── UnrealEngine/
```

### EngineType Enum (before change)

```cpp
enum class EngineType {
    UnrealEngine,
    UnityIL2CPP,
    UnityMono,
    Source,       // ← REMOVE (Source 1)
    Source2,      // ← KEEP
    Godot,
    CryEngine,    // ← REMOVE
    GameMaker,
    Unigine,      // ← REMOVE
    Unknown
};
```

### Engines CMakeLists.txt (before change)

```cmake
add_subdirectory(UnrealEngine)
add_subdirectory(UnityIL2CPP)
add_subdirectory(UnityMono)
add_subdirectory(Source)       # ← REMOVE
add_subdirectory(Source2)      # ← KEEP
add_subdirectory(Godot)
add_subdirectory(CryEngine)    # ← REMOVE
add_subdirectory(GameMaker)
add_subdirectory(UNIGINE)      # ← REMOVE
```

---

## Post-Removal Verification

After code changes, verify:

1. `grep -rn "CryEngine\|Unigine\|UNIGINE" modules/Dumper/` returns only Source2-related hits (if any)
2. `grep -rn "EngineType::Source[^2]" modules/Dumper/` returns nothing
3. Build still compiles (no dangling includes)
4. `Source2/` directory and all its files are untouched
