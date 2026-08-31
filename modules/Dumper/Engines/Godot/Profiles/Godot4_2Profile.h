#pragma once
// ── Profiles/Godot4_2Profile.h ────────────────────────────────────
// PCK format version V2 (PACK_FORMAT_VERSION = 2).
// Source: core/io/file_access_pack.cpp @ 4.2-stable, struct PackedSourcePCK::try_open_pack
// Header size: 100 bytes (V2 layout: magic + version fields + flags + file_base + reserved[16] + file_count)
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::godot {

class Godot4_2Profile : public IEngineProfile {
public:
    std::string version() const override { return "4.2"; }

    uint64_t offsetOf(const std::string& key) const override {
        // PCK header fields — all offsets from start of header (byte 0 = magic "GDPC")
        // src: core/io/file_access_pack.cpp @ 4.2-stable, PackedSourcePCK::try_open_pack()
        if (key == "PckMagic")         return 0x00; // uint32_t, always 0x43504447 ("GDPC")
        if (key == "PckVersion")       return 0x04; // uint32_t, pack format version (2 for V2)
        if (key == "PckVerMajor")      return 0x08; // uint32_t, engine major version
        if (key == "PckVerMinor")      return 0x0C; // uint32_t, engine minor version
        if (key == "PckVerPatch")      return 0x10; // uint32_t, engine patch (read but not validated)
        if (key == "PckFlags")         return 0x14; // uint32_t, pack_flags bitfield
        if (key == "PckFileBase")      return 0x18; // uint64_t, base offset for file data
        if (key == "PckReserved")      return 0x20; // 16 × uint32_t = 64 bytes reserved
        if (key == "PckFileCount")     return 0x60; // uint32_t, number of files in pack

        // File table entry fields (relative to start of each entry)
        // src: core/io/file_access_pack.cpp @ 4.2-stable, file table read loop
        if (key == "PckEntryPathLen")  return 0x00; // uint32_t, path string length in bytes
        // path data follows at +4, length = path_len bytes
        // Entry size is variable: 4 (path_len) + path_len + 8 (offset) + 8 (size) + 16 (md5) + 4 (flags)
        if (key == "PckEntryFileOffset") return 0x00; // variable — use path_len at entry+0, then +4+path_len
        if (key == "PckEntryFileSize")   return 0x00; // variable — see above
        if (key == "PckEntryMD5")        return 0x00; // variable — see above
        if (key == "PckEntryFlags")      return 0x00; // variable — see above

        return 0; // unknown key
    }

    size_t structSize(const std::string& key) const override {
        // src: core/io/file_access_pack.h @ 4.2-stable, PACK_FORMAT_VERSION = 2
        if (key == "PckHeader")    return 0x64; // 100 bytes: V2 header (magic through file_count)
        if (key == "PckReserved")  return 0x40; // 64 bytes: 16 × uint32_t reserved area
        // File entry is variable-length (40 bytes fixed + path_len), no fixed structSize
        return 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // src: core/io/file_access_pack.cpp @ 4.2-stable
        // magic = 0x43504447 ("GDPC"), version must == PACK_FORMAT_VERSION (2)
        if (len < 8) return false;
        uint32_t magic = headerBytes[0] | (headerBytes[1] << 8) |
                         (headerBytes[2] << 16) | (headerBytes[3] << 24);
        uint32_t ver   = headerBytes[4] | (headerBytes[5] << 8) |
                         (headerBytes[6] << 16) | (headerBytes[7] << 24);
        return magic == 0x43504447 && ver == 2;
    }
};

} // namespace omnibyte::dumper::godot
