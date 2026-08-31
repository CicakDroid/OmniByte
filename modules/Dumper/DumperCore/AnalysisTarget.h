#pragma once
#include <string>
#include <cstdint>

namespace omnibyte::dumper {

// Representasi target analisis — bisa berupa file statis (APK/pak/dll/ELF)
// atau proses live yang sudah di-load ke memori (butuh runtime/MemoryIO).
// IDumperEngine::detect(), analyze(), resolveSymbols() semua menerima ini.
enum class TargetType {
    File,       // file statis: APK, .pak, .so, .dll, .win, .pck, .ung, .bsp
    Process     // live process via runtime/MemoryIO (ASLR-safe)
};

struct AnalysisTarget {
    TargetType type = TargetType::File;

    // === File-target fields ===
    std::string filePath;               // path absolut ke file (mis. "/data/app/.../base.apk")

    // === Process-target fields ===
    int pid = 0;                        // PID proses live
    uintptr_t baseAddress = 0;          // base address modul di memori (ASLR-resolved)
    std::string moduleName;             // nama modul (mis. "libil2cpp.so", "UE4Game-Win64-Shipping.exe")

    // === Helpers ===
    bool isFile() const { return type == TargetType::File; }
    bool isProcess() const { return type == TargetType::Process; }

    // Factory untuk file target
    static AnalysisTarget fromFile(const std::string& path) {
        AnalysisTarget t;
        t.type = TargetType::File;
        t.filePath = path;
        return t;
    }

    // Factory untuk live process target
    static AnalysisTarget fromProcess(int pid, uintptr_t base, const std::string& mod = "") {
        AnalysisTarget t;
        t.type = TargetType::Process;
        t.pid = pid;
        t.baseAddress = base;
        t.moduleName = mod;
        return t;
    }
};

} // namespace omnibyte::dumper
