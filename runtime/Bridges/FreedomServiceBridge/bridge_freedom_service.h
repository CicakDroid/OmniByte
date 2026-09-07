#pragma once
// FreedomServiceBridge — JNI marshalling for FreedomService.
// Pure JNI marshalling — no logic duplication, all delegating to FreedomService.

#include <jni.h>
#include <string>

namespace omnibyte::runtime::bridge {

/// JNI bridge between Kotlin UI and FreedomService.
/// Exposes root status to Settings UI.
class FreedomServiceBridge {
public:
    /// JNI_OnLoad registration — register native methods with JVM.
    static void registerMethods(JNIEnv* env);

    /// nativeHasRoot() → jboolean
    /// Check if root is currently available.
    static jboolean nativeHasRoot(JNIEnv* env, jobject thiz);

    /// nativeRequestRoot() → jboolean
    /// Trigger FreedomService::acquire() — show root prompt if needed.
    static jboolean nativeRequestRoot(JNIEnv* env, jobject thiz);

    /// nativeGetActiveRootBackend() → jstring
    /// Get name of active backend for Settings display (e.g. "KernelSU").
    static jstring nativeGetActiveRootBackend(JNIEnv* env, jobject thiz);
};

} // namespace omnibyte::runtime::bridge
