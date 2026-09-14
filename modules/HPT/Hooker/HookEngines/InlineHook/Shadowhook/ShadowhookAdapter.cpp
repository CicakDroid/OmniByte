// ShadowhookAdapter.cpp — Implementation for bytedance/android-inline-hook inline hooking backend.
// Source: https://github.com/bytedance/android-inline-hook
// License: MIT
// Version: 2.0.1

#include "ShadowhookAdapter.h"
#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>

#define TAG "ShadowhookAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// --- shadowhook C API declarations (avoid header dependency) ---
extern "C" {
    const char *shadowhook_get_version(void);
    int shadowhook_init(shadowhook_mode_t default_mode, bool debuggable);
    int shadowhook_get_init_errno(void);
    void *shadowhook_hook_func_addr(void *func_addr, void *new_addr, void **orig_addr);
    void *shadowhook_hook_sym_addr(void *sym_addr, void *new_addr, void **orig_addr);
    void *shadowhook_hook_sym_name(const char *lib_name, const char *sym_name,
                                   void *new_addr, void **orig_addr);
    int shadowhook_unhook(void *stub);
    void shadowhook_set_debuggable(bool debuggable);
}

namespace omnibyte::runtime::backends {

// --- Updater ---

static constexpr const char* GITHUB_API_URL =
    "https://api.github.com/repos/bytedance/android-inline-hook/releases/latest";
static constexpr const char* EMBEDDED_VERSION = "2.0.1";

std::string ShadowhookAdapter::fetchLatestVersion() {
    FILE* pipe = popen("curl -s -H 'Accept: application/vnd.github.v3+json' " GITHUB_API_URL, "r");
    if (!pipe) return "";

    char buf[256];
    std::string response;
    while (fgets(buf, sizeof(buf), pipe)) {
        response += buf;
    }
    pclose(pipe);

    // Extract "tag_name":"vX.Y.Z"
    auto pos = response.find("\"tag_name\":\"");
    if (pos == std::string::npos) return "";
    pos += 12;
    auto end = response.find('"', pos);
    if (end == std::string::npos) return "";
    return response.substr(pos, end - pos);
}

bool ShadowhookAdapter::downloadRelease(const std::string& version, const std::string& destPath) {
    std::string url = "https://github.com/bytedance/android-inline-hook/releases/download/"
                      + version + "/libshadowhook.so";
    std::string cmd = "curl -sL -o '" + destPath + "' '" + url + "'";
    int rc = system(cmd.c_str());
    return rc == 0;
}

// --- Lifecycle ---

ShadowhookAdapter::~ShadowhookAdapter() {
    // Unhook all remaining hooks
    std::lock_guard<std::mutex> lock(hooksMutex_);
    for (auto& [addr, stub] : hooks_) {
        if (stub) {
            shadowhook_unhook(stub);
        }
    }
    hooks_.clear();
}

bool ShadowhookAdapter::init(shadowhook_mode_t mode, bool debug) {
    if (initialized_) return true;

    mode_ = mode;
    int result = shadowhook_init(mode, debug);

    if (result == 0) { // SHADOWHOOK_ERRNO_OK
        initialized_ = true;
        initError_ = 0;
        LOGI("Shadowhook initialized successfully (mode=%d, debug=%d)", mode, debug);
        return true;
    }

    initError_ = shadowhook_get_init_errno();
    LOGE("Shadowhook init failed with errno %d", initError_);
    return false;
}

bool ShadowhookAdapter::isAvailable() const {
    return initialized_ || initError_ == -1; // NOT_INIT or initialized
}

const char* ShadowhookAdapter::getVersion() {
    return shadowhook_get_version();
}

// --- Hook Operations ---

void* ShadowhookAdapter::hookByAddr(void* funcAddr, void* newAddr, void** origAddr) {
    if (!initialized_) {
        LOGE("Shadowhook not initialized");
        return nullptr;
    }

    void* stub = shadowhook_hook_func_addr(funcAddr, newAddr, origAddr);
    if (!stub) {
        LOGW("hookByAddr failed for %p", funcAddr);
    }
    return stub;
}

void* ShadowhookAdapter::hookByName(const char* libName, const char* symName,
                                    void* newAddr, void** origAddr) {
    if (!initialized_) {
        LOGE("Shadowhook not initialized");
        return nullptr;
    }

    void* stub = shadowhook_hook_sym_name(libName, symName, newAddr, origAddr);
    if (!stub) {
        LOGW("hookByName failed for %s:%s", libName ? libName : "?", symName ? symName : "?");
    }
    return stub;
}

void* ShadowhookAdapter::hookBySym(void* symAddr, void* newAddr, void** origAddr) {
    if (!initialized_) {
        LOGE("Shadowhook not initialized");
        return nullptr;
    }

    void* stub = shadowhook_hook_sym_addr(symAddr, newAddr, origAddr);
    if (!stub) {
        LOGW("hookBySym failed for %p", symAddr);
    }
    return stub;
}

bool ShadowhookAdapter::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    if (!initialized_) {
        LOGE("Shadowhook not initialized");
        return false;
    }

    void* stub = shadowhook_hook_func_addr((void*)addr, replacement, originalOut);
    if (!stub) {
        LOGW("hookFunction failed for addr=0x%lx", (long)addr);
        return false;
    }

    std::lock_guard<std::mutex> lock(hooksMutex_);
    hooks_[addr] = stub;
    return true;
}

bool ShadowhookAdapter::unhook(uintptr_t addr) {
    std::lock_guard<std::mutex> lock(hooksMutex_);
    auto it = hooks_.find(addr);
    if (it == hooks_.end()) {
        LOGW("No hook found at addr=0x%lx", (long)addr);
        return false;
    }

    int result = shadowhook_unhook(it->second);
    if (result != 0) {
        LOGW("shadowhook_unhook failed with errno %d", result);
        return false;
    }

    hooks_.erase(it);
    return true;
}

bool ShadowhookAdapter::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    LOGW("patchMemory not supported by Shadowhook — use KittyMemory backend");
    return false;
}

// --- Updater ---

std::string ShadowhookAdapter::checkAndUpdate(const char* downloadDir) {
    std::string latest = fetchLatestVersion();
    if (latest.empty()) {
        LOGW("Could not fetch latest shadowhook version");
        return EMBEDDED_VERSION;
    }

    LOGI("Embedded shadowhook: %s, latest: %s", EMBEDDED_VERSION, latest.c_str());

    if (latest == EMBEDDED_VERSION) {
        LOGI("Already on latest version");
        return EMBEDDED_VERSION;
    }

    if (!downloadDir) {
        LOGW("No download dir specified, skipping update");
        return EMBEDDED_VERSION;
    }

    std::string destPath = std::string(downloadDir) + "/libshadowhook.so";
    if (downloadRelease(latest, destPath)) {
        LOGI("Downloaded shadowhook %s to %s", latest.c_str(), destPath.c_str());

        // Verify file exists and has content
        struct stat st;
        if (stat(destPath.c_str(), &st) == 0 && st.st_size > 0) {
            LOGI("Update verified: %s (%ld bytes)", destPath.c_str(), (long)st.st_size);
            return latest;
        }
        LOGW("Downloaded file missing or empty after save");
    }

    LOGW("Update download failed, staying on %s", EMBEDDED_VERSION);
    return EMBEDDED_VERSION;
}

} // namespace omnibyte::runtime::backends
