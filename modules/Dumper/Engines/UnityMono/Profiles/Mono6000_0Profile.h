#pragma once
// ── Profiles/Mono6000_0Profile.h ──────────────────────────────────
// Skeleton -- offset Unity 6 6000.0 belum diisi (TODO).
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::unitymono {

class Mono6000_0Profile : public IEngineProfile {
public:
    std::string version() const override { return "6000.0"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // TODO: isi offset MonoImage/MonoClass utk 6000.0
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // TODO
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8; // TODO: cek header libmono 6000.0
    }
};

} // namespace omnibyte::dumper::unitymono