#pragma once
// ── Profiles/V24Profile.h ───────────────────────────────────────
// IL2CPP metadata version 24 (Unity 2017.1–2018.4).
// Source: Perfare/Il2CppDumper @ 4741d46, Il2Cpp/MetadataClass.cs
// Header: Il2CppGlobalMetadataHeader with rgctx fields present (version <= 24.1)
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace omnibyte::dumper::unityil2cpp {

class V24Profile : public IEngineProfile {
public:
    std::string version() const override { return "24"; }

    uint64_t offsetOf(const std::string& key) const override {
        // Header field offsets — computed from Il2CppDumper MetadataClass.cs field order
        // src: Il2CppDumper/Il2Cpp/MetadataClass.cs @ 4741d46
        static const std::unordered_map<std::string, uint64_t> kOffsets = {
            // Base header (always present)
            {"sanity",                            0x00}, // uint32_t, magic 0xFAB11BAF
            {"version",                           0x04}, // int32_t, metadata version (24)
            {"stringLiteralOffset",               0x08}, // uint32_t
            {"stringLiteralSize",                 0x0C}, // int32_t
            {"stringLiteralDataOffset",           0x10}, // uint32_t
            {"stringLiteralDataSize",             0x14}, // int32_t
            {"stringOffset",                      0x18}, // uint32_t, metadata string table
            {"stringSize",                        0x1C}, // int32_t
            {"eventsOffset",                      0x20}, // uint32_t, Il2CppEventDefinition[]
            {"eventsSize",                        0x24}, // int32_t
            {"propertiesOffset",                  0x28}, // uint32_t, Il2CppPropertyDefinition[]
            {"propertiesSize",                    0x2C}, // int32_t
            {"methodsOffset",                     0x30}, // uint32_t, Il2CppMethodDefinition[]
            {"methodsSize",                       0x34}, // int32_t
            {"parameterDefaultValuesOffset",      0x38}, // uint32_t, Il2CppParameterDefaultValue[]
            {"parameterDefaultValuesSize",        0x3C}, // int32_t
            {"fieldDefaultValuesOffset",          0x40}, // uint32_t, Il2CppFieldDefaultValue[]
            {"fieldDefaultValuesSize",            0x44}, // int32_t
            {"fieldAndParameterDefaultValueDataOffset", 0x48}, // uint32_t, raw data
            {"fieldAndParameterDefaultValueDataSize",   0x4C}, // int32_t
            {"fieldMarshaledSizesOffset",         0x50}, // int32_t, Il2CppFieldMarshaledSize[]
            {"fieldMarshaledSizesSize",           0x54}, // int32_t
            {"parametersOffset",                  0x58}, // uint32_t, Il2CppParameterDefinition[]
            {"parametersSize",                    0x5C}, // int32_t
            {"fieldsOffset",                      0x60}, // uint32_t, Il2CppFieldDefinition[]
            {"fieldsSize",                        0x64}, // int32_t
            {"genericParametersOffset",           0x68}, // uint32_t, Il2CppGenericParameter[]
            {"genericParametersSize",             0x6C}, // int32_t
            {"genericParameterConstraintsOffset", 0x70}, // uint32_t, int32_t[]
            {"genericParameterConstraintsSize",   0x74}, // int32_t
            {"genericContainersOffset",           0x78}, // uint32_t, Il2CppGenericContainer[]
            {"genericContainersSize",             0x7C}, // int32_t
            {"nestedTypesOffset",                 0x80}, // uint32_t, int32_t[]
            {"nestedTypesSize",                   0x84}, // int32_t
            {"interfacesOffset",                  0x88}, // uint32_t, int32_t[]
            {"interfacesSize",                    0x8C}, // int32_t
            {"vtableMethodsOffset",               0x90}, // uint32_t, uint32_t[]
            {"vtableMethodsSize",                 0x94}, // int32_t
            {"interfaceOffsetsOffset",            0x98}, // int32_t, Il2CppInterfaceOffsetPair[]
            {"interfaceOffsetsSize",              0x9C}, // int32_t
            {"typeDefinitionsOffset",             0xA0}, // uint32_t, Il2CppTypeDefinition[]
            {"typeDefinitionsSize",               0xA4}, // int32_t
            // V24/V24.1 only — rgctx fields (Version Max=24.1)
            {"rgctxEntriesOffset",                0xA8}, // uint32_t, Il2CppRGCTXDefinition[]
            {"rgctxEntriesCount",                 0xAC}, // int32_t
            // Common fields after rgctx
            {"imagesOffset",                      0xB0}, // uint32_t, Il2CppImageDefinition[]
            {"imagesSize",                        0xB4}, // int32_t
            {"assembliesOffset",                  0xB8}, // uint32_t, Il2CppAssemblyDefinition[]
            {"assembliesSize",                    0xBC}, // int32_t
            // V19–V24.5 only — metadata usage (Version Min=19, Max=24.5)
            {"metadataUsageListsOffset",          0xC0}, // uint32_t, Il2CppMetadataUsageList[]
            {"metadataUsageListsCount",           0xC4}, // int32_t
            {"metadataUsagePairsOffset",          0xC8}, // uint32_t, Il2CppMetadataUsagePair[]
            {"metadataUsagePairsCount",           0xCC}, // int32_t
            // V19+ only — field refs
            {"fieldRefsOffset",                   0xD0}, // uint32_t, Il2CppFieldRef[]
            {"fieldRefsSize",                     0xD4}, // int32_t
            // V20+ only — referenced assemblies
            {"referencedAssembliesOffset",        0xD8}, // int32_t, int32_t[]
            {"referencedAssembliesSize",          0xDC}, // int32_t
            // V21–V27.2 only — custom attributes
            {"attributesInfoOffset",              0xE0}, // uint32_t, Il2CppCustomAttributeTypeRange[]
            {"attributesInfoCount",               0xE4}, // int32_t
            {"attributeTypesOffset",              0xE8}, // uint32_t, int32_t[]
            {"attributeTypesCount",               0xEC}, // int32_t
            // V22+ only — unresolved virtual calls
            {"unresolvedVirtualCallParameterTypesOffset",   0xF0}, // int32_t, int32_t[]
            {"unresolvedVirtualCallParameterTypesSize",     0xF4}, // int32_t
            {"unresolvedVirtualCallParameterRangesOffset",  0xF8}, // int32_t, Il2CppRange[]
            {"unresolvedVirtualCallParameterRangesSize",    0xFC}, // int32_t
            // V23+ only — Windows Runtime
            {"windowsRuntimeTypeNamesOffset",     0x100}, // int32_t, Il2CppWindowsRuntimeTypeNamePair[]
            {"windowsRuntimeTypeNamesSize",       0x104}, // int32_t
            // V24+ only — exported types
            {"exportedTypeDefinitionsOffset",     0x108}, // int32_t, int32_t[]
            {"exportedTypeDefinitionsSize",       0x10C}, // int32_t
        };
        auto it = kOffsets.find(key);
        return it != kOffsets.end() ? it->second : 0;
    }

    size_t structSize(const std::string& key) const override {
        // Struct sizes computed from Il2CppDumper MetadataClass.cs field layout
        // src: Il2CppDumper/Il2Cpp/MetadataClass.cs @ 4741d46
        static const std::unordered_map<std::string, size_t> kSizes = {
            {"Il2CppTypeDefinition",  0x70}, // v24: has customAttributeIndex + rgctx + byrefTypeIndex
            {"Il2CppMethodDefinition", 0x34}, // v24: has customAttributeIndex + methodIndex/invoker/delegate/rgctx
            {"Il2CppFieldDefinition",  0x10}, // v24: has customAttributeIndex
            {"Il2CppParameterDefinition", 0x10}, // v24: has customAttributeIndex
            {"Il2CppPropertyDefinition", 0x18}, // v24: has customAttributeIndex
            {"Il2CppEventDefinition",   0x1C}, // v24: has customAttributeIndex
            {"Il2CppImageDefinition",   0x20}, // v24: no customAttributeStart/Count
            {"Il2CppAssemblyDefinition", 0x44}, // v24: has customAttributeIndex + hashValueIndex
            {"Il2CppAssemblyNameDefinition", 0x34}, // v24: has hashValueIndex
            {"Il2CppCustomAttributeTypeRange", 0x08}, // v24: no token field
        };
        auto it = kSizes.find(key);
        return it != kSizes.end() ? it->second : 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // src: Il2CppDumper/Il2Cpp/Metadata.cs @ 4741d46, lines 45-58
        // magic = 0xFAB11BAF, version field == 24
        if (len < 8) return false;
        uint32_t magic = headerBytes[0] | (headerBytes[1] << 8) |
                         (headerBytes[2] << 16) | (headerBytes[3] << 24);
        int32_t ver = headerBytes[4] | (headerBytes[5] << 8) |
                      (headerBytes[6] << 16) | ((uint32_t)headerBytes[7] << 24);
        return magic == 0xFAB11BAF && ver == 24;
    }
};

} // namespace omnibyte::dumper::unityil2cpp
