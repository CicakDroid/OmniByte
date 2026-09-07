// ModuleBridge — JNI marshalling for ZigZagManager + HPT.
// Pure marshalling — all logic delegates to ZigZagManager/HPT.

#include "bridge_module.h"
#include "../../ZigZagManager/ZigZagManager.h"
#include "../../HPT/HPT.h"

namespace omnibyte::runtime::bridge {

static omnibyte::runtime::ZigZagManager* getZigZagManager() {
    static omnibyte::runtime::ZigZagManager mgr;
    return &mgr;
}

static omnibyte::runtime::HPT* getHPT() {
    static omnibyte::runtime::HPT hpt;
    return &hpt;
}

void ModuleBridge::registerMethods(JNIEnv* env) {
    jclass clazz = env->FindClass("com/omnibyte/runtime/NativeModuleBridge");
    if (!clazz) return;

    static const JNINativeMethod methods[] = {
        {"nativeActivateStealth",    "(I)Z",  (void*)nativeActivateStealth},
        {"nativeIsStealthActive",    "()Z",   (void*)nativeIsStealthActive},
        {"nativeGetActiveHookBackend", "()Ljava/lang/String;", (void*)nativeGetActiveHookBackend},
        {"nativeInstallHook",        "(JJJ)Z", (void*)nativeInstallHook},
    };
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0]));
}

jboolean ModuleBridge::nativeActivateStealth(JNIEnv* env, jobject thiz, jint pid) {
    (void)thiz;
    // TODO: Fetch RuntimeConfig to pass to selectAndActivate.
    omnibyte::dumper::config::RuntimeConfig cfg;
    auto result = getZigZagManager()->selectAndActivate(
        static_cast<pid_t>(pid), cfg);
    return result == omnibyte::dumper::DumpResult::Success ? JNI_TRUE : JNI_FALSE;
}

jboolean ModuleBridge::nativeIsStealthActive(JNIEnv* env, jobject thiz) {
    (void)thiz;
    return getZigZagManager()->activeBackend() != nullptr ? JNI_TRUE : JNI_FALSE;
}

jstring ModuleBridge::nativeGetActiveHookBackend(JNIEnv* env, jobject thiz) {
    (void)thiz;
    std::string name = getHPT()->activeBackendName();
    return env->NewStringUTF(name.c_str());
}

jboolean ModuleBridge::nativeInstallHook(JNIEnv* env, jobject thiz,
                                          jlong addr, jlong replacement, jlong originalOut) {
    (void)thiz;
    bool ok = getHPT()->hookFunction(
        static_cast<uintptr_t>(addr),
        reinterpret_cast<void*>(replacement),
        reinterpret_cast<void**>(originalOut));
    return ok ? JNI_TRUE : JNI_FALSE;
}

} // namespace omnibyte::runtime::bridge
