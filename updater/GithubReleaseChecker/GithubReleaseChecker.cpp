#include "GithubReleaseChecker.h"
#include <sstream>
#include <regex>
#include <algorithm>
#include <cstring>
#include <curl/curl.h>

namespace omnibyte::updater {

// === Version ===

Version Version::parse(const std::string& tag) {
    Version v;
    std::string cleaned = tag;
    if (!cleaned.empty() && cleaned[0] == 'v') {
        cleaned = cleaned.substr(1);
    }

    std::regex versionRegex(R"((\d+)\.(\d+)\.(\d+))");
    std::smatch match;
    if (std::regex_search(cleaned, match, versionRegex)) {
        v.major = std::stoi(match[1].str());
        v.minor = std::stoi(match[2].str());
        v.patch = std::stoi(match[3].str());
    }
    return v;
}

bool Version::operator<(const Version& o) const {
    if (major != o.major) return major < o.major;
    if (minor != o.minor) return minor < o.minor;
    return patch < o.patch;
}

bool Version::operator>(const Version& o) const { return o < *this; }
bool Version::operator==(const Version& o) const {
    return major == o.major && minor == o.minor && patch == o.patch;
}
bool Version::operator<=(const Version& o) const { return !(o < *this); }
bool Version::operator>=(const Version& o) const { return !(*this < o); }

std::string Version::toString() const {
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

// === Curl Helpers ===

namespace {
struct WriteCallbackData {
    std::string data;
    ProgressCallback progress;
    uint64_t totalSize = 0;
    uint64_t downloaded = 0;
};

size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* data = static_cast<WriteCallbackData*>(userdata);
    size_t bytes = size * nmemb;
    data->data.append(ptr, bytes);
    return bytes;
}

size_t writeToFileCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* data = static_cast<WriteCallbackData*>(userdata);
    size_t bytes = size * nmemb;
    data->downloaded += bytes;
    if (data->progress) {
        float pct = data->totalSize > 0 ? static_cast<float>(data->downloaded) / data->totalSize : 0.0f;
        data->progress(pct, data->downloaded, data->totalSize);
    }
    return bytes;
}

std::string escapeJsonString(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:   result += c; break;
        }
    }
    return result;
}

std::string extractJsonString(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    auto pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return "";

    pos = json.find_first_not_of(" \t\n\r", pos + 1);
    if (pos == std::string::npos) return "";

    if (json[pos] == '"') {
        auto end = json.find('"', pos + 1);
        if (end == std::string::npos) return "";
        return json.substr(pos + 1, end - pos - 1);
    }

    auto end = json.find_first_of(",}\n", pos);
    if (end == std::string::npos) return json.substr(pos);
    return json.substr(pos, end - pos);
}

uint64_t extractJsonUint64(const std::string& json, const std::string& key) {
    std::string val = extractJsonString(json, key);
    try { return std::stoull(val); } catch (...) { return 0; }
}

bool extractJsonBool(const std::string& json, const std::string& key) {
    std::string val = extractJsonString(json, key);
    return val == "true";
}
} // anon namespace

// === GithubReleaseChecker ===

GithubReleaseChecker::GithubReleaseChecker(const std::string& owner, const std::string& repo)
    : m_owner(owner), m_repo(repo) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

GithubReleaseChecker::~GithubReleaseChecker() {
    curl_global_cleanup();
}

void GithubReleaseChecker::setAuthToken(const std::string& token) { m_authToken = token; }
void GithubReleaseChecker::setProxy(const std::string& proxyUrl) { m_proxyUrl = proxyUrl; }
void GithubReleaseChecker::setTimeout(int timeoutMs) { m_timeoutMs = timeoutMs; }
void GithubReleaseChecker::setLogCallback(LogCallback cb) { m_logCallback = std::move(cb); }
void GithubReleaseChecker::setProgressCallback(ProgressCallback cb) { m_progressCallback = std::move(cb); }

std::string GithubReleaseChecker::buildApiUrl(const std::string& endpoint) const {
    return "https://api.github.com/repos/" + m_owner + "/" + m_repo + endpoint;
}

std::string GithubReleaseChecker::httpRequest(const std::string& url, const std::string& method) const {
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    WriteCallbackData data;
    std::string userAgent = "OmniByte-Updater/1.0";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, m_timeoutMs);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    if (!m_proxyUrl.empty()) {
        curl_easy_setopt(curl, CURLOPT_PROXY, m_proxyUrl.c_str());
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/vnd.github+json");
    if (!m_authToken.empty()) {
        std::string authHeader = "Authorization: Bearer " + m_authToken;
        headers = curl_slist_append(headers, authHeader.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        if (m_logCallback) m_logCallback("HTTP request failed: " + std::string(curl_easy_strerror(res)));
        return "";
    }
    return data.data;
}

GithubRelease GithubReleaseChecker::parseReleaseJson(const std::string& json) const {
    GithubRelease release;
    release.tagName = extractJsonString(json, "tag_name");
    release.name = extractJsonString(json, "name");
    release.body = extractJsonString(json, "body");
    release.draft = extractJsonBool(json, "draft");
    release.prerelease = extractJsonBool(json, "prerelease");
    release.publishedAt = extractJsonString(json, "published_at");

    auto assetsPos = json.find("\"assets\"");
    if (assetsPos != std::string::npos) {
        auto arrStart = json.find('[', assetsPos);
        auto arrEnd = json.find(']', arrStart);
        if (arrStart != std::string::npos && arrEnd != std::string::npos) {
            std::string assetsJson = json.substr(arrStart, arrEnd - arrStart + 1);
            size_t pos = 0;
            while ((pos = assetsJson.find('{', pos)) != std::string::npos) {
                auto objEnd = assetsJson.find('}', pos);
                if (objEnd == std::string::npos) break;

                std::string obj = assetsJson.substr(pos, objEnd - pos + 1);
                GithubAsset asset;
                asset.name = extractJsonString(obj, "name");
                asset.browserDownloadUrl = extractJsonString(obj, "browser_download_url");
                asset.size = extractJsonUint64(obj, "size");
                asset.contentType = extractJsonString(obj, "content_type");
                release.assets.push_back(std::move(asset));
                pos = objEnd + 1;
            }
        }
    }
    return release;
}

std::vector<GithubRelease> GithubReleaseChecker::parseReleasesJson(const std::string& json) const {
    std::vector<GithubRelease> releases;
    size_t pos = 0;
    while ((pos = json.find('{', pos)) != std::string::npos) {
        auto objEnd = json.find('}', pos);
        if (objEnd == std::string::npos) break;

        std::string obj = json.substr(pos, objEnd - pos + 1);
        if (obj.find("tag_name") != std::string::npos) {
            releases.push_back(parseReleaseJson(obj));
        }
        pos = objEnd + 1;
    }
    return releases;
}

std::optional<GithubRelease> GithubReleaseChecker::getLatestRelease() {
    std::string json = httpRequest(buildApiUrl("/releases/latest"));
    if (json.empty()) return std::nullopt;
    return parseReleaseJson(json);
}

std::optional<GithubRelease> GithubReleaseChecker::getReleaseByTag(const std::string& tag) {
    std::string json = httpRequest(buildApiUrl("/releases/tags/" + tag));
    if (json.empty()) return std::nullopt;
    return parseReleaseJson(json);
}

std::vector<GithubRelease> GithubReleaseChecker::getAllReleases(int page, int perPage) {
    std::string endpoint = "/releases?page=" + std::to_string(page) + "&per_page=" + std::to_string(perPage);
    std::string json = httpRequest(buildApiUrl(endpoint));
    if (json.empty()) return {};
    return parseReleasesJson(json);
}

bool GithubReleaseChecker::isNewerAvailable(const std::string& currentVersion) {
    auto latest = getLatestRelease();
    if (!latest) return false;

    Version current = Version::parse(currentVersion);
    Version latestVer = Version::parse(latest->tagName);
    return latestVer > current;
}

std::optional<Version> GithubReleaseChecker::checkAndGetNewerVersion(const std::string& currentVersion) {
    auto latest = getLatestRelease();
    if (!latest) return std::nullopt;

    Version current = Version::parse(currentVersion);
    Version latestVer = Version::parse(latest->tagName);

    if (latestVer > current) {
        if (m_logCallback) {
            m_logCallback("New version available: " + latestVer.toString() + " (current: " + current.toString() + ")");
        }
        return latestVer;
    }
    return std::nullopt;
}

} // namespace omnibyte::updater
