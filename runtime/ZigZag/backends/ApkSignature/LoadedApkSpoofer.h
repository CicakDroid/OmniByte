#pragma once
// LoadedApkSpoofer — runtime signature bypass via LoadedApk field spoofing.
// Hooks ActivityThread.mLoadedApk to replace ApplicationInfo.signingInfo at runtime.
//
// Technique: Access ActivityThread → mBoundApplication → mApplicationInfo → signingInfo
//            and replace the signing certificates with our own.
//   API < 28:  Hook PackageItemInfo.signatures field directly.
//   API >= 28: Hook SigningInfo class, replace signingCertificateHistory.
// Works on: Android 9+ (API 28+) for full bypass, Android 7+ for partial.

#include <string>
#include <jni.h>

namespace omnibyte::runtime::backends {

class LoadedApkSpoofer {
public:
    LoadedApkSpoofer() = default;
    ~LoadedApkSpoofer() = default;

    // --- Lifecycle ---
    /// Find ActivityThread class via JNI and resolve field/method IDs.
    bool initialize();

    /// Hook mLoadedApk to intercept ApplicationInfo and replace signingInfo.
    /// @param packageName   Target package name.
    /// @param signatureData Raw certificate bytes (DER-encoded X.509).
    bool spoof(const std::string& packageName, const std::string& signatureData);

    /// Restore original signingInfo for the given package.
    bool restore(const std::string& packageName);

    /// Check if API level supports this technique (API 28+).
    bool isSupported() const;

    bool isInitialized() const { return initialized_; }

private:
    bool initialized_ = false;
    bool hooked_ = false;
    int apiLevel_ = 0;

    JavaVM* jvm_ = nullptr;
    jclass activityThreadClass_ = nullptr;
    jfieldID mBoundAppField_ = nullptr;
    jfieldID mApplicationInfoField_ = nullptr;
    jfieldID signingInfoField_ = nullptr;

    // Saved originals for restore.
    std::string savedSigningInfo_;
    std::string targetPackage_;

    bool resolveActivityThread();
    bool resolveApplicationInfoFields();
    int getApiLevel() const;
    JNIEnv* getEnv();
};

} // namespace omnibyte::runtime::backends
