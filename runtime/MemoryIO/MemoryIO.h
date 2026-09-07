#pragma once
// MemoryIO — read process memory via /proc/<pid>/mem (pread) or proxy path.
// All sizes/timeouts from RuntimeConfig — no hardcoded values.

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <sys/types.h>

namespace omnibyte::runtime {

/// Memory region parsed from /proc/<pid>/maps.
struct MemoryRegion {
    uintptr_t start = 0;
    uintptr_t end = 0;
    std::string perms;      // e.g. "r-xp"
    std::string pathname;   // e.g. "/data/app/.../libil2cpp.so"
};

class MemoryIO {
public:
    MemoryIO() = default;

    /// Read `size` bytes from process memory at `addr`, chunked by `chunkSizeBytes`.
    /// Uses pread on /proc/<pid>/mem.
    std::optional<std::vector<uint8_t>> readChunk(pid_t pid, uintptr_t addr,
                                                   size_t size,
                                                   uint32_t chunkSizeBytes);

    /// Read via syscall proxy path (stealthReadStrategy == "syscall_proxy").
    /// Delegates to FreedomServiceBridge — actual anti-detection technique
    /// is backend-specific (TODO: ptrace-based, /proc/mem stealth variants).
    std::optional<std::vector<uint8_t>> readViaProxy(pid_t pid, uintptr_t addr,
                                                      size_t size);

    /// Write process memory. DISABLED by default — only active when
    /// RuntimeConfig.enableMemoryWrite == true. Bails out if flag is off.
    bool writeChunk(pid_t pid, uintptr_t addr, const uint8_t* data, size_t size,
                    bool writeEnabled);

private:
    /// Open /proc/<pid>/mem, return fd or -1.
    int openProcMem(pid_t pid);

    /// Read exactly `size` bytes from open fd at offset using pread loop.
    std::optional<std::vector<uint8_t>> preadAll(int fd, uintptr_t addr, size_t size,
                                                  uint32_t chunkSize);
};

} // namespace omnibyte::runtime
