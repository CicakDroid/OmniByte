# Source 2 Engine Offset Research Report

**Date:** 2026-09-01
**Engine:** Valve Source 2
**Purpose:** Define struct offsets for Source 2 resource files (.vpk_c, .vpcf_c, etc.)
**Status:** Research complete — limited applicability for version-specific offsets

---

## 1. Executive Summary

ValveResourceFormat/ValveResourceFormat (Source 2 Viewer) is a **well-maintained community parser** (2405 stars, MIT license, pushed 2026-09-01) for Source 2 compiled resource files. However, **the resource file header format is version-agnostic** — the same 12-byte header version is used across all Source 2 engine versions. Version-specific differences are in the DATA block content, not the header structure.

**Key finding:** Source 2 resource file headers do NOT vary by engine version (2015/2020/2023/2025). The header format is fixed. This means there are no meaningful per-version offset differences to fill in the Profile files.

---

## 2. Source 2 Resource File Header Format

### 2.1 Header Structure (All Versions)

The resource file header is identical across all Source 2 versions:

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x00 | 4 | FileSize | Total file size (uint32) |
| 0x04 | 2 | HeaderVersion | Always 12 (KnownHeaderVersion) |
| 0x06 | 2 | Version | File type version (uint16) |
| 0x08 | 4 | BlockOffset | Offset to block table from start |
| 0x0C | 4 | BlockCount | Number of blocks |

**Source:** ValveResourceFormat/ValveResourceFormat @ master, `ValveResourceFormat/Resource/Resource.cs`

**RAW OUTPUT (fetched 2026-09-01):**
```csharp
// From Resource.cs Read() method:
FileSize = Reader.ReadUInt32();           // offset 0x00
HeaderVersion = Reader.ReadUInt16();      // offset 0x04
Version = Reader.ReadUInt16();            // offset 0x06
var blockOffset = Reader.ReadUInt32();    // offset 0x08
var blockCount = Reader.ReadUInt32();     // offset 0x0C
// Fixed-offset header: 16 bytes (0x10).
// Block table position is DYNAMIC: Reader.BaseStream.Position += blockOffset - 8
```

### 2.2 Block Table Entry Format

Each block entry in the table is 12 bytes:

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| +0x00 | 4 | BlockType | FourCC block type identifier |
| +0x04 | 4 | Offset | Offset to block data |
| +0x08 | 4 | Size | Size of block data |

**RAW OUTPUT (fetched 2026-09-01):**
```csharp
// From Resource.cs Read() method:
var blockType = (BlockType)Reader.ReadUInt32();
var position = Reader.BaseStream.Position;
var offset = (uint)position + Reader.ReadUInt32();
var size = Reader.ReadUInt32();
```

### 2.3 Known Block Types

| Block Type | Description |
|------------|-------------|
| DATA | Resource data (type-specific content) |
| RERL | External references list |
| REDI | Resource edit info (v1) |
| RED2 | Resource edit info (v2) |
| NTRO | Resource introspection manifest |
| VBIB | Vertex buffer info block |
| MBUF | Mesh buffer |
| TBUF | Texture buffer |
| MVTX | Mesh vertex buffer |
| MIDX | Mesh index buffer |
| MADJ | Mesh adjacency buffer |
| MSLT | Meshlet buffer |
| CTRL | Control data |
| MDAT | Mesh data |
| INSG | Instance data |
| SrMa | Source map |
| LaCo | Layout compiler |
| STAT | Statistics |
| FLCI | Frame list |
| DSTF | Distance field |
| MRPH | Morph data |
| ANIM | Animation data |
| ASEQ | Animation sequence |
| AGRP | Animation group |
| PHYS | Physics data |
| SPRV | Shader parameter |

**Source:** ValveResourceFormat/ValveResourceFormat @ master, `ValveResourceFormat/Resource/Resource.cs` ConstructFromType() method

---

## 3. Version Detection

### 3.1 Header Version

The `HeaderVersion` field is always 12 across all Source 2 resource files. This is a compile-time constant:

```csharp
public const ushort KnownHeaderVersion = 12;
```

If the header version is not 12, the parser throws:
```csharp
throw new UnexpectedMagicException(
    $"Unexpected header. You likely tried to read a file that is not actually a resource...",
    HeaderVersion, nameof(HeaderVersion));
```

### 3.2 File Type Version

The `Version` field (uint16 at offset 0x06) indicates the specific resource type version, but this is **per resource type**, not per engine version. For example, a texture resource in Source 2 2015 and 2025 may have different `Version` values, but the header structure is the same.

### 3.3 Engine Version Detection

**Source 2 engine version is NOT determined from the resource file header.** The same resource file format is used across all Source 2 engine versions (2015–2025). Engine version is typically determined by:
- Game-specific file paths and naming conventions
- Content within DATA blocks (model formats, material definitions)
- External metadata (game manifests, package files)

---

## 4. Applicability to Profile Files

### 4.1 What CAN Be Filled

Since the header format is identical across versions, the following offsets are **version-agnostic** and apply to ALL Source 2 versions:

| Key | Offset | Size | Description |
|-----|--------|------|-------------|
| "Resource.FileSize" | 0x00 | 4 | File size field |
| "Resource.HeaderVersion" | 0x04 | 2 | Header version (always 12) |
| "Resource.Version" | 0x06 | 2 | File type version |
| "Resource.BlockOffset" | 0x08 | 4 | Block table offset (used to locate block table at runtime) |
| "Resource.BlockCount" | 0x0C | 4 | Number of blocks |
| "Resource.BlockEntry.Size" | - | 12 | Size of one block entry |

**Note:** Only the first 16 bytes (0x10) have fixed offsets. The block table position is NOT fixed — it is calculated at runtime via `Reader.BaseStream.Position += blockOffset - 8` using the `blockOffset` field value.

### 4.2 What CANNOT Be Filled (Version-Specific)

The following are **NOT available** from ValveResourceFormat because they vary by content type, not engine version:
- DATA block internal structure (varies by ResourceType)
- NTRO introspection data (varies by type)
- Resource-specific field offsets within DATA blocks

---

## 5. Source Repository Status

| Attribute | Value |
|-----------|-------|
| Repository | ValveResourceFormat/ValveResourceFormat |
| Stars | 2405 |
| License | MIT |
| Language | C# |
| Last Push | 2026-09-01T00:27:04Z |
| Default Branch | master |
| Active | Yes (pushed today) |

**URL:** https://github.com/ValveResourceFormat/ValveResourceFormat

---

## 6. Sources

| Claim | Source | URL |
|-------|--------|-----|
| Resource header format | ValveResourceFormat Resource.cs @ master | https://github.com/ValveResourceFormat/ValveResourceFormat/blob/master/ValveResourceFormat/Resource/Resource.cs |
| KnownHeaderVersion = 12 | ValveResourceFormat Resource.cs @ master | https://github.com/ValveResourceFormat/ValveResourceFormat/blob/master/ValveResourceFormat/Resource/Resource.cs |
| Block type definitions | ValveResourceFormat Resource.cs @ master | https://github.com/ValveResourceFormat/ValveResourceFormat/blob/master/ValveResourceFormat/Resource/Resource.cs |
| Repo activity/status | GitHub API response 2026-09-01 | https://api.github.com/repos/ValveResourceFormat/ValveResourceFormat |

---

## 7. Per-Version Assessment

| Version | Status | Reason |
|---------|--------|--------|
| 2015 | Header offsets applicable | Same header format, version-agnostic |
| 2020 | Header offsets applicable | Same header format, version-agnostic |
| 2023 | Header offsets applicable | Same header format, version-agnostic |
| 2025 | Header offsets applicable | Same header format, version-agnostic |

**Note:** All four versions use the identical header format. The `Version` field (offset 0x06) varies by resource type, not engine version.

---

## 8. Recommendation

**For Profile files:** Fill `offsetOf()` with the version-agnostic header field offsets (0x00, 0x04, 0x06, 0x08, 0x0C). Leave `structSize()` as TODO for DATA block internals since those are resource-type-specific, not engine-version-specific. Do NOT expose a static "HeaderSize" key — only 16 bytes are fixed-offset, and the block table position is dynamic.

**Important clarification:** The Profile files' current design assumes per-version offsets for "Source 2 2015" vs "Source 2 2025", but the resource header format does NOT change between these versions. The profiles should either:
1. Use a single shared offset set for all versions (recommended), or
2. Acknowledge that version-specific differentiation happens at the DATA block level, not the header level
