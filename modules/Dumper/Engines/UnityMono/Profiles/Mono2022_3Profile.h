#pragma once
// ── Profiles/Mono2022_3Profile.h ──────────────────────────────────
// Satu file per versi SDK yang secara eksplisit didukung.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unitymono {

class Mono2022_3Profile : public IEngineProfile {
public:
    std::string version() const override { return "2022.3"; } // versi terlawas yg masih didukung (LTS)
    uint64_t offsetOf(const std::string& key) const override {
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            {"MonoImageOffset",   0x0}, // TODO -- beda dari 2022.3
            {"MonoClassOffset",   0x0},
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }
    size_t structSize(const std::string& key) const override { return 0; }
    bool validate(const uint8_t* headerBytes, size_t len) const override { return len >= 8; }
};

} // namespace omnibyte::dumper::unitymono