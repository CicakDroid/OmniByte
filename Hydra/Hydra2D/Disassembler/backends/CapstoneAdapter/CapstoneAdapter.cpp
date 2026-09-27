#include "Disassembler/backends/CapstoneAdapter/CapstoneAdapter.h"

namespace omnibyte::hydradis {
namespace {

// tiap DisassemblerArch harus ada di sini; -Wswitch kalau ada yang terlewat.
bool mapArch(DisassemblerArch arch, cs_arch& outArch, cs_mode& outMode) {
    switch (arch) {
        case DisassemblerArch::ARM:
            outArch = CS_ARCH_ARM;
            outMode = CS_MODE_ARM;
            return true;
        case DisassemblerArch::ARM_Thumb:
            outArch = CS_ARCH_ARM;
            outMode = CS_MODE_THUMB;
            return true;
        case DisassemblerArch::ARM64:
            outArch = CS_ARCH_ARM64;
            outMode = CS_MODE_ARM;
            return true;
        case DisassemblerArch::x86:
            outArch = CS_ARCH_X86;
            outMode = CS_MODE_32;
            return true;
        case DisassemblerArch::x86_64:
            outArch = CS_ARCH_X86;
            outMode = CS_MODE_64;
            return true;
        case DisassemblerArch::MIPS:
            outArch = CS_ARCH_MIPS;
            outMode = CS_MODE_MIPS32;
            return true;
        case DisassemblerArch::PPC:
            outArch = CS_ARCH_PPC;
            outMode = CS_MODE_32;
            return true;
        case DisassemblerArch::SPARC:
            outArch = CS_ARCH_SPARC;
            outMode = CS_MODE_V9;
            return true;
        case DisassemblerArch::SystemZ:
            outArch = CS_ARCH_SYSZ;
            outMode = CS_MODE_LITTLE_ENDIAN;
            return true;
        case DisassemblerArch::XCore:
            outArch = CS_ARCH_XCORE;
            outMode = CS_MODE_LITTLE_ENDIAN;
            return true;
        case DisassemblerArch::M68K:
            outArch = CS_ARCH_M68K;
            outMode = CS_MODE_BIG_ENDIAN;
            return true;
        case DisassemblerArch::TMS320C64X:
            outArch = CS_ARCH_TMS320C64X;
            outMode = CS_MODE_LITTLE_ENDIAN;
            return true;
        case DisassemblerArch::M680X:
            outArch = CS_ARCH_M680X;
            outMode = CS_MODE_LITTLE_ENDIAN;
            return true;
        case DisassemblerArch::EVM:
            outArch = CS_ARCH_EVM;
            outMode = CS_MODE_LITTLE_ENDIAN;
            return true;
        case DisassemblerArch::None:
        case DisassemblerArch::WASM:
            return false;
    }

    return false;
}

} // namespace

CapstoneAdapter::CapstoneAdapter(DisassemblerArch arch) : arch_(arch) {
    cs_arch csArch;
    cs_mode csMode;
    if (!mapArch(arch, csArch, csMode)) {
        return;  // arch tidak didukung Capstone -> valid_ tetap false
    }

    csh handle = 0;
    if (cs_open(csArch, csMode, &handle) != CS_ERR_OK) {
        return;
    }

    handle_ = handle;
    valid_ = true;
}

CapstoneAdapter::~CapstoneAdapter() {
    if (valid_) {
        cs_close(&handle_);
    }
}

DisassemblyResult CapstoneAdapter::disassemble(
    const uint8_t* code,
    size_t codeSize,
    uint64_t baseAddr,
    size_t count
) const {
    DisassemblyResult result;

    if (!valid_) {
        result.errorMessage = "Capstone handle not open for this arch";
        return result;
    }

    if (!code || codeSize == 0) {
        result.errorMessage = "Empty code buffer";
        return result;
    }

    cs_insn* insn = cs_malloc(handle_);
    if (!insn) {
        result.errorMessage = std::string("cs_malloc failed: ") + cs_strerror(cs_errno(handle_));
        return result;
    }

    // cs_disasm_iter() majukan p/remaining/addr sendiri tiap iterasi.
    const uint8_t* p = code;
    size_t remaining = codeSize;
    uint64_t addr = baseAddr;

    while (remaining > 0 && (count == 0 || result.instructions.size() < count)) {
        if (!cs_disasm_iter(handle_, &p, &remaining, &addr, insn)) {
            break;  // end-of-buffer / byte tidak bisa di-decode
        }

        Instruction instr;
        instr.address = insn->address;
        instr.size = insn->size;
        instr.mnemonic = insn->mnemonic;
        instr.opStr = insn->op_str;
        instr.bytes.assign(insn->bytes, insn->bytes + insn->size);

        result.totalBytes += insn->size;
        result.instructions.push_back(std::move(instr));
    }

    cs_free(insn, 1);

    if (result.instructions.empty()) {
        cs_err err = cs_errno(handle_);
        if (err != CS_ERR_OK) {
            result.errorMessage = std::string("cs_disasm_iter failed: ") + cs_strerror(err);
        } else {
            result.errorMessage = "No instructions decoded";
        }
        return result;
    }

    result.success = true;
    return result;
}

std::unique_ptr<HydraDisassembler> createCapstoneDisassembler(DisassemblerArch arch) {
    auto adapter = std::make_unique<CapstoneAdapter>(arch);
    if (!adapter->valid()) {
        return nullptr;
    }
    return adapter;
}

} // namespace omnibyte::hydradis
