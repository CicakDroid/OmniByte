// MemoryIO — read process memory via /proc/<pid>/mem or proxy.
// Source: Linux procfs documentation, ptrace patterns from Android RE.
// All sizes from RuntimeConfig — zero hardcoded values.

#include "MemoryIO.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace omnibyte::runtime {

std::optional<std::vector<uint8_t>> MemoryIO::readChunk(pid_t pid, uintptr_t addr,
                                                         size_t size,
                                                         uint32_t chunkSizeBytes) {
    if (size == 0 || chunkSizeBytes == 0) return std::vector<uint8_t>{};

    int fd = openProcMem(pid);
    if (fd < 0) return std::nullopt;

    auto result = preadAll(fd, addr, size, chunkSizeBytes);
    ::close(fd);
    return result;
}

std::optional<std::vector<uint8_t>> MemoryIO::readViaProxy(const std::string& path) {
    (void)path;
    return std::nullopt;
}

std::optional<std::vector<uint8_t>> MemoryIO::readViaHook(pid_t pid, uintptr_t addr,
                                                           size_t size) {
    (void)pid; (void)addr; (void)size;
    return std::nullopt;
}

bool MemoryIO::writeChunk(pid_t pid, uintptr_t addr, const uint8_t* data,
                           size_t size, bool writeEnabled) {
    if (!writeEnabled) return false;
    if (!data || size == 0) return false;

    int fd = openProcMemForWrite(pid);
    if (fd < 0) return false;

    ssize_t written = pwrite(fd, data, size, static_cast<off_t>(addr));
    ::close(fd);
    return written == static_cast<ssize_t>(size);
}

int MemoryIO::openProcMem(pid_t pid) {
    std::string path = "/proc/" + std::to_string(pid) + "/mem";
    return ::open(path.c_str(), O_RDONLY);
}

int MemoryIO::openProcMemForWrite(pid_t pid) {
    std::string path = "/proc/" + std::to_string(pid) + "/mem";
    return ::open(path.c_str(), O_WRONLY);
}

std::optional<std::vector<uint8_t>> MemoryIO::preadAll(int fd, uintptr_t addr,
                                                        size_t size,
                                                        uint32_t chunkSize) {
    std::vector<uint8_t> buf(size);
    size_t offset = 0;

    while (offset < size) {
        size_t toRead = std::min(static_cast<size_t>(chunkSize), size - offset);
        ssize_t n = ::pread(fd, buf.data() + offset, toRead,
                            static_cast<off_t>(addr + offset));
        if (n <= 0) {
            // EIO/EFAULT/EOF — partial read is still a failure for raw memory dump
            return std::nullopt;
        }
        offset += static_cast<size_t>(n);
    }

    return buf;
}

} // namespace omnibyte::runtime
