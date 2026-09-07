#pragma once

#include "modules/Dumper/Dumper.h"
#include "config/Runtime/RuntimeConfig.h"

#include "ProcessManager/ProcessManager.h"
#include "MemoryIO/MemoryIO.h"
#include "SymbolResolver/SymbolResolver.h"
#include "FreedomService/FreedomService.h"
#include "ZigZagManager/ZigZagManager.h"
#include "HPT/HPT.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace omnibyte::runtime {

class Runtime {
public:
    Runtime() = default;
    ~Runtime() = default;

    omnibyte::dumper::DumpResult attach(pid_t pid,
                                         const config::RuntimeConfig& cfg);

    void detach();

    std::optional<std::vector<uint8_t>> readMemory(uintptr_t address, size_t size);

    std::optional<uintptr_t> resolveSymbol(const std::string& libName,
                                            const std::string& symbolName);

    std::optional<std::vector<uint8_t>> readFilePrivileged(const std::filesystem::path& path);

    omnibyte::dumper::DumpResult activateStealth(pid_t pid);

    omnibyte::dumper::DumpResult installHook(uintptr_t addr, void* replacement,
                                              void** originalOut);

    bool isAttached() const;
    bool hasRoot() const;
    bool isStealthActive() const;

private:
    config::RuntimeConfig cfg_;
    pid_t pid_ = 0;

    ProcessManager processMgr_;
    MemoryIO memoryIO_;
    SymbolResolver symbolResolver_;
    FreedomService freedomService_;
    ZigZagManager zigZagManager_;
    HPT hpt_;
};

} // namespace omnibyte::runtime
