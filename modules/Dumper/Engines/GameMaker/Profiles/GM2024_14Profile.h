#pragma once
// ── Profiles/GM2024_14Profile.h ───────────────────────────────────
// GameMaker GEN8 chunk header offsets — consistent across all GM versions.
// Source: UnderminersTeam/UndertaleModTool @ master, UndertaleGeneralInfo.cs
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::gamemaker {

class GM2024_14Profile : public IEngineProfile {
public:
    std::string version() const override { return "2024.14"; }

    uint64_t offsetOf(const std::string& key) const override {
        if (key == "IsDebuggerDisabled")   return 0x00;
        if (key == "BytecodeVersion")      return 0x01;
        if (key == "Padding")              return 0x02;
        if (key == "FileName")             return 0x04;
        if (key == "Config")               return 0x08;
        if (key == "LastObj")              return 0x0C;
        if (key == "LastTile")             return 0x10;
        if (key == "GameID")               return 0x14;
        if (key == "DirectPlayGuid")       return 0x18;
        if (key == "Name")                 return 0x28;
        if (key == "Version.Major")        return 0x2C;
        if (key == "Version.Minor")        return 0x30;
        if (key == "Version.Release")      return 0x34;
        if (key == "Version.Build")        return 0x38;
        if (key == "DefaultWindowWidth")   return 0x3C;
        if (key == "DefaultWindowHeight")  return 0x40;
        if (key == "Info")                 return 0x44;
        if (key == "LicenseCRC32")         return 0x48;
        if (key == "LicenseMD5")           return 0x4C;
        if (key == "Timestamp")            return 0x5C;
        if (key == "DisplayName")          return 0x64;
        if (key == "ActiveTargets")        return 0x68;
        if (key == "FunctionClassifications") return 0x70;
        if (key == "SteamAppID")           return 0x78;
        if (key == "DebuggerPort")         return 0x7C;
        return 0;
    }

    size_t structSize(const std::string& key) const override {
        if (key == "GEN8")                 return 0x80;
        return 0;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        if (len < 0x3C) return false;
        uint8_t bytecodeVersion = headerBytes[0x01];
        return bytecodeVersion >= 13 && bytecodeVersion <= 17;
    }
};

} // namespace omnibyte::dumper::gamemaker