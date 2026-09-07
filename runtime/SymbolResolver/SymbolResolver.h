#pragma once
// SymbolResolver — resolve symbols in remote processes via xDL.
// xDL handles Android linker namespace restrictions that break plain dlopen/dlsym.
// Source: https://github.com/hexhacking/xDL (Apache-2.0)
//
// Cache: per (pid, libName, symbolName) → resolved address.

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <mutex>

#include <sys/types.h>

namespace omnibyte::runtime {

class SymbolResolver {
public:
    SymbolResolver() = default;

    /// Resolve a symbol address in a remote process.
    /// Uses xdl_open/xdl_sym which bypasses Android linker namespace restrictions.
    std::optional<uintptr_t> resolveSymbol(pid_t pid, const std::string& libName,
                                            const std::string& symbolName);

    /// Clear the resolve cache (e.g. after process restart).
    void clearCache();

    /// Get cache stats for diagnostics.
    size_t cacheSize() const;

private:
    /// Cache key: pid + libName + symbolName.
    struct CacheKey {
        pid_t pid;
        std::string libName;
        std::string symbolName;

        bool operator==(const CacheKey& o) const {
            return pid == o.pid && libName == o.libName && symbolName == o.symbolName;
        }
    };

    struct CacheKeyHash {
        size_t operator()(const CacheKey& k) const {
            size_t h = std::hash<int>{}(k.pid);
            h ^= std::hash<std::string>{}(k.libName) << 1;
            h ^= std::hash<std::string>{}(k.symbolName) << 2;
            return h;
        }
    };

    mutable std::mutex mutex_;
    std::unordered_map<CacheKey, uintptr_t, CacheKeyHash> cache_;
};

} // namespace omnibyte::runtime
