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

    /// Read a FILE (not live memory) via root-privileged path.
    /// Use for SemiAuto mode: reading .dat / config from another app's data dir.
    /// NOT for reading live process memory — use readChunk for that.
    std::optional<std::vector<uint8_t>> readViaProxy(const std::string& path);

    /// Read via HPT hook trampoline — placeholder until HPT is fully implemented.
    std::optional<std::vector<uint8_t>> readViaHook(pid_t pid, uintptr_t addr,
                                                    size_t size);

    /// Write process memory via pwrite on /proc/<pid>/mem.
    /// Guarded by RuntimeConfig.enableMemoryWrite — caller must pass the flag.
    bool writeChunk(pid_t pid, uintptr_t addr, const uint8_t* data, size_t size,
                    bool writeEnabled);

private:
    int openProcMem(pid_t pid);
    int openProcMemForWrite(pid_t pid);

    std::optional<std::vector<uint8_t>> preadAll(int fd, uintptr_t addr, size_t size,
                                                  uint32_t chunkSize);
};

} // namespace omnibyte::runtime
