#pragma once
// UnrealEngine — static analyzer for .pak pack files.
// Reads GNames/GObjects table structure from .pak / binary section.
// Does not require live process.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::unrealengine {

class UnrealEngineAnalyzer {
public:
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "UnrealEngine";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!target.isFile()) {
            result.errorMessage = "UE analyzer only supports file targets (.pak/.exe/.so)";
            return result;
        }

        auto fileData = utils::readFileBytes(target.filePath);
        if (fileData.empty()) {
            result.errorMessage = "Failed to read file: " + target.filePath;
            return result;
        }

        // Try .pak format first
        if (fileData.size() >= kPakHeaderSize) {
            uint32_t magic = readU32(fileData, 0);
            if (magic == kPakMagic) {
                return analyzePak(fileData, profile);
            }
        }

        // Try binary scan for UE version string
        return analyzeBinary(fileData, profile);
    }

private:
    static constexpr uint32_t kPakMagic = 0x5A6F12E1;
    static constexpr size_t kPakHeaderSize = 44;  // Minimum .pak header

    // .pak header layout:
    // offset 0x00: Magic (u32)
    // offset 0x04: Version (u32)
    // offset 0x08: IndexOffset (u64)
    // offset 0x10: IndexSize (u32)
    // offset 0x14: IndexHash (20 bytes SHA1)
    // total: 44 bytes minimum
    static DumpResult analyzePak(const std::vector<uint8_t>& data,
                                  const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "UnrealEngine";
        result.detectedVersion = profile ? profile->version() : "unknown";

        uint32_t version = readU32(data, 0x04);
        uint64_t indexOffset = readU64(data, 0x08);
        uint32_t indexSize = readU32(data, 0x10);

        result.setMeta("pakVersion", std::to_string(version));
        result.setMeta("indexOffset", std::to_string(indexOffset));
        result.setMeta("indexSize", std::to_string(indexSize));

        // Parse index if it fits in the file
        if (indexOffset < data.size() && indexSize > 0 &&
            indexOffset + indexSize <= data.size()) {
            int entryCount = parsePakIndex(data, static_cast<size_t>(indexOffset),
                                            indexSize, result);
            result.setMeta("entryCount", std::to_string(entryCount));
        }

        result.success = true;
        return result;
    }

    // Parse .pak index table — extract file names as StringEntries
    static int parsePakIndex(const std::vector<uint8_t>& data, size_t indexStart,
                              uint32_t indexSize, DumpResult& result) {
        size_t pos = indexStart;
        size_t indexEnd = indexStart + indexSize;
        int count = 0;

        while (pos + 4 <= indexEnd && pos + 4 <= data.size()) {
            // Each entry: string length (u32) + string chars
            uint32_t nameLen = readU32(data, pos);
            if (nameLen == 0 || nameLen > 1024) break;
            pos += 4;

            if (pos + nameLen > data.size()) break;
            std::string name(reinterpret_cast<const char*>(data.data() + pos), nameLen);
            pos += nameLen;

            // Skip entry metadata (offset u64, size u64, etc.)
            // Exact layout depends on pak version, but at minimum skip 16 bytes
            if (pos + 16 > indexEnd) break;
            pos += 16;

            StringEntry entry;
            entry.value = name;
            entry.address = indexStart + (pos - indexStart - nameLen - 20);
            result.stringTable.push_back(entry);
            count++;
        }

        return count;
    }

    // Scan binary for UE version strings ("UE4", "UE5")
    static DumpResult analyzeBinary(const std::vector<uint8_t>& data,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "UnrealEngine";
        result.detectedVersion = profile ? profile->version() : "unknown";

        // Search for UE version string patterns
        std::string dataStr(reinterpret_cast<const char*>(data.data()),
                            std::min(data.size(), static_cast<size_t>(4 * 1024 * 1024)));

        // Look for "UE4" or "UE5" version strings
        auto ue4Pos = dataStr.find("UE4");
        auto ue5Pos = dataStr.find("UE5");

        if (ue4Pos != std::string::npos) {
            result.setMeta("ueVersion", "UE4");
            result.setMeta("versionStringOffset", std::to_string(ue4Pos));
        } else if (ue5Pos != std::string::npos) {
            result.setMeta("ueVersion", "UE5");
            result.setMeta("versionStringOffset", std::to_string(ue5Pos));
        }

        // Look for GEngineVersion pattern (common in UE binaries)
        auto engineVersionPos = dataStr.find("GEngineVersion");
        if (engineVersionPos != std::string::npos) {
            result.setMeta("gEngineVersionFound", "true");
        }

        result.success = (ue4Pos != std::string::npos || ue5Pos != std::string::npos);
        if (!result.success) {
            result.errorMessage = "No UE4/UE5 version string found in binary";
        }

        return result;
    }

    static uint32_t readU32(const std::vector<uint8_t>& buf, size_t off) {
        if (off + 4 > buf.size()) return 0;
        uint32_t v;
        std::memcpy(&v, buf.data() + off, 4);
        return v;
    }

    static uint64_t readU64(const std::vector<uint8_t>& buf, size_t off) {
        if (off + 8 > buf.size()) return 0;
        uint64_t v;
        std::memcpy(&v, buf.data() + off, 8);
        return v;
    }
};

} // namespace omnibyte::dumper::unrealengine
