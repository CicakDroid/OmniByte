#pragma once
// ── rz_ghidra.h ──────────────────────────────────────────────────────
// STUB HEADER for compile-checking only.
// Public API from rz-ghidra: rz_ghidra_decompile_annotated_code()

#include "rz_core.h"

struct RzAnnotatedCode {
    char* code;
    char* error;
};

#ifdef __cplusplus
extern "C" {
#endif

RzAnnotatedCode* rz_ghidra_decompile_annotated_code(RzCore* core, ut64 addr);
void rz_ghidra_free(RzAnnotatedCode* code);

#ifdef __cplusplus
}
#endif
