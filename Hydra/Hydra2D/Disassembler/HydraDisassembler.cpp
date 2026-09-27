#include "Disassembler/HydraDisassembler.h"

#include "Disassembler/backends/CapstoneAdapter/CapstoneAdapter.h"
#include "Disassembler/backends/ProtobufAdapter/ProtobufAdapter.h"
#include "Disassembler/backends/WasmAdapter/WasmAdapter.h"

namespace omnibyte::hydradis {

std::unique_ptr<HydraDisassembler> createDisassembler(
    DisassemblerBackend backend,
    DisassemblerArch arch
) {
    switch (backend) {
        case DisassemblerBackend::Capstone:
            return createCapstoneDisassembler(arch);

        case DisassemblerBackend::Protobuf:
            return createProtobufDisassembler(arch);

        case DisassemblerBackend::WASM:
            return createWasmDisassembler(arch);
    }

    return nullptr;
}

} // namespace omnibyte::hydradis
