#pragma once
// ── Profiles/Godot4_6Profile.h ────────────────────────────────────
// PCK format version V3/V4 (PACK_FORMAT_VERSION_V3 = 3, PACK_FORMAT_VERSION_V4 = 4).
// Source: core/io/file_access_pack.cpp @ 4.6-stable, struct PackedSourcePCK::try_open_pack
// V3/V4 header: magic(4) + version(4) + major(4) + minor(4) + patch(4) + flags(4) + file_base(8) + dir_offset(8) = 40 bytes
// V4 adds optional 32-byte salt AFTER header (only when sparse_bundle && enc_directory).
// V4 adds PACK_SPARSE_BUNDLE flag and PACK_FILE_DELTA flag.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::godot {

class Godot4_6Profile : public IEngineProfile {
public:
    std::string version() const override { return "4.6"; }

    uint64_t offsetOf(const std::string& key) const override {
        // PCK V3/V4 header fields — same as 4.4/4.5 V3 layout
        // src: core/io/file_access_pack.cpp @ 4.6-stable, PackedSourcePCK::try_open_pack()
        if (key == "PckMagic")         return 0x00; // uint32_t, always 0x43504447 ("GDPC")
        if (key == "PckVersion")       return 0x04; // uint32_t, pack format version (3 or 4)
        if (key == "PckVerMajor")      return 0x08; // uint32_t, engine major version
        if (key == "PckVerMinor")      return 0x0C; // uint32_t, engine minor version
        if (key == "PckVerPatch")      return 0x10; // uint32_t, engine patch (read but not validated)
        if (key == "PckFlags")         return 0x14; // uint32_t, pack_flags bitfield
        if (key == "PckFileBase")      return 0x18; // uint64_t, base offset for file data
        if (key == "PckDirOffset")     return 0x20; // uint64_t, offset to directory
        if (key == "PckFileCount")     return 0x00; // NOT in header — located at dir_offset
        // V4 salt: 32 bytes, read AFTER header at position header_size (0x28), only when sparse_bundle && enc_directory
        if (key == "PckSalt")          return 0x28; // uint8_t[32], V4 only (conditional)

        // File table entry fields (relative to start of each entry)
        if (key == "PckEntryPathLen")  return 0x00; // uint32_t, path string length in bytes

        return 0; // unknown key
    }

    size_t structSize(const std::string& key) const override {
        // src: core/io/file_access_pack.cpp @ 4.6-stable
        if (key == "PckHeader")    return 0x28; // 40 bytes: V3/V4 header (same base as 4.4/4.5)
        if (key == "PckSalt")      return 0x20; // 32 bytes: V4 salt (conditional, only when sparse_bundle && enc_directory)
        return 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // src: core/io/file_access_pack.cpp @ 4.6-stable
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
