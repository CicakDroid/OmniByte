#pragma once
// ── rz_core.h ────────────────────────────────────────────────────────
// STUB HEADER for compile-checking only.
// Based on rizin API: https://rizin.re/
// Types and functions used by decompiler_rz_ghidra.cpp and decompiler_rizin_native.cpp.

#include <cstdint>
#include <cstddef>

typedef uint64_t ut64;
typedef uint32_t ut32;
typedef uint8_t ut8;

#define RZ_PERM_RX 5
#define RZ_PERM_R 4
#define RZ_PERM_RWX 7

struct rz_io_t;

struct RzCoreIO {
    int dummy;
};

struct RzCore {
    RzCoreIO* io;
    void* config;
    void* analysis;
    void* bin;
    void* rcmd;
    void* block;
    size_t blocksize;
    ut64 offset;
};

struct RzCoreFile {
    int fd;
};

// Core API functions
#ifdef __cplusplus
extern "C" {
#endif

RzCore* rz_core_new(void);
void rz_core_free(RzCore* core);

char* rz_str_newf(const char* fmt, ...);

RzCoreFile* rz_core_file_open(RzCore* core, const char* path, int perms, ut64 mapaddr);
void rz_core_file_free(RzCoreFile* file);

void rz_io_write_at(RzCoreIO* io, ut64 addr, const ut8* buf, size_t len);
void rz_core_block_read(RzCore* core);
void rz_core_seek(RzCore* core, ut64 addr, int rizin);

char* rz_core_cmd_str(RzCore* core, const char* cmd);
char* rz_core_cmdf(RzCore* core, const char* fmt, ...);

#ifdef __cplusplus
}
#endif
