#pragma once
// ApkSigKiller adapter — wraps L-JINBIN/ApkSignatureKiller for APK signature bypass.
// Source: https://github.com/L-JINBIN/ApkSignatureKiller (969 stars)
//
// Technique: PMS hook via Xposed — intercepts PackageManager.getPackageInfo()
//            and replaces signatures with the original APK's signatures.
// Works on: Android 4.x–14+, both root (Xposed/LSPosed) and inject (LSPatch).
//
// This adapter provides:
//   - Version check against upstream commits
//   - Auto-update from upstream repo
//   - Signature data management (read/write/store target signatures)
//   - PMS hook installation and removal

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace omnibyte::runtime::backends {

class ApkSigKillerAdapter {
public:
    ApkSigKillerAdapter();
    ~ApkSigKillerAdapter() = default;

    // --- Metadata ---
    static constexpr const char* kName = "ApkSignatureKiller";
    static constexpr const char* kUpstreamUrl = "https://github.com/L-JINBIN/ApkSignatureKiller";
    static constexpr const char* kUpstreamBranch = "master";

    /// Current adapter version (semver).
    static constexpr const char* kVersion = "1.0.0";

    std::string name() const { return kName; }
    std::string version() const { return kVersion; }

    // --- Lifecycle ---
    /// Initialize adapter: fetch upstream version info, check compatibility.
    bool initialize();

    /// Check if PMS hook is currently active.
    bool isActive() const { return active_; }

    // --- Signature Bypass ---
    /// Install the PMS hook that intercepts getPackageInfo() calls.
    /// @param targetPackage  Package name to spoof signatures for (empty = all packages).
    /// @param signatureData  Base64-encoded original APK signature data.
    /// @return true if hook installed successfully.
    bool installHook(const std::string& targetPackage, const std::string& signatureData);

    /// Remove the PMS hook and restore original behavior.
    bool removeHook();

    /// Inject signature bypass smali into an APK file (offline, no root needed).
    /// @param srcApk   Path to the unsigned/modified APK.
    /// @param signApk  Path to the original signed APK (source of legitimate signatures).
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
    /// @return true if update succeeded (sources are now current).
    bool updateFromUpstream();

    /// Register a callback for progress reporting during update.
    void setProgressCallback(std::function<void(int percent, const std::string& message)> cb) {
        progressCb_ = std::move(cb);
    }

private:
    bool active_ = false;
    std::string installedCommit_;
    std::string latestCommit_;
    std::string workDir_;  // temp dir for upstream sources
    std::function<void(int, const std::string&)> progressCb_;

    /// Fetch a single URL via curl/wget and return body.
    std::string httpGet(const std::string& url) const;

    /// Execute a shell command and return stdout.
    std::string exec(const std::string& cmd) const;

    /// Report progress to callback.
    void reportProgress(int pct, const std::string& msg);
};

} // namespace omnibyte::runtime::backends
