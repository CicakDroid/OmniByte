#pragma once
// AlbatrossAdapter — C++ wrapper for AlbatrossHook/AlbatrossAndroid (ART method hooking).
// Source: https://github.com/AlbatrossHook/AlbatrossAndroid (Apache-2.0)
// Author: QingWan (qingwanmail@foxmail.com)
// Version: 3.6.0
//
// Bridges IHookBackend interface to Albatross Java API via JNI.
// Supports Android 7.0–16, ARM/ARM64/x86/x86_64.
//
// ponytail: JNI bridge is inherently complex; no simpler path exists here.

#include "../../IHookEngines.h"

#include <jni.h>
#include <string>
#include <unordered_map>
#include <mutex>

namespace omnibyte::runtime::backends {

/// JNI bridge to AlbatrossHook for ART method hooking.
/// Unlike inline/PLT hooks, Albatross operates on java.lang.reflect.Method/Constructor.
/// This adapter translates address-based IHookBackend calls to method-based Albatross calls.
class AlbatrossAdapter : public IHookBackend {
public:
    AlbatrossAdapter() = default;
    ~AlbatrossAdapter() override;

    std::string name() const override { return "Albatross"; }
    bool isAvailable() const override;

    /// Hook an ART method at `addr`. `addr` is resolved to a Method* via
    /// JNI introspection. `replacement` must be a JNI native method stub.
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) override;

    /// Unhook a previously hooked ART method.
    bool unhook(uintptr_t addr) override;

    /// Raw memory patch (delegates to KittyMemory fallback — Albatross doesn't patch bytes).
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;

    /// Initialize with JNI environment. Must be called before any hook operations.
    bool init(JNIEnv* env);

    /// Initialize from a loaded library name (e.g. "albatross_core").
    bool loadLibrary(JNIEnv* env, const char* libName);

    // --- Albatross-specific API (beyond IHookBackend) ---

    /// Hook a java.lang.reflect.Method directly via Albatross.
    bool hookMethod(JNIEnv* env, jobject targetMethod, jobject hookMethod, jobject backupMethod);

    /// Unhook a previously hooked Method.
    bool unhookMethod(JNIEnv* env, jobject targetMethod, jobject hookMethod, jobject backupMethod);

    /// Check if Albatross core library is loaded and initialized.
    bool isInitialized() const { return initialized_; }

    /// Get Albatross init status (STATUS_INIT_OK=1, STATUS_DISABLED=2, etc.).
    int getInitStatus() const { return initStatus_; }

private:
    /// Resolve a native address to a java.lang.reflect.Method via art::ArtMethod introspection.
    jobject resolveMethodFromAddress(JNIEnv* env, uintptr_t addr);

    /// Cache Albatross class/method JNI references.
    bool cacheJNIClasses(JNIEnv* env);

    bool initialized_ = false;
    int initStatus_ = 4; // STATUS_NOT_INIT
    JavaVM* jvm_ = nullptr; // cached for thread-attach in unhook()

    // JNI cached references
    jclass albatrossClass_ = nullptr;
    jmethodID backupAndHook_ = nullptr;
    jmethodID unhookMethod_ = nullptr;
    jmethodID loadLibrary_ = nullptr;
    jmethodID isAvailable_ = nullptr;

    // Hook registry: address → {target, hook, backup} jobjects
    struct HookEntry {
        jobject target;
        jobject hook;
        jobject backup;
    };
    std::unordered_map<uintptr_t, HookEntry> hooks_;
    std::mutex hooksMutex_;
};

} // namespace omnibyte::runtime::backends
