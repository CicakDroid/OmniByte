#pragma once

#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace omnibyte::updater {

struct GithubAsset {
    std::string name;
    std::string browserDownloadUrl;
    uint64_t size = 0;
    std::string contentType;
};

struct GithubRelease {
    std::string tagName;
    std::string name;
    std::string body;
    bool draft = false;
    bool prerelease = false;
    std::vector<GithubAsset> assets;
    std::string publishedAt;
};

struct Version {
    int major = 0;
    int minor = 0;
    int patch = 0;

    static Version parse(const std::string& tag);
    bool operator<(const Version& other) const;
    bool operator>(const Version& other) const;
    bool operator==(const Version& other) const;
    bool operator<=(const Version& other) const;
    bool operator>=(const Version& other) const;
    std::string toString() const;
};

using ProgressCallback = std::function<void(float progress, uint64_t current, uint64_t total)>;
using LogCallback = std::function<void(const std::string& message)>;

class GithubReleaseChecker {
public:
    GithubReleaseChecker(const std::string& owner, const std::string& repo);
    ~GithubReleaseChecker();

    void setAuthToken(const std::string& token);
    void setProxy(const std::string& proxyUrl);
    void setTimeout(int timeoutMs);

    std::optional<GithubRelease> getLatestRelease();
    std::optional<GithubRelease> getReleaseByTag(const std::string& tag);
    std::vector<GithubRelease> getAllReleases(int page = 1, int perPage = 30);

    bool isNewerAvailable(const std::string& currentVersion);
    std::optional<Version> checkAndGetNewerVersion(const std::string& currentVersion);

    void setLogCallback(LogCallback callback);
    void setProgressCallback(ProgressCallback callback);

private:
    std::string m_owner;
    std::string m_repo;
    std::string m_authToken;
    std::string m_proxyUrl;
    int m_timeoutMs = 10000;
    LogCallback m_logCallback;
    ProgressCallback m_progressCallback;

    std::string buildApiUrl(const std::string& endpoint) const;
    std::string httpRequest(const std::string& url, const std::string& method = "GET") const;
    GithubRelease parseReleaseJson(const std::string& json) const;
    std::vector<GithubRelease> parseReleasesJson(const std::string& json) const;
};

} // namespace omnibyte::updater
