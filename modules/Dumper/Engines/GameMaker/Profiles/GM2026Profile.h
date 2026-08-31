#pragma once
// ── Profiles/GM2026Profile.h ──────────────────────────────────────
// Skeleton -- offset GameMaker 2026.0 belum diisi (TODO).
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::gamemaker {

class GM2026Profile : public IEngineProfile {
public:
    std::string version() const override { return "2026.0"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // TODO: isi offset data.win chunk 2026.0
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // TODO
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8; // TODO: cek magic data.win 2026.0
    }
};

} // namespace omnibyte::dumper::gamemaker