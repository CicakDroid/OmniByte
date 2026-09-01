# Batch C: Resolver Logic — Research Report
**Date**: 2026-09-01
**Status**: Research complete, ready for implementation

---

## Common Pattern (from UnityMonoResolver — 345 lines, fully implemented)

All resolvers follow this pattern:
1. Validate target is live process (not file)
2. Read process memory via `utils::readProcessMemory()` or `readProcessBytes()`
3. Use profile offsets or symbol names to locate runtime structures
4. Walk live data structures (chains, arrays, pointers)
5. Fill `DumpResult` with resolved addresses
6. Use `profile->patternFor()` for AOB scan or `profile->symbolFor()` for xdl_sym

Helper pattern from UnityMonoResolver:
```cpp
static bool readProcessMemory(int pid, uintptr_t addr, void* out, size_t size) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/mem", pid);
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    fseek(f, addr, SEEK_SET);
    size_t read = fread(out, 1, size, f);
    fclose(f);
    return read == size;
}
```

---

## 1. UnrealEngineResolver

### Resolution Strategy: AOB Pattern Scan
UE globals (GNames, GObjects, GWorld) don't have fixed addresses — they're resolved via pattern scanning at runtime.

### Profile Usage
- `profile->patternFor("GNamesPattern")` → `AOBPattern` with byte pattern + mask
- `profile->patternFor("GObjectsPattern")` → `AOBPattern`
- `profile->patternFor("GWorldPattern")` → `AOBPattern`

### Resolution Flow
```
1. Read process memory regions from /proc/pid/maps
2. For each global (GNames, GObjects, GWorld):
   a. Get AOBPattern from profile->patternFor(key)
   b. Scan memory region with utils::scanAndExtractRIP()
   c. Extract RIP-relative address from match
   d. Store resolved address in metadata
3. Walk GNames chunk table to resolve FName strings
4. Walk GObjects array to enumerate UObject instances
5. Fill DumpResult with TypeEntry/MethodEntry/FieldEntry
```

### Key Structures (runtime)
| Global | Purpose | Profile Key |
|--------|---------|-------------|
| GNames | FName string table (chunked array) | `"GNamesPattern"` |
| GObjects | UObject array (FUObjectArray) | `"GObjectsPattern"` |
| GWorld | Current UWorld instance | `"GWorldPattern"` |

### GNames Walk
```
GNames → GNames.ChunkTable (pointer array)
  → GNames.ChunkTable[i] → FNameEntry[16384]
    → FNameEntry.StringData → char[]
```

### GObjects Walk
```
GObjects → FUObjectArray.Objects
  → TUObjectArray[i] → UObject*
    → UObject.Name (FName → GNames lookup)
    → UObject.Class (UClass*)
    → UObject.PropertySize
```

### Notes
- Requires live process — pattern scan happens on actual memory
- AOB patterns are engine-version-specific (defined in profiles)
- Memory read via `/proc/pid/mem` (same as UnityMonoResolver)

---

## 2. UnityIL2CPPResolver

### Resolution Strategy: xdl_sym + Cross-Reference
IL2CPP resolves symbols via dynamic library exports (libil2cpp.so).

### Profile Usage
- `profile->symbolFor("il2cpp_class_get_name")` → symbol name for xdl_sym
- `profile->symbolFor("il2cpp_class_get_methods")` → method enumeration
- `profile->patternFor("GlobalMetadataPointer")` → AOB for metadata base

### Resolution Flow
```
1. Open target process handle via xdl_open(pid)
2. Resolve key symbols via xdl_sym:
   - il2cpp_class_get_name, il2cpp_class_get_namespace
   - il2cpp_class_get_methods, il2cpp_class_get_method_count
   - il2cpp_class_get_fields, il2cpp_class_get_field_count
   - il2cpp_class_get_parent, il2cpp_class_get_interfaces
3. Use symbol addresses to call into IL2CPP runtime
4. Walk TypeDefinitions from metadata (cross-ref with analyze() results)
5. Resolve method addresses from MethodPointer table
6. Fill DumpResult with resolved names + addresses
```

### Key Symbols
| Symbol | Purpose |
|--------|---------|
| `il2cpp_class_get_name` | Get class name string |
| `il2cpp_class_get_methods` | Enumerate methods |
| `il2cpp_class_get_fields` | Enumerate fields |
| `il2cpp_method_get_name` | Get method name |
| `il2cpp_method_get_pointer` | Get method native address |
| `il2cpp_field_get_offset` | Get field offset |

### Notes
- Requires live process with libil2cpp.so loaded
- Symbol addresses change per game build (ASLR)
- Cross-reference with analyze() results for complete picture

---

## 3. GameMakerResolver

### Resolution Strategy: yoyorun Library Symbols
GameMaker uses yoyorun library for runtime execution.

### Profile Usage
- `profile->symbolFor("yy_object_new")` → object constructor
- `profile->symbolFor("yy_func_execute")` → function executor
- `profile->offsetOf("RuntimeObject_size")` → runtime object size

### Resolution Flow
```
1. Open target process handle
2. Find yoyorun library in process maps
3. Resolve key symbols:
   - yy_object_new, yy_object_free
   - yy_func_execute, yy_func_create
   - yy_rval_* (runtime value accessors)
4. Walk runtime object table
5. Resolve function addresses from code chunk offsets
6. Fill DumpResult with resolved symbols
```

### Key Symbols
| Symbol | Purpose |
|--------|---------|
| `yy_object_new` | Object instantiation |
| `yy_func_execute` | Function execution |
| `yy_str_create` | String creation |
| `yy_array_create` | Array creation |

### Notes
- GameMaker runtime is less documented than UE/IL2CPP
- Symbol names vary across GM versions
- Some games strip symbols — fallback to pattern scan

---

## 4. GodotResolver

### Resolution Strategy: ClassDB + StringName via xdl
Godot exposes ClassDB and StringName as exported symbols.

### Profile Usage
- `profile->symbolFor("ClassDB::classes")` → class database
- `profile->symbolFor("StringName::setup")` → string name initialization
- `profile->symbolFor("ClassDB::get_class_tag")` → class tag lookup

### Resolution Flow
```
1. Open target process handle
2. Find libgodot library in process maps
3. Resolve key symbols:
   - ClassDB::classes (HashMap of class info)
   - StringName::setup / StringName::intern
   - ClassDB::get_class_tag, ClassDB::get_parent_class
4. Walk ClassDB class map
5. For each class: enumerate methods, properties, signals
6. Fill DumpResult with resolved class/method/property info
```

### Key Symbols
| Symbol | Purpose |
|--------|---------|
| `ClassDB::classes` | Global class database |
| `StringName::setup` | String name table |
| `ClassDB::get_class_tag` | Class unique identifier |
| `ClassDB::get_parent_class` | Inheritance hierarchy |

### Notes
- Godot 4.x has well-defined ClassDB structure
- Symbol names are consistent across Godot versions
- StringName table is global — shared across all classes

---

## 5. Source2Resolver

### Resolution Strategy: Schema System via xdl
Source 2 uses a schema-based resource system.

### Profile Usage
- `profile->symbolFor("ResourceSystem::Cache")` → resource cache
- `profile->symbolFor("Schema::FindClass")` → class schema lookup

### Resolution Flow
```
1. Open target process handle
2. Find resource system library in process maps
3. Resolve key symbols:
   - ResourceSystem::Cache, ResourceSystem::Load
   - Schema::FindClass, Schema::GetClassInfo
   - CMorphData, CVProperty classes
4. Walk schema class registry
5. Resolve class definitions from schema
6. Fill DumpResult with resolved schema info
```

### Notes
- Source 2 schema is self-describing at runtime
- Symbol names may vary by game (Dota 2, CS2, Deadlock)
- Less standardized than UE/Unity

---

## 6. Cocos2dResolver

### Resolution Strategy: Direct Symbol Export (xdl_sym)
Cocos2d resolves symbols directly from native library exports.

### Profile Usage
- `profile->symbolFor("luaL_loadbuffer")` → Lua chunk loader
- `profile->symbolFor("FileUtils::getInstance")` → file system access
- `profile->symbolFor("Director::getInstance")` → game director

### Resolution Flow
```
1. Open target process handle
2. Find libcocos2dlua.so / libcocos2djs.so in process maps
3. Resolve symbols from profile->symbolFor():
   - luaL_loadbuffer, luaL_openlibs, lua_pcall (Lua variant)
   - ScriptingCore::evalString (JS variant)
   - FileUtils::getInstance, Director::getInstance (common)
   - cc::AssetManager::getInstance (Creator)
4. Store resolved addresses in metadata
5. Fill DumpResult with resolved symbol addresses
```

### Key Symbols (per variant)
| Variant | Key Symbols |
|---------|------------|
| **Lua** | `luaL_loadbuffer`, `luaL_openlibs`, `lua_pcall`, `lua_tostring` |
| **JS** | `ScriptingCore::evalString`, `JSContext` |
| **Common** | `FileUtils::getInstance`, `Director::getInstance`, `xxtea_decrypt` |
| **Creator** | `cc::AssetManager::getInstance`, `cc::Game::getInstance` |

### Notes
- Simplest resolver — direct symbol lookup, no complex struct walking
- Symbol addresses are stable per game build
- `offsetOf()` returns 0 for Cocos2d — only `symbolFor()` matters

---

## Summary: Implementation Order

| Order | Resolver | Lines (est.) | Complexity | Key Dependency |
|-------|----------|-------------|------------|----------------|
| 1 | Cocos2dResolver | ~60 | Low | xdl_sym direct lookup |
| 2 | Source2Resolver | ~60 | Low | xdl_sym + schema walk |
| 3 | GodotResolver | ~70 | Low | xdl_sym + ClassDB walk |
| 4 | GameMakerResolver | ~70 | Low | xdl_sym + yoyorun |
| 5 | UnityIL2CPPResolver | ~100 | Medium | xdl_sym + cross-ref with analyze() |
| 6 | UnrealEngineResolver | ~120 | High | AOB scan + GNames/GObjects walk |

All resolvers use `utils::readProcessBytes()` from SharedUtils (Batch A).
