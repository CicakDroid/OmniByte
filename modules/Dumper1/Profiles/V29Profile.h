#pragma once
// Dumper1 — IL2CPP v29 profile (Unity 2021.2+).
// Source: il2cpp-dumper-rs metadata.rs + Perfare/Il2CppDumper MetadataClass.cs
// Header: DIFFERENT from V27 — attributesInfo removed, attributeData added
#include "../../Dumper/DumperCore/IEngineProfile.h"
#include <string>
#include <cstdint>
#include <cstring>
#include <optional>
#include <unordered_map>

namespace omnibyte::dumper::dumper1 {

class V29Profile : public IEngineProfile {
public:
    std::string version() const override { return "29"; }

    uint64_t offsetOf(const std::string& key) const override {
        // V29 header: no attributesInfo (Max=27.2), has attributeData (Min=29)
        // Source: il2cpp-dumper-rs metadata.rs + Il2CppDumper MetadataClass.cs
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            {"sanity",                            0x00},
            {"version",                           0x04},
            {"stringLiteralOffset",               0x08},
            {"stringLiteralSize",                 0x0C},
            {"stringLiteralDataOffset",           0x10},
            {"stringLiteralDataSize",             0x14},
            {"stringOffset",                      0x18},
            {"stringSize",                        0x1C},
            {"eventsOffset",                      0x20},
            {"eventsSize",                        0x24},
            {"propertiesOffset",                  0x28},
            {"propertiesSize",                    0x2C},
            {"methodsOffset",                     0x30},
            {"methodsSize",                       0x34},
            {"parameterDefaultValuesOffset",      0x38},
            {"parameterDefaultValuesSize",        0x3C},
            {"fieldDefaultValuesOffset",          0x40},
            {"fieldDefaultValuesSize",            0x44},
            {"fieldAndParameterDefaultValueDataOffset", 0x48},
            {"fieldAndParameterDefaultValueDataSize",   0x4C},
            {"fieldMarshaledSizesOffset",         0x50},
            {"fieldMarshaledSizesSize",           0x54},
            {"parametersOffset",                  0x58},
            {"parametersSize",                    0x5C},
            {"fieldsOffset",                      0x60},
            {"fieldsSize",                        0x64},
            {"genericParametersOffset",           0x68},
            {"genericParametersSize",             0x6C},
            {"genericParameterConstraintsOffset", 0x70},
            {"genericParameterConstraintsSize",   0x74},
            {"genericContainersOffset",           0x78},
            {"genericContainersSize",             0x7C},
            {"nestedTypesOffset",                 0x80},
            {"nestedTypesSize",                   0x84},
            {"interfacesOffset",                  0x88},
            {"interfacesSize",                    0x8C},
            {"vtableMethodsOffset",               0x90},
            {"vtableMethodsSize",                 0x94},
            {"interfaceOffsetsOffset",            0x98},
            {"interfaceOffsetsSize",              0x9C},
            {"typeDefinitionsOffset",             0xA0},
            {"typeDefinitionsSize",               0xA4},
            // V29: no rgctx (29 > 24.1), no metadataUsage (29 > 24.5)
            {"imagesOffset",                      0xA8},
            {"imagesSize",                        0xAC},
            {"assembliesOffset",                  0xB0},
            {"assembliesSize",                    0xB4},
            {"fieldRefsOffset",                   0xB8},
            {"fieldRefsSize",                     0xBC},
            {"referencedAssembliesOffset",        0xC0},
            {"referencedAssembliesSize",          0xC4},
            // V29+: NEW attribute data fields (Min=29, replaces attributesInfo)
            {"attributeDataOffset",               0xC8},
            {"attributeDataSize",                 0xCC},
            {"attributeDataRangeOffset",          0xD0},
            {"attributeDataRangeSize",            0xD4},
            // V22+: unresolved virtual calls
            {"unresolvedVirtualCallParameterTypesOffset",   0xD8},
            {"unresolvedVirtualCallParameterTypesSize",     0xDC},
            {"unresolvedVirtualCallParameterRangesOffset",  0xE0},
            {"unresolvedVirtualCallParameterRangesSize",    0xE4},
            // V23+: Windows Runtime
            {"windowsRuntimeTypeNamesOffset",     0xE8},
            {"windowsRuntimeTypeNamesSize",       0xEC},
            // V27+: Windows Runtime strings
            {"windowsRuntimeStringsOffset",       0xF0},
            {"windowsRuntimeStringsSize",         0xF4},
            // V24+: exported types
            {"exportedTypeDefinitionsOffset",     0xF8},
            {"exportedTypeDefinitionsSize",       0xFC},
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        // V29 structs: same as V27 for most types
        // Il2CppCustomAttributeDataRange: NEW struct (replaces Il2CppCustomAttributeTypeRange)
        // Source: il2cpp-dumper-rs metadata.rs + Il2CppDumper MetadataClass.cs
        static const std::unordered_map<std::string, size_t> kSizes = {
            {"Il2CppTypeDefinition",     0x58}, // same as v27
            {"Il2CppMethodDefinition",    0x1C}, // same as v27
            {"Il2CppFieldDefinition",    0x0C}, // same as v27
            {"Il2CppParameterDefinition", 0x0C}, // same as v27
            {"Il2CppPropertyDefinition", 0x14}, // same as v27
            {"Il2CppEventDefinition",    0x18}, // same as v27
            {"Il2CppImageDefinition",    0x28}, // same as v27
            {"Il2CppAssemblyDefinition", 0x44}, // same as v27
            {"Il2CppAssemblyNameDefinition", 0x30}, // same as v27 (no hashValueIndex)
            {"Il2CppCustomAttributeDataRange", 0x08}, // NEW: token + startOffset
            {"Il2CppStringLiteral",      0x10}, // token + dataIndex
        };
        auto it = kSizes.find(key);
        return it != kSizes.end() ? it->second : 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        if (len < 8) return false;
        uint32_t magic;
        std::memcpy(&magic, headerBytes, 4);
        uint32_t ver;
        std::memcpy(&ver, headerBytes + 4, 4);
        return magic == 0xFAB11BAF && ver == 29;
    }
};

} // namespace omnibyte::dumper::dumper1
