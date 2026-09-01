#pragma once
// ── Profiles/V22Profile.h ───────────────────────────────────────
// IL2CPP metadata version 22 (Unity 2018.3–2019.1).
// Source: Perfare/Il2CppDumper @ master, Il2Cpp/MetadataClass.cs
// Header: V21 + unresolvedVirtualCall (Min=22)
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unityil2cpp {

class V22Profile : public IEngineProfile {
public:
    std::string version() const override { return "22"; }

    uint64_t offsetOf(const std::string& key) const override {
        // V22 header: V21 + unresolvedVirtualCall (Min=22)
        // src: Il2CppDumper/Il2Cpp/MetadataClass.cs @ master (4741d46)
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            // Base fields (always present, 0x00-0xA4)
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
            // rgctxEntries (Max=24.1, present for V22 since 22 <= 24.1)
            {"rgctxEntriesOffset",                0xA8},
            {"rgctxEntriesCount",                 0xAC},
            // images/assemblies (always present)
            {"imagesOffset",                      0xB0},
            {"imagesSize",                        0xB4},
            {"assembliesOffset",                  0xB8},
            {"assembliesSize",                    0xBC},
            // metadataUsage (Min=19, present for V22 since 22 >= 19)
            {"metadataUsageListsOffset",          0xC0},
            {"metadataUsageListsCount",           0xC4},
            {"metadataUsagePairsOffset",          0xC8},
            {"metadataUsagePairsCount",           0xCC},
            // fieldRefs (Min=19, present for V22 since 22 >= 19)
            {"fieldRefsOffset",                   0xD0},
            {"fieldRefsSize",                     0xD4},
            // referencedAssemblies (Min=20, present for V22 since 22 >= 20)
            {"referencedAssembliesOffset",        0xD8},
            {"referencedAssembliesSize",          0xDC},
            // attributesInfo (Min=21, present for V22 since 22 >= 21)
            {"attributesInfoOffset",              0xE0},
            {"attributesInfoCount",               0xE4},
            {"attributeTypesOffset",              0xE8},
            {"attributeTypesCount",               0xEC},
            // unresolvedVirtualCall (Min=22, present for V22 since 22 >= 22)
            {"unresolvedVirtualCallParameterTypesOffset",   0xF0},
            {"unresolvedVirtualCallParameterTypesSize",     0xF4},
            {"unresolvedVirtualCallParameterRangesOffset",  0xF8},
            {"unresolvedVirtualCallParameterRangesSize",    0xFC},
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        // V22 structs: has customAttributeIndex (Max=24), rgctx (Min=21, Max=24.1)
        // delegateWrapper/marshalingFunctions present (Max=22, V22 <= 22)
        // ccwFunction/guid present (Min=21, Max=22, V22 <= 22)
        // src: Il2CppDumper/Il2Cpp/MetadataClass.cs @ master (4741d46)
        static const std::unordered_map<std::string, size_t> kSizes = {
            {"Il2CppTypeDefinition",  0x84}, // has customAttributeIndex + rgctx + delegateWrapper + marshaling + ccw + guid
            {"Il2CppMethodDefinition", 0x34}, // has customAttributeIndex + methodIndex/invoker/delegate/rgctx
            {"Il2CppFieldDefinition",  0x10}, // has customAttributeIndex
            {"Il2CppParameterDefinition", 0x10}, // has customAttributeIndex
            {"Il2CppPropertyDefinition", 0x18}, // has customAttributeIndex
            {"Il2CppEventDefinition",   0x1C}, // has customAttributeIndex
            {"Il2CppImageDefinition",   0x20}, // no customAttributeStart/Count (Min=24.1)
            {"Il2CppAssemblyDefinition", 0x44}, // has customAttributeIndex + referencedAssemblyStart/Count
            {"Il2CppAssemblyNameDefinition", 0x34}, // has hashValueIndex (Max=24.3)
            {"Il2CppCustomAttributeTypeRange", 0x08}, // no token (Min=24.1)
        };
        auto it = kSizes.find(key);
        return it != kSizes.end() ? it->second : 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // src: Il2CppDumper/Il2Cpp/Metadata.cs @ master
        if (len < 8) return false;
        uint32_t magic = headerBytes[0] | (headerBytes[1] << 8) |
                         (headerBytes[2] << 16) | (headerBytes[3] << 24);
        int32_t ver = headerBytes[4] | (headerBytes[5] << 8) |
                      (headerBytes[6] << 16) | ((uint32_t)headerBytes[7] << 24);
        return magic == 0xFAB11BAF && ver == 22;
    }
};

} // namespace omnibyte::dumper::unityil2cpp
