#pragma once
// Godot — static analyzer for .pck package files.
// Parses GDPC magic header and file table from Godot pack files.
// Does not require live process.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::godot {

class GodotAnalyzer {
public:
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "Godot";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!target.isFile()) {
            result.errorMessage = "Godot analyzer only supports file targets (.pck)";
            return result;
        }

        auto fileData = utils::readFileBytes(target.filePath);
        if (fileData.empty()) {
            result.errorMessage = "Failed to read file: " + target.filePath;
            return result;
        }

        if (fileData.size() < kMinPckSize) {
            result.errorMessage = "File too small to be a valid .pck";
            return result;
        }

        // Validate GDPC magic
        if (std::memcmp(fileData.data(), kGdpcMagic, 4) != 0) {
            result.errorMessage = "Invalid .pck magic (expected GDPC)";
            return result;
        }

        // Parse header
        uint32_t packVersion = readU32(fileData, 0x04);
        uint32_t verMajor = readU32(fileData, 0x08);
        uint32_t verMinor = readU32(fileData, 0x0C);
        uint32_t verPatch = readU32(fileData, 0x10);
        uint32_t flags = readU32(fileData, 0x14);

        result.setMeta("packVersion", std::to_string(packVersion));
        result.setMeta("godotVersion",
            std::to_string(verMajor) + "." + std::to_string(verMinor) +
            "." + std::to_string(verPatch));
        result.setMeta("flags", std::to_string(flags));

        // File table offset depends on pack version
        size_t fileTableOffset = 0;
        size_t fileTableSize = 0;

        if (packVersion >= 1) {
            // packVersion 1+: offset + size at 0x18 and 0x20
            fileTableOffset = static_cast<size_t>(readU64(fileData, 0x18));
            fileTableSize = static_cast<size_t>(readU64(fileData, 0x20));
        }

        if (fileTableOffset == 0 || fileTableOffset >= fileData.size()) {
            result.errorMessage = "Invalid file table offset in .pck header";
            return result;
        }

        // Parse file table entries
        size_t pos = fileTableOffset;
        int fileCount = 0;

        while (pos + 4 <= fileData.size() && pos + 4 <= fileTableOffset + fileTableSize) {
            // Pascal string: u32 length + chars
            uint32_t pathLen = readU32(fileData, pos);
            if (pathLen == 0 || pathLen > 1024) break;  // sanity check
            pos += 4;

            if (pos + pathLen > fileData.size()) break;
            std::string path(reinterpret_cast<const char*>(fileData.data() + pos), pathLen);
            pos += pathLen;

            // offset (u64) + size (u64) + md5 (16 bytes) + flags (u32)
            if (pos + 8 + 8 + 16 + 4 > fileData.size()) break;
            pos += 8 + 8 + 16 + 4;

            // Add as StringEntry (file path)
            StringEntry entry;
            entry.value = path;
            entry.address = fileTableOffset;  // conceptual address in pack
            result.stringTable.push_back(entry);

            fileCount++;
        }

        result.setMeta("fileCount", std::to_string(fileCount));
        result.setMeta("fileTableOffset", std::to_string(fileTableOffset));
        result.success = true;
        return result;
    }

private:
    static constexpr size_t kMinPckSize = 64;  // minimum valid .pck size
    static const uint8_t kGdpcMagic[4];        // "GDPC"

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

// Static definition
const uint8_t GodotAnalyzer::kGdpcMagic[4] = {'G', 'D', 'P', 'C'};

} // namespace omnibyte::dumper::godot
