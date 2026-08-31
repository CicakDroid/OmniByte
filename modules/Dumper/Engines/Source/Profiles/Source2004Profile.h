#pragma once
// ── Profiles/Source2004Profile.h ──────────────────────────────────
// Skeleton -- offset Source 2004 belum diisi (TODO).
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::source {

class Source2004Profile : public IEngineProfile {
public:
    std::string version() const override { return "2004"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // TODO: isi offset .bsp lump table 2004
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // TODO
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8; // TODO: cek footer .bsp 2004
    }
};

} // namespace omnibyte::dumper::source