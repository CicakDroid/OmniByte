#pragma once
// ── Profiles/GM2024Profile.h ──────────────────────────────────────
// Skeleton -- offset GameMaker 2024 belum diisi (TODO).
// Catatan: prefix "2024" juga match "2024.14" -- resolveProfile harus
// mengecek "2024.14" SEBELUM branch ini.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::gamemaker {

class GM2024Profile : public IEngineProfile {
public:
    std::string version() const override { return "2024"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // TODO: isi offset data.win chunk 2024
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // TODO
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8; // TODO: cek magic data.win 2024
    }
};

} // namespace omnibyte::dumper::gamemaker