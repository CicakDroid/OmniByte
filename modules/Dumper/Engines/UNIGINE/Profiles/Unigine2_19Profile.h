#pragma once
// ── Profiles/Unigine2_19Profile.h ─────────────────────────────────
// Skeleton -- offset UNIGINE 2.19 belum diisi (TODO).
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::unigine {

class Unigine2_19Profile : public IEngineProfile {
public:
    std::string version() const override { return "2.19"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // TODO: isi offset .unisync 2.19
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // TODO
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8; // TODO: cek header .unisync 2.19
    }
};

} // namespace omnibyte::dumper::unigine