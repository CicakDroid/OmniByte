#pragma once
// ── Profiles/Source2_2015Profile.h ────────────────────────────────
// Source 2 resource file header — version-agnostic (same across all engine versions).
// Source: ValveResourceFormat/ValveResourceFormat @ master, Resource.cs
// KnownHeaderVersion = 12, header structure identical 2015–2025.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::source2 {

class Source2_2015Profile : public IEngineProfile {
public:
    std::string version() const override { return "2015"; }

    // Source 2 resource header fields (identical across all engine versions).
    // Fixed-offset header: 16 bytes (0x10). Block table position is dynamic
    // via Reader.BaseStream.Position += blockOffset - 8 (not a fixed offset).
    // Block entry: BlockType(4) + Offset(4) + Size(4) = 12 bytes.
    uint64_t offsetOf(const std::string& key) const override {
        if (key == "FileSize")            return 0x00;  // uint32
        if (key == "HeaderVersion")       return 0x04;  // uint16, always 12
        if (key == "Version")             return 0x06;  // uint16, file type version
        if (key == "BlockOffset")         return 0x08;  // uint32, offset to block table (runtime-calculated)
        if (key == "BlockCount")          return 0x0C;  // uint32, number of blocks
        if (key == "BlockEntry.Size")     return 12;    // per-block entry size
        if (key == "BlockEntry.BlockType")return 0;     // +0x00 within entry
        if (key == "BlockEntry.Offset")   return 4;     // +0x04 within entry
        if (key == "BlockEntry.Size")     return 8;     // +0x08 within entry
        return 0;
    }

    size_t structSize(const std::string& key) const override {
        if (key == "ResourceHeader")      return 0x10;  // 16 bytes fixed-offset header
        if (key == "BlockEntry")          return 12;    // 12 bytes per block
        return 0;
    }

    // Validate: check HeaderVersion == 12 (KnownHeaderVersion).
    // Source: ValveResourceFormat Resource.cs — throws if != 12.
    bool validate(const uint8_t* headerBytes, size_t len) const override {
        if (len < 8) return false;
        // HeaderVersion at offset 0x04 (little-endian uint16)
        uint16_t headerVersion = headerBytes[0x04] | (headerBytes[0x05] << 8);
        return headerVersion == 12;
    }
};

} // namespace omnibyte::dumper::source2