#include "ApkSigKillerAdapter.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <dlfcn.h>

namespace omnibyte::runtime::backends {

ApkSigKillerAdapter::ApkSigKillerAdapter() {
    workDir_ = "/data/local/tmp/apksigkiller";
}

bool ApkSigKillerAdapter::initialize() {
    exec("mkdir -p " + workDir_);
    return true;
}

bool ApkSigKillerAdapter::installHook(const std::string& targetPackage,
                                       const std::string& signatureData) {
    if (signatureData.empty()) return false;
    if (active_) return true;

    targetPackage_ = targetPackage;
    signatureData_ = signatureData;

    // Primary: NativePmsHook (inline hook on libandroid_runtime.so)
    if (nativePmsHook_.initialize() && nativePmsHook_.hook()) {
        useNativePms_ = true;
        reportProgress(50, "NativePmsHook installed");
    }

    // Fallback: LoadedApkSpoofer (JNI field spoofing)
    if (!useNativePms_ && loadedApkSpoofer_.initialize()) {
        if (loadedApkSpoofer_.spoof(targetPackage_, signatureData_)) {
            useLoadedApkSpoof_ = true;
            reportProgress(50, "LoadedApkSpoofer fallback activated");
        }
    }

    active_ = useNativePms_ || useLoadedApkSpoof_;
    reportProgress(100, active_ ? "Hook installed" : "Hook installation failed");
    return active_;
}

bool ApkSigKillerAdapter::removeHook() {
    if (!active_) return true;

    if (useNativePms_) {
        nativePmsHook_.unhook();
        useNativePms_ = false;
    }

    if (useLoadedApkSpoof_) {
        loadedApkSpoofer_.restore(targetPackage_);
        useLoadedApkSpoof_ = false;
    }

    active_ = false;
    return true;
}

bool ApkSigKillerAdapter::injectSmali(const std::string& srcApk,
                                       const std::string& signApk,
                                       const std::string& outApk) {
    if (srcApk.empty() || signApk.empty() || outApk.empty()) return false;

    std::string configPath = workDir_ + "/inject_config.txt";
    std::ofstream cfg(configPath);
    if (!cfg.is_open()) return false;

    cfg << "apk.src=" << srcApk << "\n"
        << "apk.signed=" << signApk << "\n"
        << "apk.out=" << outApk << "\n"
        << "sign.enable=false\n";
    cfg.close();

    std::string jarPath = workDir_ + "/ApkSignatureKiller.jar";
    std::string cmd = "cd " + workDir_ + " && "
                      "java -cp " + jarPath + " cc.binmt.signature.NKillSignatureTool 2>&1";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return false;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        // consume output
    }
    int ret = pclose(pipe);
    return (ret == 0);
}

std::string ApkSigKillerAdapter::exec(const std::string& cmd) const {
    FILE* pipe = popen((cmd + " 2>&1").c_str(), "r");
    if (!pipe) return "";

    std::ostringstream oss;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        oss << buffer;
    }
    pclose(pipe);
    return oss.str();
}

void ApkSigKillerAdapter::reportProgress(int pct, const std::string& msg) {
    if (progressCb_) progressCb_(pct, msg);
}

} // namespace omnibyte::runtime::backends
