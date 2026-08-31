#pragma once
// ── Profiles/Source2_2015Profile.h ────────────────────────────────
// Skeleton -- offset Source 2 2015 belum diisi (TODO).
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::source2 {

class Source2_2015Profile : public IEngineProfile {
public:
    std::string version() const override { return "2015"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // TODO: isi offset header .vpk_c 2015
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // TODO
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8; // TODO: cek header .vpk_c 2015
    }
};

} // namespace omnibyte::dumper::source2