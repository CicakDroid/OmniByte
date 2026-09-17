// ApkSignatureBypass — orchestrator for APK signature verification bypass.
// Coordinates ApkSigKiller (v1) and ApkSigKillerEx (v2/v3 + anti-MT).

#include "ApkSignatureBypass.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

namespace omnibyte::runtime::backends {

// --- Construction ---

ApkSignatureBypass::ApkSignatureBypass() = default;

// --- Lifecycle ---

bool ApkSignatureBypass::initialize() {
    if (initialized_) return true;

    reportProgress(0, "Initializing ApkSignatureBypass...");

    // Initialize both adapters in parallel (each fetches upstream version).
    bool killerOk = killer_.initialize();
    bool killerExOk = killerEx_.initialize();

    if (!killerOk && !killerExOk) {
        reportProgress(100, "Both adapters failed to initialize");
        return false;
    }

    initialized_ = true;

    std::string msg = "Initialized: ";
    if (killerOk) msg += "ApkSigKiller(" + std::string(killer_.installedVersion().substr(0, 8)) + ") ";
    if (killerExOk) msg += "ApkSigKillerEx(" + std::string(killerEx_.installedVersion().substr(0, 8)) + ") ";
    reportProgress(100, msg);

    return true;
}

void ApkSignatureBypass::shutdown() {
    killer_.removeHook();
    killerEx_.removeHook();
    initialized_ = false;
}

// --- Online Bypass ---

BypassResult ApkSignatureBypass::bypassPackage(const std::string& packageName,
                                                const std::string& signatureData,
                                                BypassStrategy strategy) {
    BypassResult result;

    if (!initialized_) {
        result.message = "Not initialized — call initialize() first";
        return result;
    }

    if (strategy == BypassStrategy::Auto) {
        strategy = detectBestStrategy();
    }

    switch (strategy) {
        case BypassStrategy::V1Only:
            if (killer_.installHook(packageName, signatureData)) {
                result.success = true;
                result.method = "ApkSignatureKiller";
                result.message = "V1 hook installed";
            } else {
                result.message = "ApkSigKiller hook failed";
            }
            break;

        case BypassStrategy::V1PlusV2V3:
            if (killerEx_.installHook(packageName, signatureData)) {
                result.success = true;
                result.method = "ApkSignatureKillerEx";
                result.message = "V1+V2/V3 hook installed (anti-MT)";
            } else {
                result.message = "ApkSigKillerEx hook failed";
            }
            break;

        case BypassStrategy::Both:
            if (killer_.installHook(packageName, signatureData) &&
                killerEx_.installHook(packageName, signatureData)) {
                result.success = true;
                result.method = "Both";
                result.message = "V1 + V1+V2/V3 hooks installed";
            } else {
                result.message = "One or both hooks failed";
            }
            break;

        case BypassStrategy::Auto:
        default:
            result.message = "Auto strategy should have been resolved";
            break;
    }

    return result;
}

bool ApkSignatureBypass::removeAllHooks() {
    bool a = killer_.removeHook();
    bool b = killerEx_.removeHook();
    return a && b;
}

// --- Offline Bypass ---

BypassResult ApkSignatureBypass::injectApk(const std::string& srcApk,
                                            const std::string& signApk,
                                            const std::string& outApk,
                                            BypassStrategy strategy) {
    BypassResult result;

    if (!initialized_) {
        result.message = "Not initialized";
        return result;
    }

    if (strategy == BypassStrategy::Auto) {
        strategy = detectBestStrategy();
    }

    // For offline injection, prefer ApkSigKillerEx (handles v2/v3).
    switch (strategy) {
        case BypassStrategy::V1Only:
        case BypassStrategy::V1PlusV2V3:
            // Try Ex first (more comprehensive), fall back to base.
            if (killerEx_.injectSmali(srcApk, signApk, outApk)) {
                result.success = true;
                result.method = "ApkSignatureKillerEx";
                result.message = "Injected extended signature bypass smali";
            } else if (killer_.injectSmali(srcApk, signApk, outApk)) {
                result.success = true;
                result.method = "ApkSignatureKiller";
                result.message = "Injected V1 signature bypass smali";
            } else {
                result.message = "Both injection methods failed";
            }
            break;

        case BypassStrategy::Both:
            if (killerEx_.injectSmali(srcApk, signApk, outApk)) {
                result.success = true;
                result.method = "Both";
                result.message = "Injected extended bypass smali";
            } else {
                result.message = "Injection failed";
            }
            break;

        default:
            break;
    }

    return result;
}

// --- Version Management ---

ApkSignatureBypass::UpdateInfo ApkSignatureBypass::checkForUpdates() const {
    UpdateInfo info;
    info.killerHasUpdate = killer_.hasUpdate();
    info.killerExHasUpdate = killerEx_.hasUpdate();
    info.killerInstalled = killer_.installedVersion();
    info.killerLatest = killer_.installedVersion(); // already latest if no update
    info.killerExInstalled = killerEx_.installedVersion();
    info.killerExLatest = killerEx_.installedVersion();
    return info;
}

bool ApkSignatureBypass::updateAll() {
    bool a = killer_.updateFromUpstream();
    bool b = killerEx_.updateFromUpstream();
    return a || b;
}

// --- Helpers ---

BypassStrategy ApkSignatureBypass::detectBestStrategy() const {
    int api = getApiLevel();

    // API 28+ (Android 9+) supports v2/v3 signatures → use Ex.
    if (api >= 28) return BypassStrategy::V1PlusV2V3;

    // Older Android → V1 only.
    return BypassStrategy::V1Only;
}

int ApkSignatureBypass::getApiLevel() const {
    // Read from system property.
    FILE* pipe = popen("getprop ro.build.version.sdk 2>/dev/null", "r");
    if (!pipe) return 0;

    char buf[16] = {};
    fgets(buf, sizeof(buf), pipe);
    pclose(pipe);

    int level = 0;
    for (char* p = buf; *p >= '0' && *p <= '9'; ++p) {
        level = level * 10 + (*p - '0');
    }
    return level;
}

void ApkSignatureBypass::reportProgress(int pct, const std::string& msg) {
    if (progressCb_) progressCb_(pct, msg);
}

} // namespace omnibyte::runtime::backends
