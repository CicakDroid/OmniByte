#pragma once

#include <string>
#include <functional>
#include <cstdint>
#include <vector>

namespace omnibyte::updater {

enum class DownloadStatus {
    Success,
    Failed,
    Cancelled,
    Timeout,
    HttpError,
    WriteError,
    ChecksumMismatch
};

struct DownloadResult {
    DownloadStatus status = DownloadStatus::Success;
    std::string savedPath;
    std::string errorMessage;
    uint64_t totalBytes = 0;
};

using DownloadProgressCallback = std::function<void(float progress, uint64_t current, uint64_t total)>;
using DownloadLogCallback = std::function<void(const std::string& message)>;

class Downloader {
public:
    Downloader();
    ~Downloader();

    void setProxy(const std::string& proxyUrl);
    void setTimeout(int timeoutMs);
    void setMaxRetries(int retries);
    void setLogCallback(DownloadLogCallback callback);
    void setProgressCallback(DownloadProgressCallback callback);

    DownloadResult downloadFile(const std::string& url, const std::string& savePath);
    DownloadResult downloadWithChecksum(const std::string& url, const std::string& savePath,
                                        const std::string& expectedSha256);

    void cancelCurrentDownload();

    static std::string sha256File(const std::string& filePath);

private:
    std::string m_proxyUrl;
    int m_timeoutMs = 30000;
    int m_maxRetries = 3;
    bool m_cancelled = false;
    DownloadLogCallback m_logCallback;
    DownloadProgressCallback m_progressCallback;

    DownloadResult downloadInternal(const std::string& url, const std::string& savePath);
};

} // namespace omnibyte::updater
