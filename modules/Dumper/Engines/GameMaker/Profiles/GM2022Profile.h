#pragma once
// ── Profiles/GM2022Profile.h ──────────────────────────────────────
// Skeleton -- offset GameMaker 2022 belum diisi (TODO).
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::gamemaker {

class GM2022Profile : public IEngineProfile {
public:
    std::string version() const override { return "2022"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // TODO: isi offset data.win chunk 2022
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // TODO
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8; // TODO: cek magic data.win 2022
    }
};

} // namespace omnibyte::dumper::gamemaker