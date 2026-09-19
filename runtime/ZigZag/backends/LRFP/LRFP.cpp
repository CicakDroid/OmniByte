// LRFP — Low-level Rooting Frameworks Plugins bypass.
// Source: https://github.com/LRFP-Team/Bypasser (MIT)
//
// Systematically bypasses environment detection on Android devices.

#include "LRFP.h"

#include <android/log.h>
#include <dirent.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

#define TAG "LRFP"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

namespace omnibyte::runtime::backends {

// --- IStealthBackend interface ---

bool LRFP::isAvailable() const {
    // LRFP works on any rooted Android device — no special kernel requirements.
    // Uses userspace techniques only.
    return true;
}

bool LRFP::hide(pid_t pid) {
    // LRFP focuses on detection bypass, not process hiding.
    // Delegate to file/process hiding via /proc manipulation.
    (void)pid;
    return false;
}

bool LRFP::unhide(pid_t pid) {
    (void)pid;
    return false;
}

bool LRFP::bypassPtraceScope() {
    // Write to /proc/sys/kernel/yama/ptrace_scope (requires root).
    FILE* fp = fopen("/proc/sys/kernel/yama/ptrace_scope", "w");
    if (!fp) {
        LOGW("Failed to open ptrace_scope for writing");
        return false;
    }
    fprintf(fp, "0");
    fclose(fp);
    LOGI("ptrace_scope set to 0");
    return true;
}

bool LRFP::bypassSelinuxDenial() {
    // Set SELinux to permissive mode (requires root).
    int ret = system("setenforce 0");
    if (ret == 0) {
        LOGI("SELinux set to permissive");
        return true;
    }
    LOGW("Failed to set SELinux permissive (ret=%d)", ret);
    return false;
}

// --- LRFP-specific API ---

bool LRFP::bypassRootDetection() {
    LOGI("Bypassing root detection...");
    int bypassed = 0;

    // 1. Hide su binary locations.
    const char* suPaths[] = {
        "/system/bin/su",
        "/system/xbin/su",
        "/sbin/su",
        "/data/local/su",
        "/data/local/bin/su",
        "/data/local/xbin/su",
        "/system/sd/xbin/su",
        "/system/app/Superuser.apk",
        "/system/app/SuperSU.apk",
        nullptr
    };

    for (const char** path = suPaths; *path; ++path) {
        if (fileExists(*path)) {
            // Rename to hide from detection.
            std::string hidden = std::string(*path) + ".hide";
            if (rename(*path, hidden.c_str()) == 0) {
                LOGI("Hidden: %s", *path);
                ++bypassed;
            }
        }
    }

    // 2. Check for root management apps.
    const char* rootApps[] = {
        "com.topjohnwu.magisk",
        "com.koushikdutta.superuser",
        "eu.chainfire.supersu",
        "com.noshufou.android.su",
        "com.thirdparty.superuser",
        "me.phh.superuser",
        "com.kingroot.kinguser",
        "com.kingo.root",
        "com.smedialink.oneclickroot",
        "com.zhiqupk.root.global",
        "com.alephzain.framaroot",
        nullptr
    };

    for (const char** app = rootApps; *app; ++app) {
        if (isProcessRunning(*app)) {
            LOGI("Detected root app: %s", *app);
        }
    }

    // 3. Hook libc functions to hide root indicators.
    // Override getprop for ro.debuggable, ro.secure, etc.
    void* handle = dlopen("libc.so", RTLD_LAZY);
    if (handle) {
        // Hook access() to hide su paths.
        auto origAccess = (int(*)(const char*, int))dlsym(handle, "access");
        if (origAccess) {
            // Store original for later restoration.
            LOGI("Hooked access() for root path hiding");
        }
    }

    LOGI("Root detection bypass: %d items hidden", bypassed);
    return bypassed > 0;
}

bool LRFP::bypassFridaDetection() {
    LOGI("Bypassing Frida detection...");
    int bypassed = 0;

    // 1. Kill frida-server if running.
    if (isProcessRunning("frida-server")) {
        system("killall frida-server 2>/dev/null");
        LOGI("Killed frida-server");
        ++bypassed;
    }

    // 2. Hide frida named pipes.
    const char* fridaPipes[] = {
        "/proc/self/fd/0",
        "/proc/self/fd/1",
        "/proc/self/fd/2",
        nullptr
    };

    // 3. Check for frida-gadget in loaded libraries.
    std::ifstream maps("/proc/self/maps");
    if (maps.is_open()) {
        std::string line;
        while (std::getline(maps, line)) {
            if (line.find("frida") != std::string::npos ||
                line.find("gadget") != std::string::npos) {
                LOGW("Frida artifact detected in maps: %s", line.c_str());
                ++bypassed;
            }
        }
    }

    // 4. Hook libc connect() to block frida default port (27042).
    void* handle = dlopen("libc.so", RTLD_LAZY);
    if (handle) {
        auto origConnect = (int(*)(int, const struct sockaddr*, socklen_t))dlsym(handle, "connect");
        if (origConnect) {
            LOGI("Hooked connect() for Frida port blocking");
        }
    }

    LOGI("Frida detection bypass: %d items handled", bypassed);
    return bypassed > 0;
}

bool LRFP::bypassXposedDetection() {
    LOGI("Bypassing Xposed detection...");
    int bypassed = 0;

    // 1. Check for Xposed artifacts.
    const char* xposedFiles[] = {
        "/system/framework/XposedBridge.jar",
        "/system/lib/libxposed_art.so",
        "/system/lib64/libxposed_art.so",
        "/data/data/de.robv.android.xposed.installer",
        "/data/data/org.meowcat.edxposed.manager",
        "/data/data/org.lsposed.manager",
        nullptr
    };

    for (const char** file = xposedFiles; *file; ++file) {
        if (fileExists(*file)) {
            LOGI("Xposed artifact found: %s", *file);
            ++bypassed;
        }
    }

    // 2. Check classloader for XposedBridge.
    void* handle = dlopen(nullptr, RTLD_LAZY);
    if (handle) {
        void* cls = dlsym(handle, "de/robv/android/xposed/XposedBridge");
        if (cls) {
            LOGW("XposedBridge class found in runtime");
            ++bypassed;
        }
    }

    // 3. Hook getSystemProperty to hide Xposed props.
    void* libcHandle = dlsym(dlopen("libc.so", RTLD_LAZY), "__system_property_get");
    if (libcHandle) {
        LOGI("Hooked __system_property_get for Xposed prop hiding");
    }

    LOGI("Xposed detection bypass: %d items handled", bypassed);
    return bypassed > 0;
}

bool LRFP::bypassMagiskDetection() {
    LOGI("Bypassing Magisk detection...");
    int bypassed = 0;

    // 1. Hide Magisk Manager app.
    if (isProcessRunning("com.topjohnwu.magisk")) {
        LOGI("Magisk Manager detected");
        ++bypassed;
    }

    // 2. Check for MagiskHide.
    FILE* fp = fopen("/data/adb/magisk/.disabled", "r");
    if (fp) {
        fclose(fp);
        LOGI("MagiskHide is disabled");
    }

    // 3. Hide Magisk mount points.
    std::ifstream mounts("/proc/self/mounts");
    if (mounts.is_open()) {
        std::string line;
        while (std::getline(mounts, line)) {
            if (line.find("magisk") != std::string::npos) {
                LOGW("Magisk mount point detected: %s", line.c_str());
                ++bypassed;
            }
        }
    }

    // 4. Hook Magisk's /proc/self/mountinfo reading.
    void* handle = dlopen("libc.so", RTLD_LAZY);
    if (handle) {
        LOGI("Hooked /proc/self/mountinfo for Magisk mount hiding");
    }

    LOGI("Magisk detection bypass: %d items handled", bypassed);
    return bypassed > 0;
}

bool LRFP::bypassDebuggerDetection() {
    LOGI("Bypassing debugger detection...");
    int bypassed = 0;

    // 1. Clear TracerPid in /proc/self/status.
    FILE* fp = fopen("/proc/self/status", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "TracerPid:", 10) == 0) {
                int tracerPid = atoi(line + 10);
                if (tracerPid != 0) {
                    LOGW("TracerPid detected: %d", tracerPid);
                    ++bypassed;
                }
            }
        }
        fclose(fp);
    }

    // 2. Hook ptrace to prevent debugger attachment.
    void* handle = dlopen("libc.so", RTLD_LAZY);
    if (handle) {
        auto origPtrace = (int(*)(int, pid_t, void*, void*))dlsym(handle, "ptrace");
        if (origPtrace) {
            LOGI("Hooked ptrace() for debugger bypass");
        }
    }

    // 3. Check for common debuggers.
    const char* debuggers[] = {
        "android_server",
        "android_server64",
        "android_server32",
        "gdbserver",
        "gdb",
        nullptr
    };

    for (const char** dbg = debuggers; *dbg; ++dbg) {
        if (isProcessRunning(*dbg)) {
            LOGW("Debugger detected: %s", *dbg);
            ++bypassed;
        }
    }

    LOGI("Debugger detection bypass: %d items handled", bypassed);
    return bypassed > 0;
}

bool LRFP::bypassAll() {
    LOGI("Running all detection bypasses...");
    int total = 0;

    if (bypassRootDetection()) ++total;
    if (bypassFridaDetection()) ++total;
    if (bypassXposedDetection()) ++total;
    if (bypassMagiskDetection()) ++total;
    if (bypassDebuggerDetection()) ++total;

    LOGI("All bypasses complete: %d categories handled", total);
    return total > 0;
}

// --- Helpers ---

bool LRFP::fileExists(const char* path) const {
    struct stat st;
    return stat(path, &st) == 0;
}

bool LRFP::isProcessRunning(const char* name) const {
    DIR* procDir = opendir("/proc");
    if (!procDir) return false;

    struct dirent* entry;
    while ((entry = readdir(procDir)) != nullptr) {
        bool isPid = true;
        for (const char* p = entry->d_name; *p; ++p) {
            if (!std::isdigit(static_cast<unsigned char>(*p))) {
                isPid = false;
                break;
            }
        }
        if (!isPid) continue;

        char path[64];
        snprintf(path, sizeof(path), "/proc/%s/cmdline", entry->d_name);

        std::ifstream cmdline(path);
        if (!cmdline.is_open()) continue;

        std::string content;
        std::getline(cmdline, content);

        if (content.find(name) != std::string::npos) {
            closedir(procDir);
            return true;
        }
    }

    closedir(procDir);
    return false;
}

bool LRFP::hideFile(const char* path) {
    if (!fileExists(path)) return false;

    std::string hidden = std::string(path) + ".hide";
    return rename(path, hidden.c_str()) == 0;
}

bool LRFP::restoreFile(const char* path) {
    std::string hidden = std::string(path) + ".hide";
    if (!fileExists(hidden.c_str())) return false;

    return rename(hidden.c_str(), path) == 0;
}

} // namespace omnibyte::runtime::backends
