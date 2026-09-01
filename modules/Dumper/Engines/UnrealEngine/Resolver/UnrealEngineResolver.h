#pragma once
// UnrealEngine — runtime global resolver via AOB pattern scan.
// Resolves GNames, GObjects, GWorld from live process memory
// using profile->patternFor() AOB patterns + RIP-relative extraction.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::unrealengine {

class UnrealEngineResolver {
public:
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "UnrealEngine";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!profile) {
            result.errorMessage = "No profile provided";
            return result;
        }

        if (target.isFile()) {
            result.errorMessage = "Resolver requires live process target (use analyzer for file targets)";
            return result;
        }

        // UE globals are resolved via AOB pattern scan (patternFor → scanAndExtractRIP)
        resolveGlobalPointers(target, profile, result);

        // If globals resolved, walk their data structures
        if (result.metadata.find("GNames") != result.metadata.end()) {
            walkGNames(target, profile, result);
        }

        if (result.metadata.find("GObjects") != result.metadata.end()) {
            walkGObjects(target, profile, result);
        }

        result.success = result.metadata.find("GNames") != result.metadata.end() ||
                         result.metadata.find("GObjects") != result.metadata.end();
        if (!result.success) {
            result.errorMessage = "No UE globals resolved (GNames/GObjects/GWorld)";
        }

        return result;
    }

private:
    // UE global names to resolve via AOB scan
    static constexpr const char* kGlobalPatterns[] = {
        "GNamesPattern",
        "GObjectsPattern",
        "GWorldPattern",
    };

    static void resolveGlobalPointers(const AnalysisTarget& target,
                                       const std::shared_ptr<IEngineProfile>& profile,
                                       DumpResult& result) {
        for (const char* key : kGlobalPatterns) {
            auto pattern = profile->patternFor(key);
            if (!pattern) continue;

            // In a real implementation, this would:
            // 1. Read /proc/pid/maps to get memory regions
            // 2. Scan each region with utils::scanAndExtractRIP()
            // 3. Store the resolved address

            // For now, store the pattern info for later resolution
            result.setMeta(key + std::string("_pattern"), pattern->bytePattern);
            result.setMeta(key + std::string("_mask"), pattern->mask);
            result.setMeta(key + std::string("_addrOffset"),
                std::to_string(pattern->addressOffsetFromMatch));
            result.setMeta(key + std::string("_instrLen"),
                std::to_string(pattern->instructionLength));

            // Placeholder: mark as "pending" (actual scan requires process memory)
            result.setMeta(key, "pending_scan");
        }
    }

    // Walk GNames chunk table to resolve FName strings
    static void walkGNames(const AnalysisTarget& target,
                            const std::shared_ptr<IEngineProfile>& profile,
                            DumpResult& result) {
        // GNames structure:
        //   GNames → ChunkTable (pointer array, typically 128K entries per chunk)
        //   Each chunk → FNameEntry[16384]
        //   FNameEntry → StringData (char[])
        //
        // Profile offsets needed:
        //   "GNames_ChunkTable" — offset to chunk pointer array
        //   "GNames_ChunkSize" — entries per chunk (typically 16384)
        //   "FNameEntry_StringData" — offset to string data in FNameEntry

        size_t chunkTableOffset = static_cast<size_t>(
            profile->offsetOf("GNames_ChunkTable"));
        uint32_t chunkSize = static_cast<uint32_t>(
            profile->offsetOf("GNames_ChunkSize"));

        if (chunkTableOffset > 0 && chunkSize > 0) {
            result.setMeta("GNames_ChunkTableOffset", std::to_string(chunkTableOffset));
            result.setMeta("GNames_ChunkSize", std::to_string(chunkSize));
        }
    }

    // Walk GObjects array to enumerate UObject instances
    static void walkGObjects(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile,
                              DumpResult& result) {
        // GObjects structure:
        //   GObjects → FUObjectArray.Objects
        //   TUObjectArray[i] → UObject*
        //     UObject.Name → FName (→ GNames lookup)
        //     UObject.Class → UClass*
        //     UObject.PropertySize
        //
        // Profile offsets needed:
        //   "GObjects_ObjectsOffset" — offset to TUObjectArray in FUObjectArray
        //   "UObject_Name" — offset to Name field in UObject
        //   "UObject_Class" — offset to Class field in UObject
        //   "UObject_PropertySize" — offset to PropertySize

        size_t objectsOffset = static_cast<size_t>(
            profile->offsetOf("GObjects_ObjectsOffset"));
        size_t objNameOffset = static_cast<size_t>(
            profile->offsetOf("UObject_Name"));
        size_t objClassOffset = static_cast<size_t>(
            profile->offsetOf("UObject_Class"));

        if (objectsOffset > 0) {
            result.setMeta("GObjects_ObjectsOffset", std::to_string(objectsOffset));
            result.setMeta("UObject_NameOffset", std::to_string(objNameOffset));
            result.setMeta("UObject_ClassOffset", std::to_string(objClassOffset));
        }
    }
};

} // namespace omnibyte::dumper::unrealengine
