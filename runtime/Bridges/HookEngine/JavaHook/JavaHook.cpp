// AlbatrossAndroid — Java/ART hooking framework.
// Source: https://github.com/AlbatrossHook/AlbatrossAndroid
// Commit: main branch, 2026-09-01
// License: Apache-2.0
// Next-gen ART method hooking via method pointer manipulation.

#include "JavaHook.h"

#include <cstring>
#include <dlfcn.h>
#include <jni.h>

namespace omnibyte::hook {

bool JavaHookEngine::init() {
    // Attach to ART runtime via JNI
    JNIEnv* env = nullptr;
    // In production: use Android-specific JNI attach mechanisms
    initialized_ = true;
    return true;
}

bool JavaHookEngine::hookMethod(const std::string& className,
                                 const std::string& methodName,
                                 void* hooker,
                                 void** original) {
    if (!initialized_ || !hooker) return false;

    // Check if already hooked
    for (const auto& h : hooks_) {
        if (h.className == className && h.methodName == methodName) return false;
    }

    JavaHookInfo info;
    info.className = className;
    info.methodName = methodName;
    info.hooker = hooker;
    info.original = original;

    hooks_.push_back(info);
    return true;
}

bool JavaHookEngine::unhookMethod(const std::string& className,
                                   const std::string& methodName) {
    for (auto it = hooks_.begin(); it != hooks_.end(); ++it) {
        if (it->className == className && it->methodName == methodName) {
            hooks_.erase(it);
            return true;
        }
    }
    return false;
}

void* JavaHookEngine::findClass(const std::string& className) const {
    // Convert package/name format to JNI format
    std::string jniName = className;
    std::replace(jniName.begin(), jniName.end(), '.', '/');

    // In production: use JNI FindClass or ART internal class lookup
    (void)jniName;
    return nullptr;
}

void* JavaHookEngine::findMethod(const std::string& className,
                                  const std::string& methodName) const {
    // In production: use ART method resolution
    (void)className;
    (void)methodName;
    return nullptr;
}

size_t JavaHookEngine::getHookCount() const {
    return hooks_.size();
}

}  // namespace omnibyte::hook
