#pragma once
// AlbatrossAndroid — Java/ART hooking framework.
// Source: https://github.com/AlbatrossHook/AlbatrossAndroid (Apache-2.0)

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::hook {

/// Java method hook info.
struct JavaHookInfo {
    std::string className;
    std::string methodName;
    std::string methodSig;
    void* hooker = nullptr;
    void* original = nullptr;
};

/// Java/ART method hooking engine.
/// Hooks Java methods via ART runtime method pointer manipulation.
class JavaHookEngine {
public:
    JavaHookEngine() = default;
    ~JavaHookEngine() = default;

    /// Initialize Java hook engine (attach to ART runtime).
    bool init();

    /// Hook a Java method by class name and method name.
    bool hookMethod(const std::string& className,
                    const std::string& methodName,
                    void* hooker,
                    void** original);

    /// Unhook a previously hooked Java method.
    bool unhookMethod(const std::string& className,
                      const std::string& methodName);

    /// Find a class by name.
    void* findClass(const std::string& className) const;

    /// Find a method by class and name.
    void* findMethod(const std::string& className,
                     const std::string& methodName) const;

    /// Get count of active hooks.
    size_t getHookCount() const;

private:
    std::vector<JavaHookInfo> hooks_;
    bool initialized_ = false;
};

}  // namespace omnibyte::hook
