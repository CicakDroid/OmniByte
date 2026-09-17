#pragma once
// SignatureInjector — inject v1 (JAR) and v2/v3 signing blocks into APK files.
// Used to clone signatures from an original APK into a modified one.

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::runtime::backends {

class SignatureInjector {
public:
    SignatureInjector() = default;
    ~SignatureInjector() = default;

    /// Copy META-INF/ files from signDir into srcApk, writing outApk.
    bool injectV1(const std::string& srcApk, const std::string& signDir,
                  const std::string& outApk);

    /// Append a v2/v3 signing block to srcApk, writing outApk.
    bool injectV2V3(const std::string& srcApk,
                    const std::vector<uint8_t>& signingBlock,
                    const std::string& outApk);

    /// Full clone: extract signatures from origApk, inject into modApk as outApk.
    bool cloneSignatures(const std::string& origApk, const std::string& modApk,
                         const std::string& outApk);

private:
    /// Read a little-endian uint32.
    static uint32_t readU32LE(const uint8_t* buf, size_t off);

    /// Write a little-endian uint32.
    static void writeU32LE(uint8_t* buf, uint32_t val);

    /// Find the End of Central Directory offset (from end of file).
    static int64_t findEOCD(const uint8_t* data, size_t len);

    /// Find the last Local File Header offset.
    static int64_t findLastLocalFileHeader(const uint8_t* data, size_t len);

    /// Append a single file entry to a ZIP stream.
    static bool appendZipEntry(const std::string& outPath,
                               const std::string& entryName,
                               const uint8_t* data, size_t dataLen);

    /// Append raw bytes to a file.
    static bool appendRaw(const std::string& path, const uint8_t* data, size_t len);

    /// Copy a file verbatim.
    static bool copyFile(const std::string& src, const std::string& dst);
};

} // namespace omnibyte::runtime::backends
