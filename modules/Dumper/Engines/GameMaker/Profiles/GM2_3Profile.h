#pragma once
// ── Profiles/GM2_3Profile.h ───────────────────────────────────────
// GameMaker GEN8 chunk header offsets — consistent across all GM versions.
// Source: UnderminersTeam/UndertaleModTool @ master, UndertaleGeneralInfo.cs
// Version-specific differences are chunk presence (UILR, PSEM, etc.), not header layout.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::gamemaker {

class GM2_3Profile : public IEngineProfile {
public:
    std::string version() const override { return "2.3"; }

    uint64_t offsetOf(const std::string& key) const override {
        if (key == "IsDebuggerDisabled")   return 0x00;  // byte
        if (key == "BytecodeVersion")      return 0x01;  // byte
        if (key == "Padding")              return 0x02;  // uint16
        if (key == "FileName")             return 0x04;  // pointer (4 bytes)
        if (key == "Config")               return 0x08;  // pointer
        if (key == "LastObj")              return 0x0C;  // uint32
        if (key == "LastTile")             return 0x10;  // uint32
        if (key == "GameID")               return 0x14;  // uint32
        if (key == "DirectPlayGuid")       return 0x18;  // 16 bytes
        if (key == "Name")                 return 0x28;  // pointer
        if (key == "Version.Major")        return 0x2C;  // uint32
        if (key == "Version.Minor")        return 0x30;  // uint32
        if (key == "Version.Release")      return 0x34;  // uint32
        if (key == "Version.Build")        return 0x38;  // uint32
        if (key == "DefaultWindowWidth")   return 0x3C;  // uint32 (GMS2+)
        if (key == "DefaultWindowHeight")  return 0x40;  // uint32 (GMS2+)
        if (key == "Info")                 return 0x44;  // uint32, InfoFlags
        if (key == "LicenseCRC32")         return 0x48;  // uint32 (GMS2+)
        if (key == "LicenseMD5")           return 0x4C;  // 16 bytes (GMS2+)
        if (key == "Timestamp")            return 0x5C;  // uint64 (GMS2+)
        if (key == "DisplayName")          return 0x64;  // pointer (GMS2+)
        if (key == "ActiveTargets")        return 0x68;  // uint64 (GMS2+)
        if (key == "FunctionClassifications") return 0x70; // uint64 (GMS2+)
        if (key == "SteamAppID")           return 0x78;  // int32 (GMS2+)
        if (key == "DebuggerPort")         return 0x7C;  // uint32 (GMS2+, BytecodeVersion >= 14)
        return 0;
    }

    size_t structSize(const std::string& key) const override {
        if (key == "GEN8")                 return 0x80;  // ~128 bytes base (varies by GMS2 fields)
        return 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        if (len < 0x3C) return false;
        uint8_t bytecodeVersion = headerBytes[0x01];
        return bytecodeVersion >= 13 && bytecodeVersion <= 17;
    }
};

} // namespace omnibyte::dumper::gamemaker