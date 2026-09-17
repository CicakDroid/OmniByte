#pragma once
// ApkSignatureBypass — orchestrator for APK signature verification bypass.
// Coordinates ApkSigKiller (v1 JAR signatures) and ApkSigKillerEx (v2/v3 + anti-MT).
//
// Usage:
//   ApkSignatureBypass bypass;
//   bypass.initialize();
//   bypass.bypassPackage("com.example.game", "base64_signatures...");
//   // or offline:
//   bypass.inject("unsigned.apk", "signed.apk", "patched.apk");

#include "ApkSigKiller/ApkSigKillerAdapter.h"
#include "ApkSigKillerEx/ApkSigKillerAdapter.h"
#include <string>
#include <functional>
#include <memory>

namespace omnibyte::runtime::backends {

/// Bypass strategy: which adapter(s) to use.
enum class BypassStrategy {
    Auto,           // Detect and pick best (default).
    V1Only,         // ApkSigKiller only (JAR signatures, Android 4.x+).
    V1PlusV2V3,     // ApkSigKillerEx (full bypass, Android 9+).
    Both,           // Install both hooks for maximum coverage.
};

/// Result of a bypass operation.
struct BypassResult {
    bool success = false;
    std::string message;
    std::string method;  // "ApkSignatureKiller" or "ApkSignatureKillerEx" or "Both"
};

class ApkSignatureBypass {
public:
    ApkSignatureBypass();
    ~ApkSignatureBypass() = default;

    // --- Lifecycle ---
    /// Initialize both adapters and check upstream versions.
    bool initialize();

    /// Shut down and release resources.
    void shutdown();

    // --- Online Bypass (requires root / Xposed) ---
    /// Bypass signature verification for a running package via PMS hook.
    /// @param packageName     Target package name.
    /// @param signatureData   Base64-encoded original APK signatures.
    /// @param strategy        Which bypass method to use.
    /// @return BypassResult with success status and details.
    BypassResult bypassPackage(const std::string& packageName,
                               const std::string& signatureData,
                               BypassStrategy strategy = BypassStrategy::Auto);

    /// Remove all installed hooks.
    bool removeAllHooks();

    // --- Offline Bypass (no root needed, patch APK file) ---
    /// Inject signature bypass into an APK file.
    /// @param srcApk   Path to the unsigned/modified APK.
    /// @param signApk  Path to the original signed APK (source of signatures).
    /// @param outApk   Path to write the patched APK.
    /// @param strategy Which bypass method to use.
    /// @return BypassResult.
    BypassResult injectApk(const std::string& srcApk,
                           const std::string& signApk,
                           const std::string& outApk,
                           BypassStrategy strategy = BypassStrategy::Auto);

    // --- Version Management ---
    /// Check if updates are available for either adapter.
    struct UpdateInfo {
        bool killerHasUpdate = false;
        bool killerExHasUpdate = false;
        std::string killerInstalled;
        std::string killerLatest;
        std::string killerExInstalled;
        std::string killerExLatest;
    };
    UpdateInfo checkForUpdates() const;

    /// Update both adapters from upstream.
    bool updateAll();

    // --- Accessors ---
    ApkSigKillerAdapter& killer() { return killer_; }
    ApkSigKillerExAdapter& killerEx() { return killerEx_; }
    bool isInitialized() const { return initialized_; }

    /// Set progress callback for all operations.
    void setProgressCallback(std::function<void(int percent, const std::string& message)> cb) {
        progressCb_ = std::move(cb);
        killer_.setProgressCallback(progressCb_);
        killerEx_.setProgressCallback(progressCb_);
    }

private:
    bool initialized_ = false;
    ApkSigKillerAdapter killer_;
    ApkSigKillerExAdapter killerEx_;
    std::function<void(int, const std::string&)> progressCb_;

    /// Auto-detect best strategy based on Android API level.
    BypassStrategy detectBestStrategy() const;

    /// Get Android API level from system properties.
    int getApiLevel() const;

    void reportProgress(int pct, const std::string& msg);
};

} // namespace omnibyte::runtime::backends
