// STUB HEADER -- untuk g++ -fsyntax-only lokal SAJA.
//  BUKAN library asli. Build production pakai header dari
//  FetchContent (lihat CMakeLists.txt backend terkait).
//  Signature di sini mengikuti dokumentasi resmi versi 0.7 --
//  jangan percaya buta, verifikasi ulang saat library asli di-install.
//
// Sources:
//   - rizinorg/rizin: librz/include/rz_core.h
//   - https://github.com/rizinorg/rizin/blob/dev/librz/include/rz_core.h

#ifndef RZ_CORE_STUB_H
#define RZ_CORE_STUB_H

#include <cstdint>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

// ── Constants ───────────────────────────────────────────────────
// Source: rizinorg/rizin/librz/include/rz_types.h
#define RZ_PERM_RX (RZ_PERM_READ | RZ_PERM_EXEC)
#define RZ_PERM_READ  0x04
#define RZ_PERM_EXEC  0x01
#define RZ_PERM_WRITE 0x02

// ── Forward declarations ────────────────────────────────────────
typedef struct rz_core_t RzCore;
typedef struct rz_io_t RzIO;
typedef struct rz_core_file_t RzCoreFile;

// ── RzIO stub ───────────────────────────────────────────────────
// Source: rizinorg/rizin/librz/include/rz_io.h
// unverified signature, inferred from usage in decompiler_rz_ghidra.cpp
struct rz_io_t {
    int dummy; // placeholder
};

// ── RzCoreFile stub ─────────────────────────────────────────────
// Source: rizinorg/rizin/librz/include/rz_core.h
// unverified signature, inferred from usage in decompiler_rz_ghidra.cpp
struct rz_core_file_t {
    int dummy; // placeholder
};

// ── RzCore stub ─────────────────────────────────────────────────
// Source: rizinorg/rizin/librz/include/rz_core.h
// unverified signature, inferred from usage in decompiler_rz_ghidra.cpp
struct rz_core_t {
    RzIO *io;
    // other members omitted for stub
};

// ── Function prototypes ─────────────────────────────────────────

/// Create a new RzCore instance.
/// Source: rizinorg/rizin/librz/include/rz_core.h
RzCore *rz_core_new(void);

/// Free an RzCore instance.
/// Source: rizinorg/rizin/librz/include/rz_core.h
void rz_core_free(RzCore *core);

/// Open a file in RzCore.
/// Source: rizinorg/rizin/librz/include/rz_core.h
// unverified signature, inferred from usage in decompiler_rz_ghidra.cpp
RzCoreFile *rz_core_file_open(RzCore *core, const char *filename, int flags, int perm);

/// Write data at an address in the IO layer.
/// Source: rizinorg/rizin/librz/include/rz_io.h
// unverified signature, inferred from usage in decompiler_rz_ghidra.cpp
int rz_io_write_at(RzIO *io, uint64_t addr, const uint8_t *data, int len);

/// Read bytes from the current block.
/// Source: rizinorg/rizin/librz/include/rz_core.h
/// unverified signature — .cpp calls with zero extra args: rz_core_block_read(core_)
int rz_core_block_read(RzCore *core);

/// Seek to an address.
/// Source: rizinorg/rizin/librz/include/rz_core.h
// unverified signature, inferred from usage in decompiler_rz_ghidra.cpp
int rz_core_seek(RzCore *core, uint64_t addr, int rb);

/// Execute a command and return the result as a string.
/// Source: rizinorg/rizin/librz/include/rz_core.h
// unverified signature, inferred from usage in decompiler_rz_ghidra.cpp
char *rz_core_cmd_str(RzCore *core, const char *cmd);

/// Create a formatted string.
/// Source: rizinorg/rizin/librz/include/rz_util/rz_str.h
// unverified signature, inferred from usage in decompiler_rz_ghidra.cpp
char *rz_str_newf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // RZ_CORE_STUB_H
