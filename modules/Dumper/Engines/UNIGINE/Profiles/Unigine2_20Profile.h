#pragma once
// ── Profiles/Unigine2_20Profile.h ─────────────────────────────────
// Skeleton -- offset UNIGINE 2.20 belum diisi (TODO).
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::unigine {

class Unigine2_20Profile : public IEngineProfile {
public:
    std::string version() const override { return "2.20"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // TODO: isi offset .unisync 2.20
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // TODO
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8; // TODO: cek header .unisync 2.20
    }
};

} // namespace omnibyte::dumper::unigine