#pragma once
// ── Profiles/Godot4_7Profile.h ────────────────────────────────────
// PCK format version V3/V4 (PACK_FORMAT_VERSION_V3 = 3, PACK_FORMAT_VERSION_V4 = 4).
// Source: core/io/file_access_pack.cpp @ 4.7-stable, struct PackedSourcePCK::try_open_pack
// IDENTICAL header layout to 4.6 V3/V4 — same read order, same offsets.
// Differences from 4.6: adds per-file salt via PackedFile.salt, adds decryption key parameter,
//             but these don't change the binary header layout.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::godot {

class Godot4_7Profile : public IEngineProfile {
public:
    std::string version() const override { return "4.7"; }

    uint64_t offsetOf(const std::string& key) const override {
        // PCK V3/V4 header fields — IDENTICAL to 4.6 V3/V4 layout
        // src: core/io/file_access_pack.cpp @ 4.7-stable, PackedSourcePCK::try_open_pack()
        // Verified: same read order as 4.6 (magic → version → major → minor → patch → flags → file_base → dir_offset)
        if (key == "PckMagic")         return 0x00; // uint32_t, always 0x43504447 ("GDPC")
        if (key == "PckVersion")       return 0x04; // uint32_t, pack format version (3 or 4)
        if (key == "PckVerMajor")      return 0x08; // uint32_t, engine major version
        if (key == "PckVerMinor")      return 0x0C; // uint32_t, engine minor version
        if (key == "PckVerPatch")      return 0x10; // uint32_t, engine patch (read but not validated)
        if (key == "PckFlags")         return 0x14; // uint32_t, pack_flags bitfield
        if (key == "PckFileBase")      return 0x18; // uint64_t, base offset for file data
        if (key == "PckDirOffset")     return 0x20; // uint64_t, offset to directory
        if (key == "PckFileCount")     return 0x00; // NOT in header — located at dir_offset
        // V4 salt: 32 bytes, read AFTER header at position 0x28, only when sparse_bundle && enc_directory
        if (key == "PckSalt")          return 0x28; // uint8_t[32], V4 only (conditional)

        // File table entry fields (relative to start of each entry)
        if (key == "PckEntryPathLen")  return 0x00; // uint32_t, path string length in bytes

        return 0; // unknown key
    }

    size_t structSize(const std::string& key) const override {
        // src: core/io/file_access_pack.cpp @ 4.7-stable
        if (key == "PckHeader")    return 0x28; // 40 bytes: V3/V4 header (identical to 4.6)
        if (key == "PckSalt")      return 0x20; // 32 bytes: V4 salt (conditional, only when sparse_bundle && enc_directory)
        return 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // src: core/io/file_access_pack.cpp @ 4.7-stable
        // Accepts V4 (4), V3 (3), or V2 (2)
        if (len < 8) return false;
        uint32_t magic = headerBytes[0] | (headerBytes[1] << 8) |
                         (headerBytes[2] << 16) | (headerBytes[3] << 24);
        uint32_t ver   = headerBytes[4] | (headerBytes[5] << 8) |
                         (headerBytes[6] << 16) | (headerBytes[7] << 24);
        return magic == 0x43504447 && (ver == 2 || ver == 3 || ver == 4);
    }
};

} // namespace omnibyte::dumper::godot
