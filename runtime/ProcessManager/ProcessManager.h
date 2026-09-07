#pragma once
// ProcessManager — attach/detach to processes, parse /proc/<pid>/maps.

#include "modules/Dumper/Dumper.h"
#include "config/Runtime/RuntimeConfig.h"
#include "MemoryIO/MemoryIO.h"

#include <cstdint>
#include <optional>
#include <string>
#include <sys/types.h>
#include <vector>

namespace omnibyte::runtime {

class ProcessManager {
public:
    ProcessManager() = default;
    ~ProcessManager() = default;

    /// Attach to a process. Verifies /proc/<pid>/status exists and not zombie.
    /// Retries up to maxRetries times with attachTimeoutMs between attempts.
    omnibyte::dumper::DumpResult attach(pid_t pid, uint32_t timeoutMs,
                                         uint32_t maxRetries);

    /// Detach from process (release ptrace if held).
    void detach();

    /// Parse /proc/<pid>/maps, return list of memory regions.
    std::vector<MemoryRegion> getMemoryMaps(pid_t pid);

    /// Find PID by package name (scan /proc/*/cmdline).
    std::optional<pid_t> findPidByPackageName(const std::string& packageName);

    /// Check if currently attached.
    bool isAttached() const;

private:
    bool attached_ = false;
    pid_t attachedPid_ = 0;
};

} // namespace omnibyte::runtime
