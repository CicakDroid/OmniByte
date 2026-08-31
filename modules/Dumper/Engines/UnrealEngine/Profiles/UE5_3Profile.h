#pragma once
// ── Profiles/UE5_3Profile.h ───────────────────────────────────────
// Satu file per versi SDK yang secara eksplisit didukung.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unrealengine {

class UE5_3Profile : public IEngineProfile {
public:
    std::string version() const override { return "5.3"; } // versi terlawas yg masih didukung
    uint64_t offsetOf(const std::string& key) const override {
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            {"GNamesOffset",     0x0},  // TODO: isi hasil signature scan UE5.3
            {"GObjectsOffset",   0x0},
            {"GWorldOffset",     0x0},
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }
    size_t structSize(const std::string& key) const override {
        if (key == "FNameEntry") return 0x0; // TODO
        return 0;
    }
    bool validate(const uint8_t* headerBytes, size_t len) const override { return len >= 8; }
};

} // namespace omnibyte::dumper::unrealengine