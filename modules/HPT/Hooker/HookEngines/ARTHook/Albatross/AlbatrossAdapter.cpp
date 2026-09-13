// AlbatrossAdapter.cpp — JNI bridge implementation for AlbatrossHook/AlbatrossAndroid.
// Source: https://github.com/AlbatrossHook/AlbatrossAndroid (Apache-2.0)
// Version: 3.6.0

#include "AlbatrossAdapter.h"

#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstring>

#define TAG "AlbatrossAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

namespace omnibyte::runtime::backends {

// --- Lifecycle ---

AlbatrossAdapter::~AlbatrossAdapter() {
    // JNI global refs are managed by caller; we just clear our cache.
    hooks_.clear();
}

bool AlbatrossAdapter::init(JNIEnv* env) {
    if (initialized_) return true;

    // Cache JavaVM for later JNIEnv retrieval
    if (!jvm_) {
        env->GetJavaVM(&jvm_);
    }

    if (!cacheJNIClasses(env)) {
        LOGE("Failed to cache JNI classes");
        return false;
    }

    // Check Albatross init status via static field
    jfieldID statusField = env->GetStaticFieldID(albatrossClass_, "initStatus", "I");
    if (statusField) {
        initStatus_ = env->GetStaticIntField(albatrossClass_, statusField);
    }

    if (initStatus_ == 1) { // STATUS_INIT_OK
        initialized_ = true;
        LOGI("Albatross already initialized (status=%d)", initStatus_);
        return true;
    }

    // Try loading via loadLibrary — flags=0 (default)
    jstring libName = env->NewStringUTF("albatross_core");
    jboolean result = env->CallStaticBooleanMethod(
        albatrossClass_, loadLibrary_, libName, (jint)0);
    env->DeleteLocalRef(libName);

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGW("Albatross loadLibrary threw exception — not available");
        return false;
    }

    if (result) {
        // Re-read init status
        initStatus_ = env->GetStaticIntField(albatrossClass_, statusField);
        initialized_ = (initStatus_ == 1);
        LOGI("Albatross loaded, status=%d", initStatus_);
    }

    return initialized_;
}

bool AlbatrossAdapter::loadLibrary(JNIEnv* env, const char* libName) {
    if (!cacheJNIClasses(env)) return false;

    jstring jlib = env->NewStringUTF(libName);
    jboolean result = env->CallStaticBooleanMethod(
        albatrossClass_, loadLibrary_, jlib, (jint)0);
    env->DeleteLocalRef(jlib);

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return false;
    }

    if (result) {
        jfieldID statusField = env->GetStaticFieldID(albatrossClass_, "initStatus", "I");
        initStatus_ = env->GetStaticIntField(albatrossClass_, statusField);
        initialized_ = (initStatus_ == 1);
    }

    return initialized_;
}

bool AlbatrossAdapter::isAvailable() const {
    // Available if Albatross is initialized OR if we can init it.
    return initStatus_ == 1 || initStatus_ == 4; // INIT_OK or NOT_INIT (can try)
}

// --- JNI Class Cache ---

bool AlbatrossAdapter::cacheJNIClasses(JNIEnv* env) {
    if (albatrossClass_) return true;

    jclass cls = env->FindClass("qing/albatross/core/Albatross");
    if (!cls) {
        LOGE("Class qing/albatross/core/Albatross not found");
        return false;
    }
    albatrossClass_ = (jclass)env->NewGlobalRef(cls);
    env->DeleteLocalRef(cls);

    backupAndHook_ = env->GetStaticMethodID(
        albatrossClass_, "backupAndHook",
        "(Ljava/lang/reflect/Member;Ljava/lang/reflect/Method;Ljava/lang/reflect/Method;)Z");
    unhookMethod_ = env->GetStaticMethodID(
        albatrossClass_, "unhookMethod",
        "(Ljava/lang/reflect/Member;Ljava/lang/reflect/Method;Ljava/lang/reflect/Method;)Z");
    loadLibrary_ = env->GetStaticMethodID(
        albatrossClass_, "loadLibrary",
        "(Ljava/lang/String;I)Z");

    if (!backupAndHook_ || !unhookMethod_ || !loadLibrary_) {
        LOGE("Failed to find Albatross method IDs");
        return false;
    }

    return true;
}

// --- Hook Operations ---

bool AlbatrossAdapter::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    // Address-based hooking is not Albatross's primary use case.
    // Albatross hooks via java.lang.reflect.Method.
    // For address-based hooks, use Inlinehook or Vector backend instead.
    LOGW("hookFunction(addr=0x%lx) not supported by Albatross — use Inlinehook backend",
         (long)addr);
    return false;
}

bool AlbatrossAdapter::unhook(uintptr_t addr) {
    std::lock_guard<std::mutex> lock(hooksMutex_);
    auto it = hooks_.find(addr);
    if (it == hooks_.end()) {
        LOGW("No hook found at addr=0x%lx", (long)addr);
        return false;
    }

    JNIEnv* env = nullptr;
    if (!jvm_) {
        LOGE("No JavaVM cached — call init() first");
        return false;
    }
    int getEnvStat = jvm_->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED) {
        if (jvm_->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            LOGE("Failed to attach thread for unhook");
            return false;
        }
    } else if (getEnvStat != JNI_OK || !env) {
        LOGE("Cannot obtain JNIEnv for unhook");
        return false;
    }

    const HookEntry& entry = it->second;
    jboolean result = env->CallStaticBooleanMethod(
        albatrossClass_, unhookMethod_, entry.target, entry.hook, entry.backup);

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGE("unhookMethod threw exception");
        return false;
    }

    env->DeleteGlobalRef(entry.target);
    env->DeleteGlobalRef(entry.hook);
    if (entry.backup) env->DeleteGlobalRef(entry.backup);
    hooks_.erase(it);

    return result;
}

bool AlbatrossAdapter::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    // Albatross does not support raw memory patching.
    // Delegate to KittyMemory if available, or fail.
    LOGW("patchMemory not supported by Albatross — use KittyMemory backend");
    return false;
}

// --- Albatross-specific Method Hooking ---

bool AlbatrossAdapter::hookMethod(JNIEnv* env, jobject targetMethod,
                                   jobject hookMethod, jobject backupMethod) {
    if (!initialized_) {
        LOGE("Albatross not initialized");
        return false;
    }

    if (!targetMethod || !hookMethod) {
        LOGE("null target or hook method");
        return false;
    }

    jboolean result = env->CallStaticBooleanMethod(
        albatrossClass_, backupAndHook_, targetMethod, hookMethod, backupMethod);

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGE("backupAndHook threw exception");
        return false;
    }

    return result;
}

bool AlbatrossAdapter::unhookMethod(JNIEnv* env, jobject targetMethod,
                                     jobject hookMethod, jobject backupMethod) {
    if (!initialized_) return false;

    jboolean result = env->CallStaticBooleanMethod(
        albatrossClass_, unhookMethod_, targetMethod, hookMethod, backupMethod);

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return false;
    }

    return result;
}

jobject AlbatrossAdapter::resolveMethodFromAddress(JNIEnv* env, uintptr_t addr) {
    LOGW("resolveMethodFromAddress(0x%lx) — not yet implemented, use hookMethod() directly",
         (long)addr);
    return nullptr;
}

// --- Updater ---

static constexpr const char* GITHUB_API_URL =
    "https://api.github.com/repos/AlbatrossHook/AlbatrossAndroid/releases/latest";
static constexpr const char* EMBEDDED_VERSION = "3.6.0";

std::string AlbatrossAdapter::fetchLatestVersion() {
    FILE* pipe = popen("curl -s -H 'Accept: application/vnd.github.v3+json' " GITHUB_API_URL, "r");
    if (!pipe) return "";

    char buf[256];
    std::string response;
    while (fgets(buf, sizeof(buf), pipe)) {
        response += buf;
    }
    pclose(pipe);

    // Extract "tag_name":"X.Y.Z" — simple substring search
    auto pos = response.find("\"tag_name\":\"");
    if (pos == std::string::npos) return "";
    pos += 12; // len of "\"tag_name\":\""
    auto end = response.find('"', pos);
    if (end == std::string::npos) return "";
    return response.substr(pos, end - pos);
}

bool AlbatrossAdapter::downloadRelease(const std::string& version, const std::string& destPath) {
    std::string url = "https://github.com/AlbatrossHook/AlbatrossAndroid/releases/download/"
                      + version + "/albatross_core-release.apk";
    std::string cmd = "curl -sL -o '" + destPath + "' '" + url + "'";
    int rc = system(cmd.c_str());
    return rc == 0;
}

std::string AlbatrossAdapter::checkAndUpdate(JNIEnv* env, const char* downloadDir) {
    std::string latest = fetchLatestVersion();
    if (latest.empty()) {
        LOGW("Could not fetch latest Albatross version");
        return EMBEDDED_VERSION;
    }

    LOGI("Embedded Albatross: %s, latest: %s", EMBEDDED_VERSION, latest.c_str());

    if (latest == EMBEDDED_VERSION) {
        LOGI("Already on latest version");
        return EMBEDDED_VERSION;
    }

    if (!downloadDir) {
        LOGW("No download dir specified, skipping update");
        return EMBEDDED_VERSION;
    }

    std::string destPath = std::string(downloadDir) + "/albatross_core.apk";
    if (downloadRelease(latest, destPath)) {
        LOGI("Downloaded Albatross %s to %s", latest.c_str(), destPath.c_str());

        jclass fileCls = env->FindClass("java/io/File");
        jmethodID ctor = env->GetMethodID(fileCls, "<init>", "(Ljava/lang/String;)V");
        jstring jpath = env->NewStringUTF(destPath.c_str());
        jobject file = env->NewObject(fileCls, ctor, jpath);
        jmethodID exists = env->GetMethodID(fileCls, "exists", "()Z");
        jboolean ok = env->CallBooleanMethod(file, exists);
        env->DeleteLocalRef(jpath);
        env->DeleteLocalRef(file);

        if (ok) {
            LOGI("Update verified: %s", destPath.c_str());
            return latest;
        }
        LOGW("Downloaded file missing after save");
    }

    LOGW("Update download failed, staying on %s", EMBEDDED_VERSION);
    return EMBEDDED_VERSION;
}

} // namespace omnibyte::runtime::backends
