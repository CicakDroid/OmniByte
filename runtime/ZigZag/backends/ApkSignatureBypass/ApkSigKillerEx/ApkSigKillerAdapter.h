#pragma once
// ApkSigKillerEx adapter — wraps L-JINBIN/ApkSignatureKillerEx for advanced APK signature bypass.
// Source: https://github.com/L-JINBIN/ApkSignatureKillerEx (841 stars)
//
// Technique: Extended PMS hook — counters MT Manager's signature removal detection.
//            Hooks getPackageInfo() and getSigningInfo() (API 28+) to spoof both
//            v1 (JAR) and v2/v3 (APK Signature Scheme) verification results.
// Works on: Android 9+ (API 28+) for full v2/v3 bypass, Android 4.x+ for v1 only.
//
// This adapter provides:
//   - Version check against upstream commits
//   - Auto-update from upstream repo
//   - Extended signature data management (v1 + v2/v3 signatures)
//   - PMS hook installation with anti-MT detection

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace omnibyte::runtime::backends {

class ApkSigKillerExAdapter {
public:
    ApkSigKillerExAdapter();
    ~ApkSigKillerExAdapter() = default;

    // --- Metadata ---
    static constexpr const char* kName = "ApkSignatureKillerEx";
    static constexpr const char* kUpstreamUrl = "https://github.com/L-JINBIN/ApkSignatureKillerEx";
    static constexpr const char* kUpstreamBranch = "main";

    /// Current adapter version (semver).
    static constexpr const char* kVersion = "1.0.0";

    std::string name() const { return kName; }
    std::string version() const { return kVersion; }

    // --- Lifecycle ---
    /// Initialize adapter: fetch upstream version info, check compatibility.
    bool initialize();

    /// Check if extended PMS hook is currently active.
    bool isActive() const { return active_; }

    // --- Signature Bypass ---
    /// Install the extended PMS hook with anti-MT detection.
    /// @param targetPackage  Package name to spoof signatures for (empty = all packages).
    /// @param signatureData  Base64-encoded original APK signature data (v1).
    /// @param signingInfo    Base64-encoded signing info blob (v2/v3, API 28+).
    /// @return true if hook installed successfully.
    bool installHook(const std::string& targetPackage, const std::string& signatureData,
                     const std::string& signingInfo = "");

    /// Remove the extended PMS hook.
    bool removeHook();

    /// Inject extended signature bypass smali into an APK (offline).
    /// @param srcApk   Path to the unsigned/modified APK.
    /// @param signApk  Path to the original signed APK.
    /// @param outApk   Path to write the patched APK.
    /// @return true if injection succeeded.
    bool injectSmali(const std::string& srcApk, const std::string& signApk, const std::string& outApk);

    // --- Version Check ---
    /// Fetch latest commit hash from upstream.
    std::string fetchLatestVersion() const;

    /// Check if a newer version is available.
    bool hasUpdate() const;

    /// Get the currently installed upstream commit hash.
    std::string installedVersion() const { return installedCommit_; }

    // --- Auto-Update ---
    /// Clone/pull latest upstream sources into the build directory.
    /// @return true if update succeeded.
    bool updateFromUpstream();

    /// Register a callback for progress reporting during update.
    void setProgressCallback(std::function<void(int percent, const std::string& message)> cb) {
        progressCb_ = std::move(cb);
    }

private:
    bool active_ = false;
    std::string installedCommit_;
    std::string latestCommit_;
    std::string workDir_;
    std::function<void(int, const std::string&)> progressCb_;

    std::string httpGet(const std::string& url) const;
    std::string exec(const std::string& cmd) const;
    void reportProgress(int pct, const std::string& msg);
};

} // namespace omnibyte::runtime::backends
