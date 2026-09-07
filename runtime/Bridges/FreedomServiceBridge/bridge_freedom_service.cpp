// FreedomServiceBridge — JNI marshalling for FreedomService.
// Pure marshalling — all logic delegates to FreedomService.

#include "bridge_freedom_service.h"
#include "../../FreedomService/FreedomService.h"

namespace omnibyte::runtime::bridge {

static omnibyte::runtime::FreedomService* getFreedomService() {
    // In production: retrieve from Runtime facade or use a shared instance.
    // For now, return a static instance.
    static omnibyte::runtime::FreedomService service;
    return &service;
}

void FreedomServiceBridge::registerMethods(JNIEnv* env) {
    jclass clazz = env->FindClass("com/omnibyte/runtime/NativeFreedomService");
    if (!clazz) return;

    static const JNINativeMethod methods[] = {
        {"nativeHasRoot",              "()Z",  (void*)nativeHasRoot},
        {"nativeRequestRoot",          "()Z",  (void*)nativeRequestRoot},
        {"nativeGetActiveRootBackend", "()Ljava/lang/String;", (void*)nativeGetActiveRootBackend},
    };
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0]));
}

jboolean FreedomServiceBridge::nativeHasRoot(JNIEnv* env, jobject thiz) {
    (void)thiz;
    auto* svc = getFreedomService();
    return svc->hasRoot() ? JNI_TRUE : JNI_FALSE;
}

jboolean FreedomServiceBridge::nativeRequestRoot(JNIEnv* env, jobject thiz) {
    (void)thiz;
    auto* svc = getFreedomService();
    // Priority list from RuntimeConfig — loaded at init time.
    // TODO: Fetch from RuntimeConfig instead of hardcoding.
    std::vector<std::string> priority = {"KernelSU", "SukiSU-Ultra", "Sui", "RootThread"};
    return svc->acquire(priority) ? JNI_TRUE : JNI_FALSE;
}

jstring FreedomServiceBridge::nativeGetActiveRootBackend(JNIEnv* env, jobject thiz) {
    (void)thiz;
    auto* svc = getFreedomService();
    std::string name = svc->activeBackendName();
    return env->NewStringUTF(name.c_str());
}

} // namespace omnibyte::runtime::bridge
