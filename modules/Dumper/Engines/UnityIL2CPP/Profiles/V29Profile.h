#pragma once
// ── Profiles/V29Profile.h ───────────────────────────────────────
// Skeleton -- offset metadata v29 belum diisi (TODO).
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::unityil2cpp {

class V29Profile : public IEngineProfile {
public:
    std::string version() const override { return "29"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // TODO: isi offset spesifik metadata v29
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // TODO
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8; // TODO: cek magic 0xAF1BB1FA + versionField == 29
    }
};

} // namespace omnibyte::dumper::unityil2cpp