#include "Downloader.h"
#include <fstream>
#include <curl/curl.h>

namespace omnibyte::updater {

namespace {
struct DownloadCtx {
    std::ofstream file;
    DownloadProgressCallback progress;
    uint64_t totalSize = 0;
    uint64_t downloaded = 0;
    bool failed = false;
};

size_t writeToFile(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* ctx = static_cast<DownloadCtx*>(userdata);
    size_t bytes = size * nmemb;

    ctx->file.write(ptr, bytes);
    if (!ctx->file) {
        ctx->failed = true;
        return 0;
    }

    ctx->downloaded += bytes;
    if (ctx->progress) {
        float pct = ctx->totalSize > 0 ? static_cast<float>(ctx->downloaded) / ctx->totalSize : 0.0f;
        ctx->progress(pct, ctx->downloaded, ctx->totalSize);
    }
    return bytes;
}

int progressCallback(void* clientp, curl_off_t, curl_off_t dltotal, curl_off_t dlnow, curl_off_t, curl_off_t) {
    auto* ctx = static_cast<DownloadCtx*>(clientp);
    ctx->totalSize = static_cast<uint64_t>(dltotal);

    if (ctx->progress && dltotal > 0) {
        float pct = static_cast<float>(dlnow) / static_cast<float>(dltotal);
        ctx->progress(pct, static_cast<uint64_t>(dlnow), static_cast<uint64_t>(dltotal));
    }

    return ctx->failed ? 1 : 0;
}
} // anon namespace

Downloader::Downloader() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

Downloader::~Downloader() {
    curl_global_cleanup();
}

void Downloader::setProxy(const std::string& proxyUrl) { m_proxyUrl = proxyUrl; }
void Downloader::setTimeout(int timeoutMs) { m_timeoutMs = timeoutMs; }
void Downloader::setMaxRetries(int retries) { m_maxRetries = retries; }
void Downloader::setLogCallback(DownloadLogCallback cb) { m_logCallback = std::move(cb); }
void Downloader::setProgressCallback(DownloadProgressCallback cb) { m_progressCallback = std::move(cb); }
void Downloader::cancelCurrentDownload() { m_cancelled = true; }

DownloadResult Downloader::downloadFile(const std::string& url, const std::string& savePath) {
    m_cancelled = false;

    for (int attempt = 1; attempt <= m_maxRetries; ++attempt) {
        if (m_cancelled) {
            return {DownloadStatus::Cancelled, savePath, "Download cancelled by user"};
        }

        if (m_logCallback) {
            m_logCallback("Download attempt " + std::to_string(attempt) + "/" + std::to_string(m_maxRetries));
        }

        auto result = downloadInternal(url, savePath);
        if (result.status == DownloadStatus::Success) {
            return result;
        }

        if (attempt < m_maxRetries && m_logCallback) {
            m_logCallback("Retry " + std::to_string(attempt + 1) + " after failure: " + result.errorMessage);
        }
    }

    return {DownloadStatus::Failed, savePath, "All " + std::to_string(m_maxRetries) + " attempts failed"};
}

DownloadResult Downloader::downloadInternal(const std::string& url, const std::string& savePath) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return {DownloadStatus::Failed, savePath, "Failed to initialize CURL"};
    }

    DownloadCtx ctx;
    ctx.progress = m_progressCallback;
    ctx.file.open(savePath, std::ios::binary);
    if (!ctx.file.is_open()) {
        curl_easy_cleanup(curl);
        return {DownloadStatus::WriteError, savePath, "Cannot open file for writing: " + savePath};
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, m_timeoutMs);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "OmniByte-Updater/1.0");

    if (!m_proxyUrl.empty()) {
        curl_easy_setopt(curl, CURLOPT_PROXY, m_proxyUrl.c_str());
    }

    CURLcode res = curl_easy_perform(curl);

    ctx.file.close();
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::remove(savePath.c_str());
        std::string err = curl_easy_strerror(res);
        if (res == CURLE_OPERATION_TIMEDOUT) {
            return {DownloadStatus::Timeout, savePath, "Download timed out: " + err};
        }
        return {DownloadStatus::HttpError, savePath, "HTTP error: " + err};
    }

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    if (httpCode < 200 || httpCode >= 300) {
        std::remove(savePath.c_str());
        return {DownloadStatus::HttpError, savePath, "HTTP status " + std::to_string(httpCode)};
    }

    if (ctx.failed) {
        std::remove(savePath.c_str());
        return {DownloadStatus::WriteError, savePath, "Write to disk failed"};
    }

    return {DownloadStatus::Success, savePath, "", ctx.downloaded};
}

DownloadResult Downloader::downloadWithChecksum(const std::string& url, const std::string& savePath,
                                                const std::string& expectedSha256) {
    auto result = downloadFile(url, savePath);
    if (result.status != DownloadStatus::Success) return result;

    if (!expectedSha256.empty()) {
        std::string actual = sha256File(savePath);
        if (actual != expectedSha256) {
            std::remove(savePath.c_str());
            return {DownloadStatus::ChecksumMismatch, savePath,
                    "SHA256 mismatch: expected " + expectedSha256 + ", got " + actual};
        }
    }
    return result;
}

std::string Downloader::sha256File(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) return "";

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    char buffer[8192];
    while (file.read(buffer, sizeof(buffer))) {
        SHA256_Update(&sha256, buffer, file.gcount());
    }
    SHA256_Update(&sha256, buffer, file.gcount());
    SHA256_Final(hash, &sha256);

    std::string result;
    result.reserve(SHA256_DIGEST_LENGTH * 2);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", hash[i]);
        result += hex;
    }
    return result;
}

} // namespace omnibyte::updater
