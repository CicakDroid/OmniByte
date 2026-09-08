#include <jni.h>
#include "FreedomServiceBridge/bridge_freedom_service.h"
#include "ModuleBridge/bridge_module.h"

using omnibyte::runtime::bridge::FreedomServiceBridge;
using omnibyte::runtime::bridge::ModuleBridge;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    (void)reserved;
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    FreedomServiceBridge::registerMethods(env);
    ModuleBridge::registerMethods(env);

    return JNI_VERSION_1_6;
}
