#pragma once
// ── Profiles/UE5_7Profile.h ───────────────────────────────────────
// Skeleton -- offset UE5.7 belum diisi (TODO).
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::unrealengine {

class UE5_7Profile : public IEngineProfile {
public:
    std::string version() const override { return "5.7"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // TODO: isi offset hasil signature scan UE5.7
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // TODO
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8; // TODO: cek .pak header UE5.7
    }
};

} // namespace omnibyte::dumper::unrealengine