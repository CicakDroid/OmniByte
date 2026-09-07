#pragma once
// ModuleBridge — JNI marshalling for ZigZagManager + HPT.
// Pure JNI marshalling — logic lives in ZigZagManager/HPT.

#include <jni.h>

namespace omnibyte::runtime::bridge {

class ModuleBridge {
public:
    static void registerMethods(JNIEnv* env);

    /// nativeActivateStealth(pid) → jboolean
    static jboolean nativeActivateStealth(JNIEnv* env, jobject thiz, jint pid);

    /// nativeIsStealthActive() → jboolean
    static jboolean nativeIsStealthActive(JNIEnv* env, jobject thiz);

    /// nativeGetActiveHookBackend() → jstring
    static jstring nativeGetActiveHookBackend(JNIEnv* env, jobject thiz);

    /// nativeInstallHook(addr, replacement, originalOut) → jboolean
    static jboolean nativeInstallHook(JNIEnv* env, jobject thiz,
                                       jlong addr, jlong replacement, jlong originalOut);
};

} // namespace omnibyte::runtime::bridge
