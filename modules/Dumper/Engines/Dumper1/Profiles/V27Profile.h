#pragma once
// Dumper1 — IL2CPP v27 profile (Unity 2019.1 – 2020.3).
// Offsets from il2cpp-dumper-rs metadata.rs + il2cpp::metadata.
#include "../../../DumperCore/IEngineProfile.h"
#include <string>
#include <cstdint>
#include <cstring>
#include <optional>

namespace omnibyte::dumper::dumper1 {

class V27Profile : public IEngineProfile {
public:
    std::string version() const override { return "27"; }

    uint64_t offsetOf(const std::string& key) const override {
        if (key == "stringLiteralOffset")        return 0x08;
        if (key == "stringLiteralDataOffset")    return 0x0C;
        if (key == "stringLiteralCount")         return 0x10;
        if (key == "stringLiteralDataCount")     return 0x14;
        if (key == "typeDefinitionsOffset")      return 0x1C;
        if (key == "typeDefinitionCount")        return 0x20;
        if (key == "methodDefinitionOffset")     return 0x28;
        if (key == "methodDefinitionCount")      return 0x2C;
        if (key == "fieldDefinitionOffset")      return 0x30;
        if (key == "fieldDefinitionCount")       return 0x34;
        if (key == "imageDefinitionOffset")      return 0x3C;
        if (key == "imageDefinitionCount")       return 0x40;
        if (key == "codeRegistrationOffset")     return 0;
        if (key == "metadataRegistrationOffset") return 0;
        return 0;
    }

    size_t structSize(const std::string& key) const override {
        if (key == "Il2CppTypeDefinition")    return 0x2C;
        if (key == "Il2CppMethodDefinition")   return 0x20;
        if (key == "Il2CppFieldDefinition")   return 0x10;
        if (key == "Il2CppImageDefinition")   return 0x2C;
        if (key == "Il2CppStringLiteral")      return 0x08;
        return 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        if (len < 8) return false;
        uint32_t magic;
        std::memcpy(&magic, headerBytes, 4);
        uint32_t ver;
        std::memcpy(&ver, headerBytes + 4, 4);
        return magic == 0xAF1BBA00 && ver == 27;
    }
};

} // namespace omnibyte::dumper::dumper1
