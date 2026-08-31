#pragma once
// ── Profiles/Godot4_6Profile.h ────────────────────────────────────
// Skeleton -- offset Godot 4.6 belum diisi (TODO).
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::godot {

class Godot4_6Profile : public IEngineProfile {
public:
    std::string version() const override { return "4.6"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // TODO: isi offset .pck file table 4.6
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // TODO
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8; // TODO: cek footer .pck GDPC 4.6
    }
};

} // namespace omnibyte::dumper::godot