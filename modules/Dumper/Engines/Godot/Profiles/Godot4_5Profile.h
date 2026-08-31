#pragma once
// ── Profiles/Godot4_5Profile.h ────────────────────────────────────
// PCK format version V3 (PACK_FORMAT_VERSION_V3 = 3), also accepts V2.
// Source: core/io/file_access_pack.cpp @ 4.5-stable, struct PackedSourcePCK::try_open_pack
// IDENTICAL header layout to 4.4 V3 — same read order, same offsets.
// Difference from 4.4: adds PackedFile.bundle field and sparse_bundle flag,
//             but these don't change the binary header layout.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::godot {

class Godot4_5Profile : public IEngineProfile {
public:
    std::string version() const override { return "4.5"; }

    uint64_t offsetOf(const std::string& key) const override {
        // PCK V3 header fields — IDENTICAL to 4.4 V3 layout
        // src: core/io/file_access_pack.cpp @ 4.5-stable, PackedSourcePCK::try_open_pack()
        // Verified: same read order as 4.4 (magic → version → major → minor → patch → flags → file_base → dir_offset)
        if (key == "PckMagic")         return 0x00; // uint32_t, always 0x43504447 ("GDPC")
        if (key == "PckVersion")       return 0x04; // uint32_t, pack format version (3 for V3)
        if (key == "PckVerMajor")      return 0x08; // uint32_t, engine major version
        if (key == "PckVerMinor")      return 0x0C; // uint32_t, engine minor version
        if (key == "PckVerPatch")      return 0x10; // uint32_t, engine patch (read but not validated)
        if (key == "PckFlags")         return 0x14; // uint32_t, pack_flags bitfield (adds PACK_SPARSE_BUNDLE in 4.5)
        if (key == "PckFileBase")      return 0x18; // uint64_t, base offset for file data
        if (key == "PckDirOffset")     return 0x20; // uint64_t, offset to directory
        if (key == "PckFileCount")     return 0x00; // NOT in header — located at dir_offset

        // File table entry fields (relative to start of each entry)
        if (key == "PckEntryPathLen")  return 0x00; // uint32_t, path string length in bytes

        return 0; // unknown key
    }

    size_t structSize(const std::string& key) const override {
        // src: core/io/file_access_pack.cpp @ 4.5-stable
        if (key == "PckHeader")    return 0x28; // 40 bytes: V3 header (identical to 4.4)
        return 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // src: core/io/file_access_pack.cpp @ 4.5-stable
        if (len < 8) return false;
        uint32_t magic = headerBytes[0] | (headerBytes[1] << 8) |
                         (headerBytes[2] << 16) | (headerBytes[3] << 24);
        uint32_t ver   = headerBytes[4] | (headerBytes[5] << 8) |
                         (headerBytes[6] << 16) | (headerBytes[7] << 24);
        return magic == 0x43504447 && (ver == 2 || ver == 3);
    }
};

} // namespace omnibyte::dumper::godot
