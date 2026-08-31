#pragma once
// ── Profiles/CryEngine5_4Profile.h ────────────────────────────────
// Skeleton -- offset CryEngine 5.4 belum diisi (TODO).
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cryengine {

class CryEngine5_4Profile : public IEngineProfile {
public:
    std::string version() const override { return "5.4"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // TODO: isi offset .pak 5.4
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // TODO
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8; // TODO: cek magic .pak 5.4
    }
};

} // namespace omnibyte::dumper::cryengine