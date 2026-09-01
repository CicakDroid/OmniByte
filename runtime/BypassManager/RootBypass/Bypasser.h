#pragma once
// Bypasser — Root detection bypass system.
// Source: https://github.com/LRFP-Team/Bypasser (GPL-3.0)

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::bypass {

/// Individual bypass rule result.
struct BypassResult {
    bool bypassed = false;
    std::string checkName;
    std::string method;
};

/// Root detection bypass system adapted from Bypasser.
class Bypasser {
public:
    Bypasser() = default;
    ~Bypasser() = default;

    /// Initialize bypass system.
    bool init();

    /// Apply all known root detection bypasses.
    std::vector<BypassResult> applyAll();

    /// Bypass a specific root check by name.
    BypassResult bypassCheck(const std::string& checkName);

    // --- Individual bypass methods ---

    /// Bypass "which su" check.
    bool bypassWhichSu();

    /// Bypass /system/xbin existence check.
    bool bypassSystemXbinCheck();

    /// Bypass property-based detection (ro.debuggable, ro.secure).
    bool bypassPropertyCheck();

    /// Bypass file-based detection (Superuser.apk, Kinguser, etc.).
    bool bypassFileCheck();

    /// Bypass Magisk Hide / Zygisk detection.
    bool bypassMagiskHide();

    /// Bypass test-keys / build-tag detection.
    bool bypassBuildTagCheck();

    /// Bypass PackageManager-based root check.
    bool bypassPackageManagerCheck();

private:
    bool initialized_ = false;
    std::vector<std::string> knownRootPaths_;
};

}  // namespace omnibyte::bypass
