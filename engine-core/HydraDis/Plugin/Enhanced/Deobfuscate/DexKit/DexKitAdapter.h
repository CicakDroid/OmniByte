#pragma once
// DexKit — DEX deobfuscation adapter.
// Source: https://github.com/LuckyPray/DexKit (Apache-2.0)

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace omnibyte::deob {

/// Class information from DEX analysis.
struct ClassInfo {
    std::string name;
    std::string superClass;
    std::vector<std::string> interfaces;
    uint32_t accessFlags = 0;
};

/// Method information from DEX analysis.
struct MethodInfo {
    std::string className;
    std::string name;
    std::string signature;
    uint32_t accessFlags = 0;
};

/// Field information from DEX analysis.
struct FieldInfo {
    std::string className;
    std::string name;
    std::string type;
    uint32_t accessFlags = 0;
};

/// DEX deobfuscation result.
struct DeobResult {
    bool success = false;
    std::string message;
    std::vector<ClassInfo> classes;
    std::vector<MethodInfo> methods;
    std::vector<FieldInfo> fields;
};

/// DEX file adapter for DexKit.
/// Wraps DexKit's JNI-based API with C++ interface.
/// Note: JNI bridge is placeholder — fill at integration time.
class DexKitAdapter {
public:
    DexKitAdapter() = default;
    ~DexKitAdapter() = default;

    /// Open a DEX file for analysis.
    bool openDex(const std::string& dexPath);

    /// Close the current DEX file.
    void closeDex();

    /// Find a class by name (exact or pattern).
    std::optional<ClassInfo> findClass(const std::string& dexPath,
                                       const std::string& className) const;

    /// Find methods by name pattern.
    std::vector<MethodInfo> findMethod(const std::string& dexPath,
                                       const std::string& methodName) const;

    /// Find fields by name pattern.
    std::vector<FieldInfo> findField(const std::string& dexPath,
                                     const std::string& fieldName) const;

    /// Perform full deobfuscation analysis.
    DeobResult deobfuscate(const std::string& dexPath,
                           const std::string& obfuscatedName) const;

    /// Get all classes in DEX.
    std::vector<ClassInfo> getAllClasses(const std::string& dexPath) const;

    /// Get all methods in a class.
    std::vector<MethodInfo> getClassMethods(const std::string& dexPath,
                                            const std::string& className) const;

private:
    bool opened_ = false;
    std::string currentDexPath_;
};

}  // namespace omnibyte::deob
