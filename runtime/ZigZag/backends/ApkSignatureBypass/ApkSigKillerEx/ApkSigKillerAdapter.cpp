// ApkSigKillerEx adapter — wraps L-JINBIN/ApkSignatureKillerEx for advanced signature bypass.
// Source: https://github.com/L-JINBIN/ApkSignatureKillerEx (841 stars)
//
// Extended PMS hook: counters MT Manager's signature removal detection.
// Hooks both getPackageInfo() (v1 signatures) and getSigningInfo() (API 28+ v2/v3).

#include "ApkSigKillerAdapter.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

namespace omnibyte::runtime::backends {

// --- Construction ---

ApkSigKillerExAdapter::ApkSigKillerExAdapter() {
    workDir_ = "/data/local/tmp/apksigkillerex";
}

// --- Lifecycle ---

bool ApkSigKillerExAdapter::initialize() {
    reportProgress(0, "Initializing ApkSignatureKillerEx adapter...");

    exec("mkdir -p " + workDir_);

    // Fetch upstream latest commit.
    latestCommit_ = fetchLatestVersion();
    if (latestCommit_.empty()) {
        reportProgress(100, "Failed to fetch upstream version");
        return false;
    }

    // Check installed version.
    std::string markerFile = workDir_ + "/.installed_commit";
    std::ifstream fin(markerFile);
    if (fin.is_open()) {
        std::getline(fin, installedCommit_);
        fin.close();
    }

    reportProgress(50, "Upstream: " + latestCommit_.substr(0, 8));

    if (installedCommit_ != latestCommit_) {
        reportProgress(60, "Update available, pulling...");
        if (!updateFromUpstream()) {
            reportProgress(100, "Update failed");
            return false;
        }
    }

    reportProgress(100, "ApkSignatureKillerEx initialized");
    return true;
}

// --- Signature Bypass ---

bool ApkSigKillerExAdapter::installHook(const std::string& targetPackage,
                                         const std::string& signatureData,
                                         const std::string& signingInfo) {
    if (signatureData.empty()) return false;
    if (active_) return true;

    // Write hook config with both v1 and v2/v3 signature data.
    std::string configPath = workDir_ + "/hook_config.json";
    std::ofstream cfg(configPath);
    if (!cfg.is_open()) return false;

    cfg << "{\n"
        << "  \"target_package\": \"" << targetPackage << "\",\n"
        << "  \"signature_data\": \"" << signatureData << "\",\n"
        << "  \"signing_info\": \"" << signingInfo << "\",\n"
        << "  \"method\": \"extended_pms_hook\",\n"
        << "  \"hook_getPackageInfo\": true,\n"
        << "  \"hook_getSigningInfo\": true,\n"
        << "  \"anti_mt_detection\": true\n"
        << "}\n";
    cfg.close();

    // Trigger Java-side hook installer.
    int ret = system(("am broadcast -a cc.binmt.signature.INSTALL_EXTENDED_HOOK "
                      "--es config_path " + configPath + " 2>/dev/null").c_str());

    active_ = (ret == 0);
    return active_;
}

bool ApkSigKillerExAdapter::removeHook() {
    if (!active_) return true;

    int ret = system("am broadcast -a cc.binmt.signature.REMOVE_EXTENDED_HOOK 2>/dev/null");
    active_ = false;
    return (ret == 0);
}

bool ApkSigKillerExAdapter::injectSmali(const std::string& srcApk,
                                         const std::string& signApk,
                                         const std::string& outApk) {
    if (srcApk.empty() || signApk.empty() || outApk.empty()) return false;

    // Use upstream Ex tool to inject extended smali.
    std::string jarPath = workDir_ + "/ApkSignatureKillerEx.jar";
    std::string cmd = "cd " + workDir_ + " && "
                      "java -cp " + jarPath + " cc.binmt.signature.Killer "
                      "--src " + srcApk + " "
                      "--sign " + signApk + " "
                      "--out " + outApk + " 2>&1";

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

std::string ApkSigKillerExAdapter::fetchLatestVersion() const {
    // GitHub API: get latest commit on main branch.
    std::string json = httpGet(
        "https://api.github.com/repos/L-JINBIN/ApkSignatureKillerEx/commits/main");
    if (json.empty()) return "";

    auto pos = json.find("\"sha\":\"");
    if (pos == std::string::npos) return "";
    pos += 7;
    auto end = json.find("\"", pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

bool ApkSigKillerExAdapter::hasUpdate() const {
    if (installedCommit_.empty() || latestCommit_.empty()) return false;
    return installedCommit_ != latestCommit_;
}

// --- Auto-Update ---

bool ApkSigKillerExAdapter::updateFromUpstream() {
    reportProgress(10, "Cloning ApkSignatureKillerEx...");

    exec("rm -rf " + workDir_ + "/ApkSignatureKillerEx");

    std::string cmd = "git clone --depth=1 " + std::string(kUpstreamUrl) + " " +
                      workDir_ + "/ApkSignatureKillerEx 2>&1";
    int ret = system(cmd.c_str());
    if (ret != 0) {
        reportProgress(100, "Clone failed");
        return false;
    }

    reportProgress(50, "Building...");

    std::string buildCmd = "cd " + workDir_ + "/ApkSignatureKillerEx && "
                           "./gradlew shadowJar 2>&1 || "
                           "gradle shadowJar 2>&1";
    ret = system(buildCmd.c_str());

    std::string findCmd = "find " + workDir_ + "/ApkSignatureKillerEx -name '*.jar' -path '*/libs/*' "
                          "-exec cp {} " + workDir_ + "/ApkSignatureKillerEx.jar \\;";
    system(findCmd.c_str());

    reportProgress(80, "Saving version marker...");

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

std::string ApkSigKillerExAdapter::httpGet(const std::string& url) const {
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

std::string ApkSigKillerExAdapter::exec(const std::string& cmd) const {
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

void ApkSigKillerExAdapter::reportProgress(int pct, const std::string& msg) {
    if (progressCb_) progressCb_(pct, msg);
}

} // namespace omnibyte::runtime::backends
