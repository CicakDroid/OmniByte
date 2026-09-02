#pragma once
// ── rz_analysis.h ────────────────────────────────────────────────────
// STUB HEADER for compile-checking only.
// Minimal types used by decompiler_rz_ghidra.cpp.

#include "rz_core.h"

#define RZ_ANALYSIS_FCN_TYPE_NULL 0

struct RzAnalysisFunction {
    ut64 addr;
    char* name;
};

#ifdef __cplusplus
extern "C" {
#endif

RzAnalysisFunction* rz_analysis_get_fcn_in(void* analysis, ut64 addr, int type);

#ifdef __cplusplus
}
#endif
