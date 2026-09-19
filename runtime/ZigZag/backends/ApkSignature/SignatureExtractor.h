#pragma once
// SignatureExtractor — extract v1 (JAR) and v2/v3 signing blocks from APK files.
// APK = ZIP. v1 signatures live under META-INF/. v2/v3 blocks sit between the
// last local file entry and the End of Central Directory record.

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::runtime::backends {

struct SignatureInfo {
    bool hasV1 = false;
    bool hasV2 = false;
    bool hasV3 = false;
    int apiLevel = 0;           // 0 = unknown
    std::string signingBlockPath; // non-empty if v2/v3 found
};

class SignatureExtractor {
public:
    SignatureExtractor() = default;
    ~SignatureExtractor() = default;

    /// Extract META-INF/ entries (MANIFEST.MF, CERT.SF, CERT.RSA) to outDir.
    bool extractV1(const std::string& apkPath, std::string& outDir);

    /// Extract the v2 or v3 signing block into outSigningBlock.
    bool extractV2V3(const std::string& apkPath, std::vector<uint8_t>& outSigningBlock);

    /// Extract both v1 and v2/v3.
    bool extractAll(const std::string& apkPath, const std::string& outDir);

    /// Detect which signature schemes are present.
    SignatureInfo getInfo(const std::string& apkPath) const;

private:
    static constexpr uint32_t kV2Magic = 0xf05368c0;
    static constexpr uint32_t kV3Magic = 0xf05368c1;

    /// Read a little-endian uint32 from buf at offset.
    static uint32_t readU32LE(const uint8_t* buf, size_t off);

    /// Find the offset of the End of Central Directory record.
    static int64_t findEOCD(const uint8_t* data, size_t len);

    /// Find the offset of the last Local File Header (start of data area end).
    static int64_t findLastLocalFileHeader(const uint8_t* data, size_t len);

    /// Check if a filename starts with "META-INF/".
    static bool isMetaInfEntry(const uint8_t* name, uint16_t nameLen);
};

} // namespace omnibyte::runtime::backends
