#pragma once
// ── Profiles/Source2_2020Profile.h ────────────────────────────────
// Source 2 resource file header — version-agnostic (same across all engine versions).
// Source: ValveResourceFormat/ValveResourceFormat @ master, Resource.cs
// KnownHeaderVersion = 12, header structure identical 2015–2025.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::source2 {

class Source2_2020Profile : public IEngineProfile {
public:
    std::string version() const override { return "2020"; }

    uint64_t offsetOf(const std::string& key) const override {
        if (key == "FileSize")            return 0x00;  // uint32
        if (key == "HeaderVersion")       return 0x04;  // uint16, always 12
        if (key == "Version")             return 0x06;  // uint16, file type version
        if (key == "BlockOffset")         return 0x08;  // uint32, offset to block table
        if (key == "BlockCount")          return 0x0C;  // uint32, number of blocks
        if (key == "HeaderSize")          return 0x14;  // total header size (20 bytes)
        if (key == "BlockEntry.Size")     return 12;    // per-block entry size
        if (key == "BlockEntry.BlockType")return 0;
        if (key == "BlockEntry.Offset")   return 4;
        if (key == "BlockEntry.Size")     return 8;
        return 0;
    }

    size_t structSize(const std::string& key) const override {
        if (key == "ResourceHeader")      return 0x14;
        if (key == "BlockEntry")          return 12;
        return 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        if (len < 8) return false;
        uint16_t headerVersion = headerBytes[0x04] | (headerBytes[0x05] << 8);
        return headerVersion == 12;
    }
};

} // namespace omnibyte::dumper::source2