#pragma once
// ── Profiles/Godot4_7Profile.h ────────────────────────────────────
// Satu file per versi SDK yang secara eksplisit didukung.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::godot {

class Godot4_7Profile : public IEngineProfile {
public:
    std::string version() const override { return "4.7"; } // versi terbaru saat ini
    uint64_t offsetOf(const std::string& key) const override { return 0; } // TODO
    size_t structSize(const std::string& key) const override { return 0; }
    bool validate(const uint8_t* headerBytes, size_t len) const override { return len >= 8; }
};

} // namespace omnibyte::dumper::godot