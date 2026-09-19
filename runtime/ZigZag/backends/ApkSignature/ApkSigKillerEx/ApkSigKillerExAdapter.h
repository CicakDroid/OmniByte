#pragma once
// ApkSigKillerEx adapter — wraps L-JINBIN/ApkSignatureKillerEx for advanced APK signature bypass.
// Source: https://github.com/L-JINBIN/ApkSignatureKillerEx (841 stars)
//
// Technique: NativePmsHook (inline hook on libandroid_runtime.so) as primary,
//            LoadedApkSpoofer (JNI field spoofing) as fallback.
// Works on: Android 9+ (API 28+) for full v2/v3 bypass, Android 4.x+ for v1 only.

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

#include "../NativePmsHook.h"
#include "../LoadedApkSpoofer.h"

namespace omnibyte::runtime::backends {

class ApkSigKillerExAdapter {
public:
    ApkSigKillerExAdapter();
    ~ApkSigKillerExAdapter() = default;

    static constexpr const char* kName = "ApkSignatureKillerEx";
    static constexpr const char* kUpstreamUrl = "https://github.com/L-JINBIN/ApkSignatureKillerEx";
    static constexpr const char* kUpstreamBranch = "main";
    static constexpr const char* kVersion = "1.1.0";

    std::string name() const { return kName; }
    std::string version() const { return kVersion; }

    bool initialize();
    bool isActive() const { return active_; }

    bool installHook(const std::string& targetPackage, const std::string& signatureData,
                     const std::string& signingInfo = "");
    bool removeHook();

    bool injectSmali(const std::string& srcApk, const std::string& signApk, const std::string& outApk);

    void setProgressCallback(std::function<void(int percent, const std::string& message)> cb) {
        progressCb_ = std::move(cb);
    }

    std::string installedVersion() const { return kVersion; }
    bool hasUpdate() const { return false; }
    bool updateFromUpstream() { return false; }

private:
    bool active_ = false;
    std::string workDir_;
    std::string targetPackage_;
    std::string signatureData_;
    std::string signingInfo_;

    NativePmsHook nativePmsHook_;
    LoadedApkSpoofer loadedApkSpoofer_;

    bool useNativePms_ = false;
    bool useLoadedApkSpoof_ = false;

    std::function<void(int, const std::string&)> progressCb_;

    std::string exec(const std::string& cmd) const;
    void reportProgress(int pct, const std::string& msg);
};

} // namespace omnibyte::runtime::backends
