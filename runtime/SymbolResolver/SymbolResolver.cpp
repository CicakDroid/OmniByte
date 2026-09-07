// SymbolResolver — resolve symbols in remote processes via xDL.
// Source: https://github.com/hexhacking/xDL (Apache-2.0)
//
// xDL wraps dlopen/dlsym to handle Android's linker namespace restrictions.
// Without xDL, symbols in app-private libraries are unresolvable from
// non-app processes (the linker blocks dlopen across namespaces).

#include "SymbolResolver.h"

// xDL API — provided by libxDL (ExternalProject_Add from GitHub hexhacking/xDL).
// Include paths set by xdl-adapter/CMakeLists.txt.
#include <dlfcn.h>
#include <link.h>

// Stub xDL API declarations until xDL is linked at build time.
// When xDL is available, these map to xdl_open/xdl_sym/xdl_close.
// TODO: Replace with real xDL headers once ExternalProject_Add resolves.
#ifndef XDL_STUB
// If real xDL is available:
// #include <xdl.h>
#endif

namespace omnibyte::runtime {

std::optional<uintptr_t> SymbolResolver::resolveSymbol(pid_t pid,
                                                        const std::string& libName,
                                                        const std::string& symbolName) {
    // Check cache first
    {
        std::lock_guard<std::mutex> lock(mutex_);
        CacheKey key{pid, libName, symbolName};
        auto it = cache_.find(key);
        if (it != cache_.end()) return it->second;
    }

    // xDL approach: open the target library handle in the remote process context,
    // then resolve the symbol. On Android, this requires either:
    //   1. Running in the same process (in-process analysis), or
    //   2. Using ptrace + /proc/<pid>/maps to locate the library base + symbol offset
    //
    // For a cross-process resolver, we:
    //   1. Parse /proc/<pid>/maps to find the library's load address
    //   2. Use xdl_open to get a handle (works within the same namespace)
    //   3. Fall back to ELF symbol table parsing if xdl_open fails
    //
    // NOTE: When this module runs inside the target process (injected),
    //       xdl_open(libName) + xdl_sym(handle, symbolName) works directly.
    //       For external analysis, we rely on /proc/<pid>/maps + ELF parsing.

    uintptr_t result = 0;

    // Attempt 1: direct xdl API (works when in same process/namespace)
    // TODO: Wire real xDL when ExternalProject_Add resolves:
    //   void* handle = xdl_open(libName.c_str(), XDL_DEFAULT);
    //   if (handle) {
    //       result = reinterpret_cast<uintptr_t>(xdl_sym(handle, symbolName.c_str(), nullptr));
    //       xdl_close(handle);
    //   }

    // Attempt 2: fallback to maps-based offset resolution
    // Parse /proc/<pid>/maps for library base, then use ELF .dynsym
    // to find symbol offset. This is the cross-process path.
    if (result == 0) {
        // TODO: Implement ELF .dynsym parsing from /proc/<pid>/mem
        // Read ELF header at library base → find .dynsym section →
        // linear search for symbolName → return base + symbol.st_value
        (void)pid; (void)libName; (void)symbolName;
    }

    if (result == 0) return std::nullopt;

    // Cache the result
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_[CacheKey{pid, libName, symbolName}] = result;
    }

    return result;
}

void SymbolResolver::clearCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
}

size_t SymbolResolver::cacheSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_.size();
}

} // namespace omnibyte::runtime
