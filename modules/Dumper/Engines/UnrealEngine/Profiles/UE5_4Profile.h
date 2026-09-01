#pragma once
// ── Profiles/UE5_4Profile.h ───────────────────────────────────────
// UE 5.4 — same FChunkedFixedUObjectArray layout as 5.3.
//
// offsetOf() / structSize() / patternFor():
//   Struct layout identical to UE5.3 (no layout changes in 5.4).
//   patternFor() TODO — identical to UE5.3 but source unverified.
//
// Source: Encryqed/Dumper-7 @ main (ObjectArray.cpp, Offsets.h, OffsetFinder.cpp)
//         UE4SS-RE/RE-UE4SS @ main (Signatures.cpp, patternsleuth_bind)
// Coverage: offsetOf()/structSize() fully covered by Dumper-7.
//           patternFor() TODO — needs verified AOB source.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unrealengine {

class UE5_4Profile : public IEngineProfile {
public:
    std::string version() const override { return "5.4"; }

    uint64_t offsetOf(const std::string& key) const override {
        // Same layout as UE5.3 — no structural changes in 5.4
        // src: Dumper-7/ObjectArray.cpp @ main (FChunkedFixedUObjectArrayLayouts[0])
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            {"FChunkedObjects.ObjectsOffset",      0x00},
            {"FChunkedObjects.MaxElementsOffset",  0x10},
            {"FChunkedObjects.NumElementsOffset",  0x14},
            {"FChunkedObjects.MaxChunksOffset",    0x18},
            {"FChunkedObjects.NumChunksOffset",    0x1C},
            {"FName.Size",                         0x08},
            {"FName.NumberOffset",                 0x04},
            {"FName.ComparisonIndexOffset",        0x00},
            {"FFieldClass.NameOffset",             0x00},
            {"FFieldClass.IdOffset",               0x08},
            {"FFieldClass.CastFlagsOffset",        0x10},
            {"FFieldClass.ClassFlagsOffset",       0x18},
            {"FFieldClass.SuperClassOffset",       0x20},
            {"FField.VftOffset",                   0x00},
            {"FField.ClassOffset",                 0x08},
            {"FField.OwnerOffset",                 0x10},
            {"FField.NextOffset",                  0x20},
            {"FField.NameOffset",                  0x28},
            {"FField.FlagsOffset",                 0x30},
            {"UStruct.HasStructBaseChain",         1},
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        static const std::unordered_map<std::string, size_t> kSizes = {
            {"FUObjectItem",                0x10},
            {"FNameEntry",                  0x10},
        };
        auto it = kSizes.find(key);
        return it != kSizes.end() ? it->second : 0;
    }

    std::optional<AOBPattern> patternFor(const std::string& key) const override {
        // TODO: sumber sitasi sebelumnya tidak dapat diverifikasi (404 dikonfirmasi via curl langsung),
        //       perlu riset ulang untuk GNames/GObjects/GWorld AOB patterns.
        //       Sebelumnya dikutip dari Neverdecel/pcileech-memprocfs-mcp @ docs/ue_signatures.md
        //       yang ternyata tidak dapat diakses (404).
        (void)key;
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        return len >= 8;
    }
};

} // namespace omnibyte::dumper::unrealengine
