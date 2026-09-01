#pragma once
// bhook — Universal Android PLT hook library.
// Source: https://github.com/bytedance/bhook (MIT)

#include <cstdint>
#include <string>

namespace omnibyte::hook {

/// PLT (Procedure Linkage Table) hook engine.
/// Modifies GOT entries for stealthier hooking than inline patching.
class PLTHookEngine {
public:
    PLTHookEngine() = default;
    ~PLTHookEngine();

    /// Initialize PLT hook engine for a target library handle.
    bool init(void* handle);

    /// Hook a symbol by name.
    /// @param symbol  Symbol name to hook.
    /// @param replacement  Replacement function pointer.
    /// @param original  Output: original function pointer.
    /// @return true if hooked successfully.
    bool hookMethod(const char* symbol, void* replacement, void** original);

    /// Unhook a previously hooked symbol.
    bool unhook(const char* symbol);

    /// Check if a symbol is currently hooked.
    bool isHooked(const char* symbol) const;

private:
    void* handle_ = nullptr;
};

}  // namespace omnibyte::hook
