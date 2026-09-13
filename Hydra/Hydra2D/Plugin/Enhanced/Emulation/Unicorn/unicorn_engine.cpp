// ── unicorn_engine.cpp ─────────────────────────────────────────────
// Implementasi UnicornEngine wrapper.
//
// Strategy:
//   1. Map Instruction[] bytes ke memory Unicorn pada base address
//   2. Setup register awal (PC = base address, SP = stack top)
//   3. Hook invalid memory access untuk error handling
//   4. Hook memory access (opsional) untuk trace
//   5. Jalankan uc_emu_start dengan timeout dan max instruction limit
//   6. Kumpulkan trace dari setiap instruksi yang dieksekusi
//
// Catatan:
//   - Unicorn v2.1.4 API: uc_open, uc_mem_map, uc_emu_start, uc_reg_read/write
//   - Hook UC_HOOK_CODE dipakai untuk trace per-instruksi
//   - Hook UC_HOOK_MEM_READ/WRITE dipakai untuk memory access trace
//   - Hook UC_HOOK_MEM_UNMAPPED dipakai untuk invalid access detection
//
// Source:
//   - unicorn-engine/unicorn: include/unicorn/unicorn.h
//   - unicorn-engine/unicorn: include/unicorn/arm64.h
//   - unicorn-engine/unicorn: samples/sample_arm64.c

#include "unicorn_engine.h"
#include "Disassembler/IDisassembler.h"
#include <chrono>
#include <cstring>
#include <sstream>
#include <algorithm>

// ── Unicorn headers (kondisional) ──────────────────────────────
// Saat compile dengan Unicorn installed, include header asli.
// Saat compile tanpa Unicorn (CI/stub mode), pakai inline stubs.
#ifdef UNICORN_FOUND
#include <unicorn/unicorn.h>
#include <unicorn/arm64.h>
#include <unicorn/x86.h>
#else
// Stub types untuk compile tanpa Unicorn — BUKAN library asli.
// Build production WAJIB install Unicorn v2.1.4 dan define UNICORN_FOUND.
//
// Source: unicorn-engine/unicorn/include/unicorn/unicorn.h
typedef void* uc_engine;
typedef unsigned int uc_err;
typedef unsigned int uc_arch;
typedef unsigned int uc_mode;

#define UC_ARCH_ARM64 0
#define UC_ARCH_ARM   1
#define UC_ARCH_X86   2
#define UC_ARCH_X86_64 3
#define UC_MODE_ARM   0
#define UC_MODE_THUMB 0x10
#define UC_MODE_32    0
#define UC_MODE_64    0
#define UC_HOOK_CODE  0
#define UC_HOOK_MEM_READ  0
#define UC_HOOK_MEM_WRITE 0
#define UC_HOOK_MEM_UNMAPPED 0
#define UC_PROT_ALL 7

// uc_err codes
#define UC_ERR_OK    0
#define UC_ERR_NOMEM 1
#define UC_ERR_ARCH  2

static inline uc_err uc_open(uc_arch, uc_mode, uc_engine** uc) { *uc = nullptr; return 1; }
static inline uc_err uc_close(uc_engine*) { return 0; }
static inline uc_err uc_mem_map(uc_engine*, uint64_t, size_t, uint32_t) { return 1; }
static inline uc_err uc_mem_write(uc_engine*, uint64_t, const void*, size_t) { return 1; }
static inline uc_err uc_mem_read(uc_engine*, uint64_t, void*, size_t) { return 1; }
static inline uc_err uc_reg_write(uc_engine*, int, const void*) { return 1; }
static inline uc_err uc_reg_read(uc_engine*, int, void*) { return 1; }
static inline uc_err uc_emu_start(uc_engine*, uint64_t, uint64_t, uint64_t, size_t) { return 1; }
static inline uc_err uc_hook_add(uc_engine*, void*, int, void*, void*, uint64_t, uint64_t) { return 1; }
static inline uc_err uc_hook_del(uc_engine*, void*) { return 0; }
static inline const char* uc_strerror(uc_err) { return "stub: Unicorn not installed"; }

// ARM64 register IDs (from unicorn/arm64.h)
#define UC_ARM64_REG_X0  0
#define UC_ARM64_REG_X1  1
#define UC_ARM64_REG_X2  2
#define UC_ARM64_REG_X3  3
#define UC_ARM64_REG_X4  4
#define UC_ARM64_REG_X5  5
#define UC_ARM64_REG_X6  6
#define UC_ARM64_REG_X7  7
#define UC_ARM64_REG_X8  8
#define UC_ARM64_REG_X9  9
#define UC_ARM64_REG_X10 10
#define UC_ARM64_REG_X11 11
#define UC_ARM64_REG_X12 12
#define UC_ARM64_REG_X13 13
#define UC_ARM64_REG_X14 14
#define UC_ARM64_REG_X15 15
#define UC_ARM64_REG_X16 16
#define UC_ARM64_REG_X17 17
#define UC_ARM64_REG_X18 18
#define UC_ARM64_REG_X19 19
#define UC_ARM64_REG_X20 20
#define UC_ARM64_REG_X21 21
#define UC_ARM64_REG_X22 22
#define UC_ARM64_REG_X23 23
#define UC_ARM64_REG_X24 24
#define UC_ARM64_REG_X25 25
#define UC_ARM64_REG_X26 26
#define UC_ARM64_REG_X27 27
#define UC_ARM64_REG_X28 28
#define UC_ARM64_REG_X29 29
#define UC_ARM64_REG_X30 30
#define UC_ARM64_REG_SP  31
#define UC_ARM64_REG_PC  32

// x86 register IDs (from unicorn/x86.h)
#define UC_X86_REG_EAX 0
#define UC_X86_REG_EBX 1
#define UC_X86_REG_ECX 2
#define UC_X86_REG_EDX 3
#define UC_X86_REG_ESP 4
#define UC_X86_REG_EBP 5
#define UC_X86_REG_EIP 6
#endif

namespace omnibyte::hydradis {

// ── Internal state ─────────────────────────────────────────────

// Mapping dari DisassemblerArch ke Unicorn arch+mode
struct ArchMapping {
    uc_arch arch;
    uc_mode mode;
    int registerSize;  // bytes per register
};

static ArchMapping mapArch(int disasmArch) {
    // DisassemblerArch enum values (from IDisassembler.h):
    //   ARM=0, ARM_Thumb=1, ARM64=2, x86=3, x86_64=4
    switch (disasmArch) {
        case 2:  return {UC_ARCH_ARM64, UC_MODE_64, 8};
        case 0:  return {UC_ARCH_ARM, UC_MODE_ARM, 4};
        case 1:  return {UC_ARCH_ARM, UC_MODE_THUMB, 4};
        case 3:  return {UC_ARCH_X86, UC_MODE_32, 4};
        case 4:  return {UC_ARCH_X86, UC_MODE_64, 8};
        default: return {UC_ARCH_ARM64, UC_MODE_64, 8};
    }
}

// Get register ID by name untuk ARM64
static int getArm64RegId(const std::string& name) {
    if (name == "pc") return UC_ARM64_REG_PC;
    if (name == "sp") return UC_ARM64_REG_SP;
    if (name.size() == 2 && name[0] == 'x') {
        int idx = name[1] - '0';
        if (idx >= 0 && idx <= 9) return idx;
    }
    if (name.size() == 3 && name[0] == 'x') {
        int idx = (name[1] - '0') * 10 + (name[2] - '0');
        if (idx >= 10 && idx <= 30) return idx;
    }
    return -1;
}

// Get all ARM64 general purpose register IDs
static std::vector<std::pair<std::string, int>> getAllArm64Regs() {
    return {
        {"x0", UC_ARM64_REG_X0}, {"x1", UC_ARM64_REG_X1},
        {"x2", UC_ARM64_REG_X2}, {"x3", UC_ARM64_REG_X3},
        {"x4", UC_ARM64_REG_X4}, {"x5", UC_ARM64_REG_X5},
        {"x6", UC_ARM64_REG_X6}, {"x7", UC_ARM64_REG_X7},
        {"x8", UC_ARM64_REG_X8}, {"x9", UC_ARM64_REG_X9},
        {"x10", UC_ARM64_REG_X10}, {"x11", UC_ARM64_REG_X11},
        {"x12", UC_ARM64_REG_X12}, {"x13", UC_ARM64_REG_X13},
        {"x14", UC_ARM64_REG_X14}, {"x15", UC_ARM64_REG_X15},
        {"x16", UC_ARM64_REG_X16}, {"x17", UC_ARM64_REG_X17},
        {"x18", UC_ARM64_REG_X18}, {"x19", UC_ARM64_REG_X19},
        {"x20", UC_ARM64_REG_X20}, {"x21", UC_ARM64_REG_X21},
        {"x22", UC_ARM64_REG_X22}, {"x23", UC_ARM64_REG_X23},
        {"x24", UC_ARM64_REG_X24}, {"x25", UC_ARM64_REG_X25},
        {"x26", UC_ARM64_REG_X26}, {"x27", UC_ARM64_REG_X27},
        {"x28", UC_ARM64_REG_X28}, {"x29", UC_ARM64_REG_X29},
        {"x30", UC_ARM64_REG_X30}, {"sp", UC_ARM64_REG_SP},
        {"pc", UC_ARM64_REG_PC}
    };
}

// ── Hook callback context ──────────────────────────────────────

struct HookContext {
    UnicornEngine::Impl* impl;
    UnicornConfig config;
    std::vector<TraceEntry>* trace;
    size_t instructionIndex;
    bool terminated;
    std::string terminationReason;
};

// ── UnicornEngine::Impl ────────────────────────────────────────

struct UnicornEngine::Impl {
    uc_engine* uc = nullptr;
    int arch = 0;  // DisassemblerArch value
    ArchMapping archMapping;
    bool initialized = false;

    // Hook handles
    void* codeHook = nullptr;
    void* memReadHook = nullptr;
    void* memWriteHook = nullptr;
    void* memUnmappedHook = nullptr;

    ~Impl() {
        shutdown();
    }

    void shutdown() {
        if (uc) {
            uc_close(uc);
            uc = nullptr;
        }
        initialized = false;
    }

    // Snap semua register ke trace entry
    void snapshotRegisters(TraceEntry& entry) {
        if (!uc) return;

        auto regs = getAllArm64Regs();
        for (const auto& [name, regId] : regs) {
            uint64_t val = 0;
            //ignore return value — best effort snapshot
            uc_reg_read(uc, regId, &val);
            entry.registers[name] = val;
        }
    }
};

// ── Hook callbacks ─────────────────────────────────────────────

// Code hook: dipanggil SETIAP instruksi yang dieksekusi
static void codeHookCallback(uc_engine* uc, uint64_t address,
                             uint32_t size, void* userData) {
    auto* ctx = static_cast<HookContext*>(userData);
    if (ctx->terminated) return;

    // Cari instruksi yang cocok dengan address ini
    TraceEntry entry;
    entry.address = address;
    entry.bytes.resize(size);

    // Baca bytes instruksi dari Unicorn memory
    uc_mem_read(uc, address, entry.bytes.data(), size);

    // Snapshot register state
    ctx->impl->snapshotRegisters(entry);

    ctx->trace->push_back(std::move(entry));
    ctx->instructionIndex++;

    // Cek max instruction limit
    if (ctx->config.maxInstructions > 0 &&
        ctx->instructionIndex >= ctx->config.maxInstructions) {
        ctx->terminated = true;
        ctx->terminationReason = "max_instructions_reached";
    }
}

// Memory read hook
static void memReadHookCallback(uc_engine* uc, uc_mem_type type,
                                uint64_t address, int size, int64_t value,
                                void* userData) {
    auto* ctx = static_cast<HookContext*>(userData);
    if (ctx->terminated || ctx->trace->empty()) return;

    TraceEntry::MemoryAccess access;
    access.address = address;
    access.size = size;
    access.isWrite = false;
    access.value.resize(size);
    // Baca value dari memory
    uc_mem_read(uc, address, access.value.data(), size);

    ctx->trace->back().memoryAccesses.push_back(std::move(access));
}

// Memory write hook
static void memWriteHookCallback(uc_engine* uc, uc_mem_type type,
                                 uint64_t address, int size, int64_t value,
                                 void* userData) {
    auto* ctx = static_cast<HookContext*>(userData);
    if (ctx->terminated || ctx->trace->empty()) return;

    TraceEntry::MemoryAccess access;
    access.address = address;
    access.size = size;
    access.isWrite = true;
    access.value.resize(size);
    // Tulis value yang akan di-write
    std::memcpy(access.value.data(), &value, std::min(size, (int)sizeof(value)));

    ctx->trace->back().memoryAccesses.push_back(std::move(access));
}

// Invalid memory access hook
static bool invalidMemHookCallback(uc_engine* uc, uc_mem_type type,
                                   uint64_t address, int size, int64_t value,
                                   void* userData) {
    auto* ctx = static_cast<HookContext*>(userData);
    ctx->terminated = true;

    std::ostringstream oss;
    oss << "invalid_memory_access";
    switch (type) {
        case 0: oss << " at 0x" << std::hex << address; break;  // UC_MEM_READ_UNMAPPED
        case 1: oss << " at 0x" << std::hex << address; break;  // UC_MEM_WRITE_UNMAPPED
        default: oss << " type=" << type << " at 0x" << std::hex << address; break;
    }

    ctx->terminationReason = oss.str();
    return false;  // false = stop emulation
}

// ── UnicornEngine public API ───────────────────────────────────

UnicornEngine::UnicornEngine()
    : impl_(std::make_unique<Impl>()) {}

UnicornEngine::~UnicornEngine() = default;

UnicornEngine::UnicornEngine(UnicornEngine&&) noexcept = default;
UnicornEngine& UnicornEngine::operator=(UnicornEngine&&) noexcept = default;

bool UnicornEngine::initialize(int arch) {
    if (impl_->initialized) {
        impl_->shutdown();
    }

    impl_->arch = arch;
    impl_->archMapping = mapArch(arch);

    uc_err err = uc_open(impl_->archMapping.arch,
                         impl_->archMapping.mode,
                         &impl_->uc);
    if (err != UC_ERR_OK) {
        impl_->uc = nullptr;
        return false;
    }

    impl_->initialized = true;
    return true;
}

bool UnicornEngine::isInitialized() const {
    return impl_->initialized;
}

void UnicornEngine::shutdown() {
    impl_->shutdown();
}

EmulationResult UnicornEngine::execute(
    const std::vector<Instruction>& instructions,
    const UnicornConfig& config) {

    EmulationResult result;

    if (!impl_->initialized || !impl_->uc) {
        result.errorMessage = "Unicorn engine not initialized";
        return result;
    }

    if (instructions.empty()) {
        result.errorMessage = "empty instruction list";
        return result;
    }

    // Kumpulkan bytes dari semua instruksi
    // dan catat offset setiap instruksi untuk trace
    std::vector<uint8_t> allBytes;
    std::vector<size_t> instrOffsets;  // offset dalam allBytes untuk tiap instruksi
    std::vector<std::string> instrTexts;

    for (const auto& instr : instructions) {
        instrOffsets.push_back(allBytes.size());
        instrTexts.push_back(instr.mnemonic + " " + instr.opStr);
        allBytes.insert(allBytes.end(), instr.bytes.begin(), instr.bytes.end());
    }

    // Map code memory
    uint64_t codeBase = config.codeBaseAddress;
    size_t codeSize = std::max(allBytes.size(), config.codeMemorySize);
    // Align to page boundary
    codeSize = (codeSize + 0xFFF) & ~0xFFFULL;

    uc_err err = uc_mem_map(impl_->uc, codeBase, codeSize, UC_PROT_ALL);
    if (err != UC_ERR_OK) {
        result.errorMessage = "failed to map code memory: " + errorMessage(err);
        return result;
    }

    // Tulis code bytes ke memory
    err = uc_mem_write(impl_->uc, codeBase, allBytes.data(), allBytes.size());
    if (err != UC_ERR_OK) {
        result.errorMessage = "failed to write code: " + errorMessage(err);
        return result;
    }

    // Map stack memory
    uint64_t stackBase = codeBase + codeSize;
    size_t stackSize = config.stackSize;
    stackSize = (stackSize + 0xFFF) & ~0xFFFULL;

    err = uc_mem_map(impl_->uc, stackBase, stackSize, UC_PROT_ALL);
    if (err != UC_ERR_OK) {
        result.errorMessage = "failed to map stack: " + errorMessage(err);
        return result;
    }

    // Setup register awal
    // PC = code base (awal instruksi pertama)
    uint64_t pc = codeBase;
    uc_reg_write(impl_->uc, UC_ARM64_REG_PC, &pc);

    // SP = stack top
    uint64_t sp = stackBase + stackSize - 8;
    uc_reg_write(impl_->uc, UC_ARM64_REG_SP, &sp);

    // Setup hooks
    HookContext hookCtx;
    hookCtx.impl = impl_.get();
    hookCtx.config = config;
    hookCtx.trace = &result.trace;
    hookCtx.instructionIndex = 0;
    hookCtx.terminated = false;

    // Code hook (trace per-instruksi)
    err = uc_hook_add(impl_->uc, &impl_->codeHook,
                      UC_HOOK_CODE, (void*)codeHookCallback,
                      &hookCtx, codeBase, codeBase + allBytes.size());
    if (err != UC_ERR_OK) {
        result.errorMessage = "failed to add code hook: " + errorMessage(err);
        return result;
    }

    // Memory access hooks (opsional)
    if (config.hookMemoryAccess) {
        uc_hook_add(impl_->uc, &impl_->memReadHook,
                    UC_HOOK_MEM_READ, (void*)memReadHookCallback,
                    &hookCtx, 1, 0);

        uc_hook_add(impl_->uc, &impl_->memWriteHook,
                    UC_HOOK_MEM_WRITE, (void*)memWriteHookCallback,
                    &hookCtx, 1, 0);
    }

    // Invalid memory access hook
    if (config.hookInvalidAccess) {
        uc_hook_add(impl_->uc, &impl_->memUnmappedHook,
                    UC_HOOK_MEM_UNMAPPED, (void*)invalidMemHookCallback,
                    &hookCtx, 1, 0);
    }

    // Eksekusi!
    auto startTime = std::chrono::steady_clock::now();

    err = uc_emu_start(impl_->uc,
                       codeBase,           // start address
                       codeBase + allBytes.size(),  // until address (end of last instr)
                       config.timeoutUs,   // timeout
                       0);                 // count (0 = use max from hook)

    auto endTime = std::chrono::steady_clock::now();
    result.executionTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime).count();

    // Cleanup hooks
    if (impl_->codeHook) {
        uc_hook_del(impl_->uc, impl_->codeHook);
        impl_->codeHook = nullptr;
    }
    if (impl_->memReadHook) {
        uc_hook_del(impl_->uc, impl_->memReadHook);
        impl_->memReadHook = nullptr;
    }
    if (impl_->memWriteHook) {
        uc_hook_del(impl_->uc, impl_->memWriteHook);
        impl_->memWriteHook = nullptr;
    }
    if (impl_->memUnmappedHook) {
        uc_hook_del(impl_->uc, impl_->memUnmappedHook);
        impl_->memUnmappedHook = nullptr;
    }

    // Isi instruction text dari instruksi asli
    for (size_t i = 0; i < result.trace.size() && i < instrTexts.size(); ++i) {
        result.trace[i].instructionText = instrTexts[i];
    }

    // Ambil register akhir
    auto regs = getAllArm64Regs();
    for (const auto& [name, regId] : regs) {
        uint64_t val = 0;
        uc_reg_read(impl_->uc, regId, &val);
        result.finalRegisters[name] = val;
    }

    result.instructionsExecuted = result.trace.size();
    result.success = true;

    if (hookCtx.terminated) {
        result.terminated = true;
        result.terminationReason = hookCtx.terminationReason;
    }

    if (err != UC_ERR_OK && err != UC_ERR_OK) {
        // Eksekusi berhenti karena error (bukan normal termination)
        if (hookCtx.terminationReason.empty()) {
            result.terminationReason = "uc_emu_start: " + errorMessage(err);
        }
    }

    return result;
}

EmulationResult UnicornEngine::executeRaw(
    const std::vector<uint8_t>& code,
    uint64_t baseAddr,
    const UnicornConfig& config) {

    EmulationResult result;

    if (!impl_->initialized || !impl_->uc) {
        result.errorMessage = "Unicorn engine not initialized";
        return result;
    }

    if (code.empty()) {
        result.errorMessage = "empty code buffer";
        return result;
    }

    // Map code memory
    size_t codeSize = std::max(code.size(), config.codeMemorySize);
    codeSize = (codeSize + 0xFFF) & ~0xFFFULL;

    uc_err err = uc_mem_map(impl_->uc, baseAddr, codeSize, UC_PROT_ALL);
    if (err != UC_ERR_OK) {
        result.errorMessage = "failed to map code memory: " + errorMessage(err);
        return result;
    }

    // Tulis code bytes
    err = uc_mem_write(impl_->uc, baseAddr, code.data(), code.size());
    if (err != UC_ERR_OK) {
        result.errorMessage = "failed to write code: " + errorMessage(err);
        return result;
    }

    // Map stack
    uint64_t stackBase = baseAddr + codeSize;
    size_t stackSize = config.stackSize;
    stackSize = (stackSize + 0xFFF) & ~0xFFFULL;

    err = uc_mem_map(impl_->uc, stackBase, stackSize, UC_PROT_ALL);
    if (err != UC_ERR_OK) {
        result.errorMessage = "failed to map stack: " + errorMessage(err);
        return result;
    }

    // Setup registers
    uint64_t pc = baseAddr;
    uc_reg_write(impl_->uc, UC_ARM64_REG_PC, &pc);
    uint64_t sp = stackBase + stackSize - 8;
    uc_reg_write(impl_->uc, UC_ARM64_REG_SP, &sp);

    // Hook context
    HookContext hookCtx;
    hookCtx.impl = impl_.get();
    hookCtx.config = config;
    hookCtx.trace = &result.trace;
    hookCtx.instructionIndex = 0;
    hookCtx.terminated = false;

    // Code hook
    err = uc_hook_add(impl_->uc, &impl_->codeHook,
                      UC_HOOK_CODE, (void*)codeHookCallback,
                      &hookCtx, baseAddr, baseAddr + code.size());

    if (config.hookInvalidAccess) {
        uc_hook_add(impl_->uc, &impl_->memUnmappedHook,
                    UC_HOOK_MEM_UNMAPPED, (void*)invalidMemHookCallback,
                    &hookCtx, 1, 0);
    }

    // Eksekusi
    auto startTime = std::chrono::steady_clock::now();
    err = uc_emu_start(impl_->uc, baseAddr, baseAddr + code.size(),
                       config.timeoutUs, 0);
    auto endTime = std::chrono::steady_clock::now();
    result.executionTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime).count();

    // Cleanup hooks
    if (impl_->codeHook) {
        uc_hook_del(impl_->uc, impl_->codeHook);
        impl_->codeHook = nullptr;
    }
    if (impl_->memUnmappedHook) {
        uc_hook_del(impl_->uc, impl_->memUnmappedHook);
        impl_->memUnmappedHook = nullptr;
    }

    // Register akhir
    auto regs = getAllArm64Regs();
    for (const auto& [name, regId] : regs) {
        uint64_t val = 0;
        uc_reg_read(impl_->uc, regId, &val);
        result.finalRegisters[name] = val;
    }

    result.instructionsExecuted = result.trace.size();
    result.success = true;

    if (hookCtx.terminated) {
        result.terminated = true;
        result.terminationReason = hookCtx.terminationReason;
    }

    return result;
}

bool UnicornEngine::readRegister(const std::string& regName, uint64_t& value) const {
    if (!impl_->uc) return false;

    int regId = getArm64RegId(regName);
    if (regId < 0) return false;

    uc_err err = uc_reg_read(impl_->uc, regId, &value);
    return err == UC_ERR_OK;
}

bool UnicornEngine::writeRegister(const std::string& regName, uint64_t value) {
    if (!impl_->uc) return false;

    int regId = getArm64RegId(regName);
    if (regId < 0) return false;

    uc_err err = uc_reg_write(impl_->uc, regId, &value);
    return err == UC_ERR_OK;
}

bool UnicornEngine::readMemory(uint64_t address, size_t size,
                               std::vector<uint8_t>& out) const {
    if (!impl_->uc) return false;

    out.resize(size);
    uc_err err = uc_mem_read(impl_->uc, address, out.data(), size);
    return err == UC_ERR_OK;
}

bool UnicornEngine::writeMemory(uint64_t address, const std::vector<uint8_t>& data) {
    if (!impl_->uc) return false;

    uc_err err = uc_mem_write(impl_->uc, address, data.data(), data.size());
    return err == UC_ERR_OK;
}

std::string UnicornEngine::errorMessage(int errorCode) {
    return uc_strerror(static_cast<uc_err>(errorCode));
}

} // namespace omnibyte::hydradis
