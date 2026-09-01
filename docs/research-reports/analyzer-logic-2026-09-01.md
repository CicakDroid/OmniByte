# Batch B: Analyzer Logic — Research Report
**Date**: 2026-09-01
**Status**: Research complete, ready for implementation

---

## Common Pattern (from UnityMonoAnalyzer — 516 lines, fully implemented)

All analyzers follow this pattern:
1. Validate target type (file vs process)
2. Read file bytes into buffer
3. Validate magic/header bytes via `profile->validate()`
4. Use `profile->offsetOf(key)` to locate struct offsets
5. Walk metadata structures using helper read functions (readU32, readU64, readString)
6. Fill `DumpResult` with `TypeEntry`, `MethodEntry`, `FieldEntry`, `StringEntry`
7. Set `result.success = true` on completion

Helper pattern from UnityMonoAnalyzer:
```cpp
static uint64_t readU64(const vector<uint8_t>& buf, size_t off) {
    if (off + 8 > buf.size()) return 0;
    uint64_t v; memcpy(&v, buf.data()+off, 8); return v;
}
static std::string readString(const vector<uint8_t>& buf, size_t strTableOff, uint32_t idx);
```

---

## 1. UnrealEngineAnalyzer

### File Format: `.pak` (UE4/UE5 Pack File)
- **Magic**: `0x5A6F12E1` (bytes: `E1 12 6F 5A`)
- **Header**: Version (uint32), IndexOffset (uint64), IndexSize (uint32), SHA1 (20 bytes)
- **Index**: Array of `FPakEntry` structs — name hash, offset, size, compression block count

### Key Structures (from profile offsets)
| Struct | Key Fields | Profile Key |
|--------|-----------|-------------|
| FPakHeader | Magic, Version, IndexOffset, IndexSize | `"PakHeader"` |
| FPakEntry | NameHash, Offset, Size, CompressedSize, CompressionMethodIndex | `"PakEntry"` |
| FNameEntry | String offset, hash (GNames table) | `"FNameEntry"` |

### What Analyzer Extracts
- **TypeEntry**: Not directly from .pak — UE types come from GNames/GObjects at runtime
- **StringEntry**: File names from pak index (extract string table)
- **metadata**: `"pakVersion"`, `"indexOffset"`, `"entryCount"`, `"compressionMethods"`

### Notes
- Static analysis of .pak gives file listing, not type/method info
- Type/method dump requires live process (Resolver phase) for GNames/GObjects walk
- Analyzer output is mainly file inventory + pak metadata

---

## 2. UnityIL2CPPAnalyzer

### File Format: `global-metadata.dat`
- **Magic**: `0xAF1BB1FA` (bytes: `FA B1 1B AF`)
- **Header**: Magic (u32), Version (u32), then version-specific offsets
- **Key offsets** (from profile): `stringLiteralOffset`, `stringLiteralDataOffset`, `typeDefinitionsOffset`, `typeDefinitionCount`, `methodDefinitionOffset`, `methodDefinitionCount`, `fieldDefinitionOffset`, `fieldDefinitionCount`, `imageDefinitionOffset`, `imageDefinitionCount`

### Key Structures (from profile `structSize()`)
| Struct | Fields | Profile Key |
|--------|--------|-------------|
| Il2CppTypeDefinition | nameIndex, namespaceIndex, bitfield, genericContainerIndex, parentIndex, declaringTypeIndex, interfacesCount | `"Il2CppTypeDefinition"` |
| Il2CppMethodDefinition | nameIndex, declaringTypeIndex, returnTypeIndex, parameterStart, genericContainerIndex, methodIndex | `"Il2CppMethodDefinition"` |
| Il2CppFieldDefinition | nameIndex, fieldTypeIndex, parentIndex, bitfield | `"Il2CppFieldDefinition"` |
| Il2CppImageDefinition | nameIndex, assemblyIndex, typeStart, typeCount, exportedTypeStart, exportedTypeCount | `"Il2CppImageDefinition"` |
| Il2CppStringLiteral | length, dataIndex | `"Il2CppStringLiteral"` |

### What Analyzer Extracts
- **TypeEntry**: From `Il2CppTypeDefinition` array — name (via string table), typeId (index), parentType
- **MethodEntry**: From `Il2CppMethodDefinition` — name, declaringType, methodIndex
- **FieldEntry**: From `Il2CppFieldDefinition` — name, declaringType, typeName
- **StringEntry**: From string literal table — global-metadata string literals
- **metadata**: `"metadataVersion"`, `"typeCount"`, `"methodCount"`, `"fieldCount"`, `"imageCount"`

### String Resolution
Strings in IL2CPP metadata are stored as index into a string heap. Resolution:
```
stringOffset = readU32(buffer, typeDefOffset + nameIndex_field_offset)
name = readStringFromHeap(buffer, stringLiteralDataOffset + stringOffset)
```

---

## 3. GameMakerAnalyzer

### File Format: `data.win` (GameMaker Data File)
- **Magic**: `"YYYG"` (GM 2.3+) or `"FORM"` (GMS1 / <2.3)
- **Structure**: Header → Chunk table → Individual chunks (GEN8, OPTN, LANG, EXTN, SOND, AAUD, SPTR, TPAG, TEXT, OBJT, ROOM, DAFL, TPAG, CODE, VARI, FUNC, STRG, TXTR, AUDO, BGND, PATH, SCPT, GLOB, FEAT, PSEM, ESCR, SEQN, TAGS, FEDS, FEAT)

### Key Chunks
| Chunk | Content | Dump Value |
|-------|---------|------------|
| **GEN8** | Game version, filename, room count, object count | metadata |
| **STRG** | String table (count + entries: id, value) | StringEntry |
| **OBJT** | Object definitions (name, spriteId, visible, solidity, parentId) | TypeEntry |
| **CODE** | GML bytecode chunks (one per script/event) | MethodEntry |
| **VARI** | Variable declarations per code chunk | FieldEntry |
| **FUNC** | Function name + code offset mapping | MethodEntry |
| **TXTR** | Texture page info | metadata |
| **AUDO** | Audio asset info | metadata |

### GEN8 Header (relative to chunk start)
```
offset 0x00: version (u32)
offset 0x04: filename (string: u32 length + chars)
offset varies: roomCount, objectCount, ...
```

### STRG Structure
```
offset 0x00: count (u32)
Per entry:
  id (u32), length (u32), chars[length] (UTF-8)
```

### What Analyzer Extracts
- **TypeEntry**: From OBJT chunk — object names, parent objects
- **MethodEntry**: From FUNC chunk — function names + code offsets
- **FieldEntry**: From VARI chunk — variable names per code entry
- **StringEntry**: From STRG chunk — all strings
- **metadata**: `"gmVersion"`, `"filename"`, `"objectCount"`, `"roomCount"`, `"stringCount"`

### Notes
- GM 2.3+ uses `"YYYG"` magic; older GM uses `"FORM"` magic
- Chunk parsing: read chunk name (4 bytes) + chunk size (u32), then parse contents
- String encoding: UTF-8 with u32 length prefix

---

## 4. GodotAnalyzer

### File Format: `.pck` (Godot Pack File)
- **Magic**: `"GDPC"` (bytes: `47 44 50 43`)
- **Header**: PCKVersionMajor (u32), PCKVersionMinor (u32), then version-specific
- **File table**: Offset + size + md5 for each packed file

### PCK Header Layout (Godot 4.x)
```
offset 0x00: magic "GDPC" (4 bytes)
offset 0x04: packVersion (u32) — usually 1 or 2
offset 0x08: verMajor (u32), verMinor (u32), verPatch (u32)
offset 0x14: flags (u32)
offset 0x18: fileBaseOffset (u64) — offset to file table
offset 0x20: fileBaseSize (u64)
... (packVersion 2 adds: exportMd5, fileMd5)
```

### File Table Entry
```
path (Pascal string: u32 len + chars)
offset (u64)
size (u64)
md5 (16 bytes)
flags (u32)
```

### What Analyzer Extracts
- **TypeEntry**: Not from .pck — Godot class info comes from ClassDB at runtime
- **StringEntry**: File paths from pack file table
- **metadata**: `"godotVersion"`, `"packVersion"`, `"fileCount"`, `"fileTableOffset"`

### Notes
- Static .pck analysis gives file listing only
- Class/method dump requires live process (ClassDB walk via Resolver)
- Key files to look for in pack: `project.godot`, `*.gdextension`, `*.gdnlib`

---

## 5. Source2Analyzer

### File Format: `.vpk_c` (Source 2 Compiled Resource)
- **Magic**: Resource system magic (version-dependent)
- **Structure**: Block headers → schema definitions → resource data

### Key Structures
| Structure | Purpose |
|-----------|---------|
| ResourceBlockHeader | Magic, version, block offsets |
| SchemaDef | Class name, field count, field definitions |
| FieldDef | Name, type, offset, flags |

### What Analyzer Extracts
- **TypeEntry**: From schema definitions — class names, field counts
- **FieldEntry**: From field definitions — name, type, offset
- **metadata**: `"schemaVersion"`, `"classCount"`, `"resourceCount"`

### Notes
- Source 2 schema is self-describing — class definitions embedded in resource blocks
- Version detection from block header version field
- Less structured than IL2CPP — more ad-hoc per game

---

## 6. Cocos2dAnalyzer

### Detection Signals (static, file-based)
| Signal | Location | Confidence |
|--------|----------|------------|
| `libcocos2dlua.so` | `lib/<abi>/` in APK | High |
| `libcocos2djs.so` | `lib/<abi>/` in APK | High |
| `libgame.so` / `libcocos2dcpp.so` | `lib/<abi>/` in APK | Medium |
| `libcocos.so` | `lib/<abi>/` in APK (Creator) | High |
| `org.cocos2dx.*` classes | DEX in APK | Medium |
| `.lua` / `.luac` files | `assets/` in APK | Medium |
| `.jsc` files | `assets/` in APK (JSB) | Medium |

### What Analyzer Extracts
- **TypeEntry**: Not from static analysis — Cocos2d types come from Lua/JS runtime
- **StringEntry**: Lua script names, asset file paths
- **metadata**: `"variant"` (lua/js/creator), `"libraryFound"`, `"scriptCount"`, `"assetCount"`

### APK Scanning Approach
1. Read APK as ZIP (use standard zip parsing)
2. Scan `lib/` for native libraries matching cocos2d patterns
3. Scan `assets/` for script/asset files
4. Check `classes.dex` for `org.cocos2dx` package prefix

### Notes
- Cocos2d analysis is mainly detection + asset enumeration
- Symbol resolution (luaL_loadbuffer, etc.) happens in Resolver phase
- No complex binary metadata to parse (unlike IL2CPP/UE)

---

## Summary: Implementation Order

| Order | Analyzer | Lines (est.) | Complexity | Key Dependency |
|-------|----------|-------------|------------|----------------|
| 1 | Cocos2dAnalyzer | ~80 | Low | ZIP scan, lib/asset detection |
| 2 | GodotAnalyzer | ~90 | Low | GDPC magic, file table parse |
| 3 | Source2Analyzer | ~70 | Low | Magic check, schema walk |
| 4 | GameMakerAnalyzer | ~120 | Medium | Chunk parsing (GEN8/STRG/OBJT/FUNC) |
| 5 | UnrealEngineAnalyzer | ~80 | Low | .pak header/index parse |
| 6 | UnityIL2CPPAnalyzer | ~150 | High | Metadata header, struct walking, string heap |

All analyzers use `utils::readFileBytes()` from SharedUtils (Batch A).
