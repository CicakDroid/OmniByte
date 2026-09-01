#include "Factory/DisassemblerFactory.h"
#include <stdexcept>

// Forward declarations — each backend has a free function in its .cpp file.
// We include them here instead of headers to avoid pulling in capstone/rizin deps.
namespace omnibyte::hydradis {
    std::unique_ptr<IDisassembler> createCapstoneDisassembler(DisassemblerArch arch);
}

namespace omnibyte::hydradis {

std::unique_ptr<IDisassembler> DisassemblerFactory::create(
    DisassemblerArch arch,
    DisassemblerBackend backend
) {
    switch (backend) {
        case DisassemblerBackend::Capstone:
            return createCapstoneDisassembler(arch);

        case DisassemblerBackend::Rizin:
            // Rizin disassembler belum diimplement (subprocess-based).
            // Fallback ke capstone untuk sekarang.
            return createCapstoneDisassembler(arch);

        default:
            return nullptr;
    }
}

std::unique_ptr<IDisassembler> DisassemblerFactory::createFromMachine(
    uint16_t machine,
    bool is64Bit,
    DisassemblerBackend backend
) {
    // ELF e_machine values — sesuai ELF spec
    // EM_ARM = 40, EM_AARCH64 = 183, EM_386 = 3, EM_X86_64 = 62,
    // EM_MIPS = 8, EM_PPC = 20, EM_SPARC = 2, EM_S390 = 22
    DisassemblerArch arch;

    switch (machine) {
        case 40:    // EM_ARM
            arch = DisassemblerArch::ARM;
            break;
        case 183:   // EM_AARCH64
            arch = DisassemblerArch::ARM64;
            break;
        case 3:     // EM_386
            arch = DisassemblerArch::x86;
            break;
        case 62:    // EM_X86_64
            arch = DisassemblerArch::x86_64;
            break;
        case 8:     // EM_MIPS
            arch = DisassemblerArch::MIPS;
            break;
        case 20:    // EM_PPC
            arch = DisassemblerArch::PPC;
            break;
        case 2:     // EM_SPARC
            arch = DisassemblerArch::SPARC;
            break;
        case 22:    // EM_S390
            arch = DisassemblerArch::SystemZ;
            break;
        default:
            return nullptr;  // machine type tidak dikenal
    }

    return create(arch, backend);
}

} // namespace omnibyte::hydradis
