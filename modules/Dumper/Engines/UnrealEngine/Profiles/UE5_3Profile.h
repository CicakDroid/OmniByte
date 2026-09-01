#pragma once
// ── Profiles/UE5_3Profile.h ───────────────────────────────────────
// UE 5.3 — oldest supported version.
//
// offsetOf() keys (struct layout statis):
//   "FChunkedObjects.ObjectsOffset"      = 0x00
//   "FChunkedObjects.MaxElementsOffset"  = 0x10
//   "FChunkedObjects.NumElementsOffset"  = 0x14
//   "FChunkedObjects.MaxChunksOffset"    = 0x18
//   "FChunkedObjects.NumChunksOffset"    = 0x1C
//   "FName.Size"                         = 0x08 (default config)
//   "FName.NumberOffset"                 = 0x04
//   "FFieldClass.CastFlagsOffset"        = 0x10
//   "UStruct.StructBaseChain"            = present (added in 5.3)
//
// patternFor() keys (AOB untuk runtime resolve):
//   "GNamesPattern", "GObjectsPattern", "GWorldPattern"
//   TODO: semua pattern dikosongkan — sitasi sebelumnya (Neverdecel/pcileech-memprocfs-mcp)
//         tidak dapat diverifikasi (404 dikonfirmasi via curl langsung), perlu riset ulang.
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

class UE5_3Profile : public IEngineProfile {
public:
    std::string version() const override { return "5.3"; }

    uint64_t offsetOf(const std::string& key) const override {
        // FChunkedFixedUObjectArray layout — default UE4.21–UE5.7
        // src: Dumper-7/ObjectArray.cpp @ main, FChunkedFixedUObjectArrayLayouts[0]
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            {"FChunkedObjects.ObjectsOffset",      0x00},
            {"FChunkedObjects.MaxElementsOffset",  0x10},
            {"FChunkedObjects.NumElementsOffset",  0x14},
            {"FChunkedObjects.MaxChunksOffset",    0x18},
            {"FChunkedObjects.NumChunksOffset",    0x1C},
            // FName config — default (no CasePreserving, no OutlineNumber)
            // src: Dumper-7/OffsetFinder.cpp @ main, InitFNameSettings()
            {"FName.Size",                         0x08},
            {"FName.NumberOffset",                 0x04},
            {"FName.ComparisonIndexOffset",        0x00},
            // FFieldClass layout
            // src: Dumper-7/Offsets.h @ main, FFieldClass namespace
            {"FFieldClass.NameOffset",             0x00},
            {"FFieldClass.IdOffset",               0x08},
            {"FFieldClass.CastFlagsOffset",        0x10},
            {"FFieldClass.ClassFlagsOffset",       0x18},
            {"FFieldClass.SuperClassOffset",       0x20},
            // FField layout
            // src: Dumper-7/Offsets.h @ main, FField namespace
            {"FField.VftOffset",                   0x00},
            {"FField.ClassOffset",                 0x08},
            {"FField.OwnerOffset",                 0x10},
            {"FField.NextOffset",                  0x20},
            {"FField.NameOffset",                  0x28},
            {"FField.FlagsOffset",                 0x30},
            // UStruct.StructBaseChain — present since UE5.3
            // src: Dumper-7/OffsetFinder.cpp @ main, FindStructBaseChainOffset()
            {"UStruct.HasStructBaseChain",         1},
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        // FUObjectItem size for chunked array: sizeof(void*) + sizeof(int32) + sizeof(int32)
        // src: Dumper-7/ObjectArray.cpp @ main, InitializeFUObjectItem()
        static const std::unordered_map<std::string, size_t> kSizes = {
            {"FUObjectItem",                0x10},  // 8 + 4 + 4 on 64-bit
            {"FNameEntry",                  0x10},  // default FName pool entry (8-byte header + string)
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

    bool validate(const uint8_t* headerBytes, size_t len) const override { return len >= 8; }
};

} // namespace omnibyte::dumper::unrealengine
