// ApkSigKiller adapter — wraps L-JINBIN/ApkSignatureKiller for APK signature bypass.
// Source: https://github.com/L-JINBIN/ApkSignatureKiller (969 stars)
//
// PMS hook technique: intercepts PackageManager.getPackageInfo() via Xposed/reflection,
// replaces returned Signature[] with legitimate signatures from original APK.

#include "ApkSigKillerAdapter.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <array>
#include <memory>
#include <stdexcept>

namespace omnibyte::runtime::backends {

// --- Construction ---

ApkSigKillerAdapter::ApkSigKillerAdapter() {
    workDir_ = "/data/local/tmp/apksigkiller";
}

// --- Lifecycle ---

bool ApkSigKillerAdapter::initialize() {
    reportProgress(0, "Initializing ApkSignatureKiller adapter...");

    // Ensure work directory exists.
    exec("mkdir -p " + workDir_);

    // Fetch upstream latest commit.
    latestCommit_ = fetchLatestVersion();
    if (latestCommit_.empty()) {
        reportProgress(100, "Failed to fetch upstream version");
        return false;
    }

    // Check if we already have sources.
    std::string markerFile = workDir_ + "/.installed_commit";
    std::ifstream fin(markerFile);
    if (fin.is_open()) {
        std::getline(fin, installedCommit_);
        fin.close();
    }

    reportProgress(50, "Upstream: " + latestCommit_.substr(0, 8));

    // Auto-update if we have old sources or none.
    if (installedCommit_ != latestCommit_) {
        reportProgress(60, "Update available, pulling...");
        if (!updateFromUpstream()) {
            reportProgress(100, "Update failed");
            return false;
        }
    }

    reportProgress(100, "ApkSignatureKiller initialized");
    return true;
}

// --- Signature Bypass ---

bool ApkSigKillerAdapter::installHook(const std::string& targetPackage,
                                       const std::string& signatureData) {
    if (signatureData.empty()) return false;
    if (active_) return true;

    // Strategy: use reflection to hook PMS via Xposed/LSPlant.
    // This is a native-side trigger — the actual hook is in Java/Xposed land.
    // We write the hook configuration and trigger the Java bridge.

    std::string configPath = workDir_ + "/hook_config.json";
    std::ofstream cfg(configPath);
    if (!cfg.is_open()) return false;

    cfg << "{\n"
        << "  \"target_package\": \"" << targetPackage << "\",\n"
        << "  \"signature_data\": \"" << signatureData << "\",\n"
        << "  \"method\": \"pms_hook\",\n"
        << "  \"hook_getPackageInfo\": true\n"
        << "}\n";
    cfg.close();

    // Trigger the Java-side hook installer via broadcast/intent.
    // The actual PMS hook happens in PmsHookApplication.java from upstream.
    int ret = system(("am broadcast -a cc.binmt.signature.INSTALL_HOOK "
                      "--es config_path " + configPath + " 2>/dev/null").c_str());

    active_ = (ret == 0);
    return active_;
}

bool ApkSigKillerAdapter::removeHook() {
    if (!active_) return true;

    int ret = system("am broadcast -a cc.binmt.signature.REMOVE_HOOK 2>/dev/null");
    active_ = false;
    return (ret == 0);
}

bool ApkSigKillerAdapter::injectSmali(const std::string& srcApk,
                                       const std::string& signApk,
                                       const std::string& outApk) {
    if (srcApk.empty() || signApk.empty() || outApk.empty()) return false;

    // Use upstream NKillSignatureTool to inject smali.
    // Command: java -cp apksigkiller.jar cc.binmt.signature.NKillSignatureTool
    // This requires a config.txt with apk.src, apk.signed, apk.out paths.

    std::string configPath = workDir_ + "/inject_config.txt";
    std::ofstream cfg(configPath);
    if (!cfg.is_open()) return false;

    cfg << "apk.src=" << srcApk << "\n"
        << "apk.signed=" << signApk << "\n"
        << "apk.out=" << outApk << "\n"
        << "sign.enable=false\n";
    cfg.close();

    // Build classpath from fetched upstream JAR.
    std::string jarPath = workDir_ + "/ApkSignatureKiller.jar";
    std::string cmd = "cd " + workDir_ + " && "
                      "java -cp " + jarPath + " cc.binmt.signature.NKillSignatureTool 2>&1";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return false;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        reportProgress(-1, std::string(buffer));
    }
    int ret = pclose(pipe);
    return (ret == 0);
}

// --- Version Check ---

std::string ApkSigKillerAdapter::fetchLatestVersion() const {
    // GitHub API: get latest commit on master branch.
    std::string json = httpGet(
        "https://api.github.com/repos/L-JINBIN/ApkSignatureKiller/commits/master");
    if (json.empty()) return "";

    // Extract "sha" field — crude JSON parse (no dependency).
    auto pos = json.find("\"sha\":\"");
    if (pos == std::string::npos) return "";
    pos += 7;
    auto end = json.find("\"", pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

bool ApkSigKillerAdapter::hasUpdate() const {
    if (installedCommit_.empty() || latestCommit_.empty()) return false;
    return installedCommit_ != latestCommit_;
}

// --- Auto-Update ---

bool ApkSigKillerAdapter::updateFromUpstream() {
    reportProgress(10, "Cloning ApkSignatureKiller...");

    // Remove old sources if they exist.
    exec("rm -rf " + workDir_ + "/ApkSignatureKiller");

    // Shallow clone.
    std::string cmd = "git clone --depth=1 " + std::string(kUpstreamUrl) + " " +
                      workDir_ + "/ApkSignatureKiller 2>&1";
    int ret = system(cmd.c_str());
    if (ret != 0) {
        reportProgress(100, "Clone failed");
        return false;
    }

    reportProgress(50, "Building...");

    // Build the tool JAR (if gradle wrapper exists).
    std::string buildCmd = "cd " + workDir_ + "/ApkSignatureKiller && "
                           "./gradlew shadowJar 2>&1 || "
                           "gradle shadowJar 2>&1";
    ret = system(buildCmd.c_str());

    // Find and copy the built JAR.
    std::string findCmd = "find " + workDir_ + "/ApkSignatureKiller -name '*.jar' -path '*/libs/*' "
                          "-exec cp {} " + workDir_ + "/ApkSignatureKiller.jar \\;";
    system(findCmd.c_str());

    reportProgress(80, "Saving version marker...");

    // Record installed commit.
    std::ofstream fout(workDir_ + "/.installed_commit");
    if (fout.is_open()) {
        fout << latestCommit_;
        fout.close();
    }
    installedCommit_ = latestCommit_;

    reportProgress(100, "Update complete: " + latestCommit_.substr(0, 8));
    return true;
}

// --- Helpers ---

std::string ApkSigKillerAdapter::httpGet(const std::string& url) const {
    std::string cmd = "curl -sL --max-time 10 '" + url + "' 2>/dev/null";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    std::ostringstream oss;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        oss << buffer;
    }
    pclose(pipe);
    return oss.str();
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
