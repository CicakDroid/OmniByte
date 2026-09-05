# Laporan Penelitian: Konsep Inti Reverse Engineering untuk Pengembangan OmniByte

**Nama Proyek:** Pengembangan OmniByte
**Tanggal:** 2026-09-05 (Updated)
**Status:** Final
**Revisi:** 3.0 - Penambahan Anti-Tampering dan Magic Number (0xFAB11BAF)

---

## Daftar Isi

1. [Synthesizer](#1-synthesizer)
2. [Taint Analysis](#2-taint-analysis)
3. [Expression Synthesis](#3-expression-synthesis)
4. [Memory Segmentation](#4-memory-segmentation)
5. [Coverage Strategies](#5-coverage-strategies)
6. [Pointer Coverage](#6-pointer-coverage)
7. [Sanitizer Mechanism](#7-sanitizer-mechanism)
8. [Basic Heap Allocator](#8-basic-heap-allocator)
9. [Metadata (IL2CPP/globalmetadata.dat)](#9-metadata-il2cppglobalmetadatadat)
10. [Recursive Binary Unpacker](#10-recursive-binary-unpacker)
11. [Section Carver](#11-section-carver)
12. [Entropy](#12-entropy)
13. [IR (Intermediate Representation)](#13-ir-intermediate-representation)
14. [Stacktrace](#14-stacktrace)
15. [Patching](#15-patching)
16. [Compiler](#16-compiler)
17. [Packer](#17-packer)
18. [Root Checker](#18-root-checker)
19. [Anti Debug](#19-anti-debug)
20. [Anti Cheat](#20-anti-cheat)
21. [Play Integrity Check](#21-play-integrity-check)
22. [SSL Unpinning](#22-ssl-unpinning)
23. [Signature](#23-signature)
24. [Deobfuscation](#24-deobfuscation)
25. [Hooking](#25-hooking)
26. [Anti-Tampering](#26-anti-tampering)
27. [Valid 0xFAB11BAF Magic Number](#27-valid-0xfab11baf-magic-number)
28. [Kesimpulan](#28-kesimpulan)

---

## 1. Synthesizer

### Definisi
Synthesizer adalah komponen yang menghasilkan input baru berdasarkan pemahaman struktur program. Berbeda dengan input random, synthesizer menghasilkan input yang terstruktur dan valid sesuai expectation program.

### Tipe Synthesizer

#### Grammar-Based Synthesizer
Menggunakan grammar (BNF/EBNF) dari format input untuk menghasilkan input valid yang mengikuti struktur program.

```
<response> ::= <header> <body> <footer>
<header> ::= <magic> <version>
<body> ::= <field>+
<field> ::= <name> ":" <value>
```

#### Byte-Level Synthesizer
Menghasilkan byte sequence berdasarkan constraint yang ditemukan di binary, seperti magic bytes, checksum, atau length field.

#### Mutational Synthesizer
Mengambil input yang sudah ada lalu melakukan mutation pada bagian-bagian tertentu secara terstruktur.

### Contoh Penggunaan
Jika program mengharapkan input berformat `{"cmd": "<payload>", "len": <N>}`, synthesizer mengetahui bahwa field `len` harus sama dengan panjang payload, bukan angka random.

---

## 2. Taint Analysis

### Definisi
Taint analysis adalah teknik melacak alur data dari source ke sink. Setiap byte yang berasal dari input attacker ditandai sebagai "tainted" dan dilacak propagasinya melalui program.

### Source (Titik Masuk Data Attacker)
- File read operation
- Network receive (recv, read)
- Standard input (stdin, readline)

### Sink (Titik Kritis)
- Pointer dereference
- Array index access
- Format string argument
- System call argument

### Contoh Propagasi
```c
// Source: buf ditandai tainted
recv(fd, buf, len, 0);

// Propagasi: tainted menyebar
memcpy(dst, buf, len);

// Sink: dst yang tainted digunakan sebagai pointer
*(int*)dst = value;  // ← VULNERABILITY
```

### Kegunaan dalam Reverse Engineering
- Menemukan input yang mempengaruhi control flow untuk fuzzing target
- Menemukan vulnerability (tainted data digunakan sebagai pointer = buffer overflow)
- Memahami bagaimana program memproses data dari attacker

---

## 3. Expression Synthesis

### Definisi
Expression synthesis adalah pembangunan ekspresi representatif dari kode. Alih-alih menganalisis assembly secara langsung, teknik ini menyintesis ekspresi algebra yang setara.

### Contoh Transformasi

**Assembly:**
```asm
mov eax, [rbp-0x10]
imul eax, 2
add eax, 5
mov [rbp-0x14], eax
```

**Expression Synthesis:**
```
v1 = mem[rbp-0x10]
v2 = v1 × 2
v3 = v2 + 5
mem[rbp-0x14] = v3
```

### Kegunaan
- **Simplifikasi**: Mengurangi kompleksitas kode untuk analisis
- **Pattern Matching**: Menemukan pola yang sama di binary berbeda
- **Equivalence Check**: Memverifikasi apakah dua fungsi memiliki behavior yang sama
- **Deobfuscation**: Mereverse efek obfuscation seperti opaque predicate

---

## 4. Memory Segmentation

### Definisi
Memory segmentation adalah analisis layout memori program untuk memahami bagaimana berbagai jenis data ditempatkan dan diakses.

### Tipe Segment

| Segment | Isi | Karakteristik |
|---------|-----|---------------|
| `.text` | Code | Execute-only, berisi instruction |
| `.rodata` | Read-only data | String constant, lookup table |
| `.data` | Initialized global/static | Global variable dengan nilai awal |
| `.bss` | Uninitialized global/static | Global variable tanpa nilai awal |
| Heap | Dynamic allocation | malloc/new, tumbuh ke atas |
| Stack | Local variables, return address | Function frame, tumbuh ke bawah |
| Mapped | mmap'd regions | Shared libraries, memory-mapped files |

### Kegunaan dalam Reverse Engineering
- Membedakan mana data dan mana code (anti-disassembly)
- Menemukan vtable, RTTI, exception handler
- Menganalisis memory corruption seperti heap overflow dan stack overflow
- Memahami mekanisme pertahanan seperti ASLR, DEP/NX, stack canary

---

## 5. Coverage Strategies

### Definisi
Coverage strategies digunakan dalam fuzzing untuk mengukur seberapa "jauh" input telah mengeksplorasi program.

### Block Coverage (Basic Block Coverage)
Menghitung berapa banyak basic block yang pernah dieksekusi.

```
Total blocks: 100
Blocks hit: 3
Coverage: 3%
```

**Kelebihan**: Paling sederhana dan cepat
**Kekurangan**: Tidak membedakan path yang berbeda melewati block yang sama

### Edge Coverage (Branch Coverage)
Menghitung transisi antar block, bukan hanya block individu.

```c
if (condition) {
    A();  // edge: entry → A
} else {
    B();  // edge: entry → B
}
```

Edge `entry → A` dan `entry → B` dihitung terpisah meskipun keduanya melewati block `entry`.

**Kelebihan**: Lebih granular dari block coverage
**Kekurangan**: Masih bisa kehilangan path yang kompleks

### Path Coverage
Mencoba mengeksplorasi semua kemungkinan path dari entry point ke exit point.

**Kelebihan**: Paling komprehensif
**Kekurangan**: Path explosion - jumlah path tumbuh secara kombinatorial, sering tidak mungkin dicover sepenuhnya

### Implementasi dalam Fuzzing Modern
Fuzzing tools seperti AFL, libFuzzer, dan Honggfuzz umumnya menggunakan edge coverage karena memberikan balance antara granularity dan performance.

---

## 6. Pointer Coverage

### Definisi
Pointer coverage adalah extension dari coverage analysis yang berfokus pada values dari pointer yang dihasilkan program.

### Kegunaan
- **Use-After-Free Detection**: Pointer masih ada tetapi target sudah di-free
- **Double-Free Detection**: Pointer yang sama di-free dua kali
- **Buffer Overflow via Pointer Arithmetic**: Pointer yang di-offset melewati batas buffer
- **Dangling Pointer**: Pointer ke stack frame yang sudah return

### Contoh Vulnerability Pattern
```c
char* ptr = malloc(100);
free(ptr);
// ... alokasi baru ...
ptr[0] = 'A';  // ← Use-After-Free
```

---

## 7. Sanitizer Mechanism

### Definisi
Sanitizer adalah runtime check yang ditambahkan ke program untuk mendeteksi vulnerability saat execution time.

### Tipe Sanitizer

| Sanitizer | Fungsi Deteksi |
|-----------|----------------|
| **ASan** (AddressSanitizer) | Buffer overflow, use-after-free, double-free, memory leak |
| **MSan** (MemorySanitizer) | Uninitialized memory read |
| **TSan** (ThreadSanitizer) | Data race, deadlocks |
| **UBSan** (UndefinedBehaviorSanitizer) | Integer overflow, null deref, shift overflow |
| **HWASan** (Hardware ASan) | Tag-based memory safety (ARM, x86) |
| **CFSan** (ControlFlowSanitizer) | Indirect call target validation (CFI) |

### Cara Kerja ASan
ASan menggunakan shadow memory untuk melacak status setiap byte di program:

```asm
; Sebelum ASan:
mov rax, [rbp-0x10]  ; normal access

; Sesudah ASan:
call __asan_load8     ; check shadow memory terlebih dahulu
; shadow memory: 0 = valid, >0 = poisoned (invalid)
mov rax, [rbp-0x10]  ; actual access
```

**Shadow Memory Layout:**
- Setiap 8 byte program → 1 byte shadow
- Shadow = 0: semua 8 byte valid
- Shadow = 1-7: N byte pertama valid, sisanya invalid
- Shadow > 7: seluruh region invalid (redzone, freed, dll)

### Kegunaan dalam Reverse Engineering
- Reverse engineer ASan build untuk menemukan vulnerability tanpa source code
- Analisis shadow memory layout untuk memahami bagaimana program mendeteksi corruption

---

## 8. Basic Heap Allocator

### Definisi
Basic heap allocator adalah implementasi memory allocator yang perlu dipahami untuk analisis exploitation di level heap.

### Struktur Data

```c
struct chunk_header {
    size_t size;           // Ukuran chunk (dengan flags di bit bawah)
    chunk_header* fd;      // Forward pointer (free list)
    chunk_header* bk;      // Backward pointer (free list)
    // ... debug info, canary, dll
};
```

### Operasi Malloc

1. Cari free chunk yang cukup besar (first-fit, best-fit, dll)
2. Split chunk jika terlalu besar → sisa menjadi free chunk baru
3. Update free list
4. Return pointer ke data area (setelah header)

### Operasi Free

1. Cek canary/integrity (jika ada)
2. Masukkan chunk ke free list
3. Coalesce adjacent free chunks (merge)
4. Update size

### Contoh Vulnerability Pattern

```c
// Heap overflow
char* a = malloc(16);  // header: [size=17] [fd] [bk] + data[16]
char* b = malloc(16);  // header: [size=17] [fd] [bk] + data[16]

// a overflow ke header b → corrupt size, fd, bk
// → free(b) → corrupted free list → arbitrary write → RCE
```

### Tcache (glibc 2.26+)
- Per-thread cache untuk alokasi kecil (< 0x410)
- Single linked list (hanya forward pointer)
- Lebih mudah di-exploit karena tidak ada double-free detection di tcache

### Kegunaan dalam Reverse Engineering
- Mengenali tcache vs fastbin vs unsorted bin
- Identifikasi use-after-free → heap spray → code execution
- Analisis teknik exploitation seperti tcache poisoning, house of force, house of spirit

---

## 9. Metadata (IL2CPP/globalmetadata.dat)

### Definisi
`globalmetadata.dat` adalah file metadata yang dihasilkan oleh Unity IL2CPP (Intermediate Language to C++) build pipeline. File ini berisi semua informasi string, type definitions, method names, dan field names yang dibutuhkan oleh IL2CPP runtime.

### Struktur Internal

```
globalmetadata.dat
├── String Literals        → semua string dalam program
├── Type Definitions       → class/interface/struct/enum definitions
├── Method Definitions     → method signatures, parameters, return types
├── Field Definitions      → field names, types, offsets
├── Parameter Definitions  → parameter names dan types
├── Generic Containers     → generic type definitions (List<T>, Dictionary<K,V>)
├── Image Definitions      → assembly references (Assembly-CSharp.dll, mscorlib.dll)
└── Usage Hints            → optimization metadata
```

### Kenapa Penting untuk Reverse Engineering?

Ketika Unity game di-compile dengan IL2CPP:

1. C# code → C++ code (IL2CPP transpilation)
2. C++ → Native binary (.so library)
3. Metadata disimpan terpisah di `globalmetadata.dat`

Tanpa metadata ini, binary native hanya berupa raw memory operations — tidak ada nama class, method, atau field. Metadata memberikan symbolic information yang memungkinkan analisis.

### Hubungan dengan DEX/APK

| Komponen | Isi | Fungsi |
|----------|-----|--------|
| `classes.dex` | Compiled Java/Kotlin bytecode | Code execution (traditional Android) |
| `lib/arm64-v8a/libil2cpp.so` | Native C++ compiled code | IL2CPP runtime execution |
| `assets/globalmetadata.dat` | Type/method/field metadata | Symbolic info untuk il2cpp runtime |
| `assets/bin/Data/Managed/Metadata/global-metadata.dat` | Sama, path berbeda | Unity 2020+ layout |

### Cara Kerja IL2CPP Runtime

```
┌─────────────────────────────────────────────┐
│  libil2cpp.so (native code)                │
│  - Compiled C++ functions                  │
│  - No symbol names (stripped)              │
└──────────────┬──────────────────────────────┘
               │ mmap/load
               ▼
┌─────────────────────────────────────────────┐
│  globalmetadata.dat (mmap'd at runtime)    │
│  - String pool: "Update", "Start", etc.    │
│  - Type definitions: MonoBehaviour         │
│  - Method table: Update() → 0x12345        │
└─────────────────────────────────────────────┘
```

### Tools untuk Analisis Metadata

| Tool | Fungsi |
|------|--------|
| **Il2CppDumper** | Ekstrak metadata → C# stubs + IDA/Ghidra headers |
| **il2cpp-inspector** | UI untuk analisis metadata |
| **Cpp2IL** | Reverse engineer metadata ke source |
| **GameGuardian** | Runtime memory patching (baca metadata via memory) |

### Contoh: Il2CppDumper Output

```cpp
// Il2CppDumper menghasilkan:
struct MonoBehaviour_StaticFields {
    int32_t ___instanceID;  // offset 0x10
    String_t* ___name;      // offset 0x18
};

struct MonoBehaviour_Methods {
    void (*Update)(MonoBehaviour_t*, float);  // 0x12345678
    void (*Start)(MonoBehaviour_t*);          // 0x12345690
};
```

### Metadata Versions

| Unity Version | Metadata Format | Lokasi |
|---------------|-----------------|--------|
| Unity 5.x | `global-metadata.dat` (v14) | `assets/bin/Data/` |
| Unity 2017-2019 | `global-metadata.dat` (v19-24) | `assets/bin/Data/Managed/Metadata/` |
| Unity 2020+ | `global-metadata.dat` (v27-29) | `assets/bin/Data/Managed/Metadata/` |
| Unity 2022+ | `global-metadata.dat` (v29) | `assets/bin/Data/Managed/Metadata/` |

### Security Implications

1. **Anti-tamper**: Beberapa game encrypt `globalmetadata.dat` → butuh decrypt sebelum dump
2. **String obfuscation**: String di metadata bisa di-obfuscate → perlu deobfuscation
3. **Metadata validation**: Runtime check integrity → patching metadata = crash
4. **Code stripping**: Unused types/methods di-strip → metadata lebih kecil

### Workflow Reverse Engineering

```
1. Extract APK
2. Find globalmetadata.dat
3. Run Il2CppDumper → dapat C# stubs
4. Analisis libil2cpp.so di IDA/Ghidra
5. Cross-reference metadata dengan native code
6. Patch/modify sesuai kebutuhan
```

### Metadata dalam Game Engine Lain

Setiap engine memiliki mekanisme metadata sendiri, bukan `globalmetadata.dat` khusus Unity IL2CPP:

| Engine | Metadata System | Lokasi/File |
|--------|-----------------|-------------|
| **Unity IL2CPP** | `global-metadata.dat` | `assets/bin/Data/Managed/Metadata/` |
| **Unity Mono** | `global-metadata.dat` (format berbeda) | `assets/bin/Data/Managed/Metadata/` + DLL files |
| **Unreal Engine** | GNames + GObjects tables | `.pak` files + binary sections (`.usmap` untuk serialization) |
| **Godot** | `.godot/` directory + `.tres`/`.tscn` resources | `res://.godot/` + binary `.scn`/`.res` |
| **GameMaker** | YY/JSON resources | `data.win` (compiled) + `.yy` metadata files |
| **Cocos2d** | `project.json` + `jsb_*.json` | `src/` (scripts) + `res/` (resources) |
| **Source 2** | VPK + VTEX/DMX/VMAT | `.vpk` paket files |

#### Unreal Engine
- **GNames**: String table berisi semua nama (class, method, property)
- **GObjects**: Array UObject instances
- **`.pak`**: Package file berisi assets + index
- **`.usmap`**: Serialized reflection data untuk modding
- Tidak ada satu file `metadata.dat` — data tersebar di binary + pak files

#### Godot
- **`.godot/`**: Internal directory berisi import database
- **`.tres`/`.tscn`**: Text-based resource/scene files
- **`.scn`/`.res`**: Binary resource files
- **`project.godot`**: Project settings (bukan runtime metadata)
- Godot 4.x: Binary serialization format berubah total dari 3.x

#### GameMaker
- **`data.win`**: Compiled game data (sprites, sounds, code, objects)
- **`.yy` files**: JSON metadata untuk setiap resource
- **`options.ini`**: Platform-specific settings
- GameMaker Studio 2: Format berbeda dari GMS 1.x

#### Cocos2d
- **`project.json`**: Engine configuration
- **`jsb_*.json`**: JavaScript binding metadata
- **`res/`**: Resources (tidak ada centralized metadata)
- Cocos Creator: `.fire`/`.scene` files + `cocos2d-x` engine

### Metadata Aplikasi Android Non-Game

Semua aplikasi Android memiliki metadata, bukan hanya game. Perbedaannya terletak pada jenis dan lokasi metadata:

| Metadata | Lokasi | Fungsi |
|----------|--------|--------|
| `AndroidManifest.xml` | Root APK | Permissions, activities, services, receivers |
| `resources.arsc` | Root APK | Compiled resources (strings, layouts, styles) |
| `classes.dex` | Root APK | Compiled Java/Kotlin bytecode |
| `res/` | Directory | Layout XML, drawable, values |
| `META-INF/` | Directory | Signing certificates, manifest |
| `lib/` | Directory | Native libraries (.so) |
| `assets/` | Directory | Raw assets (user-defined) |
| `BuildConfig.class` | In DEX | Build metadata (version, debug flag) |
| `R.class` | In DEX | Resource ID mappings |

Contoh struktur APK non-game:
```
myapp.apk
├── AndroidManifest.xml          → metadata aplikasi
├── classes.dex                  → compiled code
├── resources.arsc               → compiled resources
├── res/
│   ├── layout/activity_main.xml
│   ├── values/strings.xml
│   └── drawable/icon.png
├── META-INF/
│   ├── CERT.SF
│   ├── CERT.RSA
│   └── MANIFEST.MF
└── lib/
    └── arm64-v8a/
        └── libnative.so
```

### Jenis Metadata dalam Aplikasi Android

Secara garis besar ada 5 kategori utama metadata Android:

#### 1. Application Metadata
- `AndroidManifest.xml`: Nama paket, versi, permissions, components
- `BuildConfig.class`: Build type, version code, debug status
- Signature certificates

#### 2. Resource Metadata
- `resources.arsc`: Compiled resource table (string pool, layout references, drawable references)
- `res/` directory: XML layouts, drawables, values
- Resource IDs (R.java/R.class mappings)

#### 3. Code Metadata
- `classes.dex`: Compiled bytecode
- `lib/*.so`: Native libraries (JNI bindings)
- Method/field definitions, class hierarchy

#### 4. Asset Metadata (user-defined)
- `assets/` directory: Raw files
- Database files (SQLite)
- Configuration files (JSON, XML, SharedPreferences)
- Game-specific: `global-metadata.dat`, level data, save files

#### 5. Security Metadata
- `META-INF/`: Signing certificates, manifest hash
- Play Integrity API tokens
- SafetyNet/Play Integrity attestation data
- Encrypted preferences (if using AndroidX Security)

---

## 10. Recursive Binary Unpacker

### Definisi
Recursive binary unpacker adalah tool yang membongkar (unpack) binary yang telah dipacking secara rekursif — artinya ia terus membongkar lapisan demi lapisan sampai mendapatkan binary asli (original payload).

### Cara Kerja
```
Packed Binary (Lapisan 3)
  → Unpack → Packed Binary (Lapisan 2)
    → Unpack → Packed Binary (Lapisan 1)
      → Unpack → Original Binary (Unpacked)
```

### Kenapa Rekursif?
Banyak packer modern menggunakan multi-layer packing (UPX + Themida + custom). Unpacker biasa hanya menghapus 1 lapisan. Recursive unpacker mendeteksi apakah hasil unpack masih ter-packed, lalu loop sampai benar-benar unpacked.

### Contoh Implementasi
- **Unipacker** — framework multi-packer
- Custom script yang check entropy setiap stage (entropy tinggi = masih packed)

---

## 11. Section Carver

### Definisi
Section carver adalah tool yang memotong/mengekstrak section tertentu dari binary PE/ELF berdasarkan header metadata.

### Kenapa Dibutuhkan?
- Binary PE punya section: `.text` (code), `.rdata` (read-only data), `.data`, `.rsrc` (resources), `.reloc`
- Kadang kita hanya butuh `.text` untuk analisis kode, atau `.rsrc` untuk extract icon/string
- Section carver bisa extract section dari corrupted binary atau dari memory dump

### Contoh Section PE
```
Section Table:
  .text   → 0x1000  size: 0x5000  (executable code)
  .rdata  → 0x6000  size: 0x2000  (constants, strings)
  .data   → 0x8000  size: 0x1000  (global variables)
  .rsrc   → 0x9000  size: 0x3000  (resources)
```

---

## 12. Entropy

### Definisi
Entropy adalah ukuran ketidakpastian/randomitas dalam data. Dalam reverse engineering, entropy digunakan untuk mendeteksi apakah bagian binary telah di-enkripsi atau di-compress.

### Skala Shannon Entropy (0-8 byte)

| Entropy | Interpretasi |
|---------|--------------|
| 0.0 - 1.0 | Data kosong/null bytes |
| 1.0 - 4.0 | Data terstruktur (code, teks) |
| 4.0 - 6.0 | Data compress (zip, gzip) |
| 6.0 - 7.5 | Data terenkripsi (AES, XOR) |
| 7.5 - 8.0 | Full random (encryption/padding) |

### Contoh Implementasi
```python
import math
from collections import Counter

def shannon_entropy(data):
    if not data: return 0
    counter = Counter(data)
    length = len(data)
    return -sum((count/length) * math.log2(count/length) 
                for count in counter.values())

# Entropy > 7.0 → kemungkinan terenkripsi/packed
```

### Visualisasi
Banyak tools menampilkan entropy sebagai grafik warna (heatmap) di sepanjang binary — area merah = entropy tinggi (terenkripsi), area biru = entropy rendah (code biasa).

---

## 13. IR (Intermediate Representation)

### Definisi
Intermediate Representation (IR) adalah bahasa perantara yang dibuat oleh compiler/decompiler antara source code dan machine code. IR bersifat tool-agnostic — bisa diproses tanpa peduli bahasa sumber atau target arch.

### Posisi IR dalam Pipeline
```
Source Code → [Frontend] → IR → [Optimizer] → IR → [Backend] → Machine Code
                  ↑                                    ↓
           (C, Rust, Go)                     (x86, ARM, RISC-V)
```

### Jenis-jenis IR

| Jenis | Contoh | Karakteristik |
|-------|--------|---------------|
| SSA (Static Single Assignment) | LLVM IR, TurboFan | Setiap variabel hanya di-assign sekali |
| Three-Address Code | GCC GIMPLE | Operasi 3 operand |
| P-code | Ghidra | 4-bit operand + opcode |
| ESIL | Rizin | Stack-based, virtual machine |
| Bytecode | JVM, CLR | Target VM, bukan CPU fisik |

### Contoh LLVM IR
```llvm
define i32 @add(i32 %a, i32 %b) {
entry:
  %result = add i32 %a, %b
  ret i32 %result
}
```

### Mengapa IR Penting dalam RE?
- Decompiler (Ghidra, IDA) mengkonversi machine code → IR → pseudocode
- IR memungkinkan optimasi dan analisis tanpa peduli target arch
- Symbolic execution engine bekerja di level IR

---

## 14. Stacktrace

### Definisi
Stacktrace (stack backtrace) adalah rekaman urutan fungsi yang dipanggil sampai titik tertentu (error/crash/breakpoint). Menunjukkan alamat return address di setiap frame stack.

### Struktur Stack Frame (x86-64)
```
[Higher Address]
┌─────────────────┐
│  Local vars     │ ← RBP + offset
├─────────────────┤
│  Saved RBP      │ ← RBP (Frame Pointer)
├─────────────────┤
│  Return Address │ ← RBP + 8
├─────────────────┤
│  Arguments      │ ← RBP + 16
└─────────────────┘
[Lower Address]
```

### Contoh Stacktrace Crash
```
#0  0x00007f8a12345678 in crash_function () from libtarget.so
#1  0x00007f8a12345abc in caller_function () from libtarget.so
#2  0x00007f8a12345def in main () from target_app
#3  0x00007f8a00123456 in __libc_start_call_main () from libc.so.6
```

### Penggunaan dalam RE
- **Symbolic stack analysis:** Mencari vulnerability seperti buffer overflow (return address overwritten)
- **Anti-debug detection:** Memeriksa stack frame untuk mendeteksi hooking (return address berubah)
- **Crash analysis:** Melacak origin crash sampai ke source code

---

## 15. Patching

### Definisi
Patching adalah mengubah byte tertentu dalam binary untuk memodifikasi perilaku program tanpa compile ulang.

### Jenis-jenis Patching

#### a) Binary Patching (Static)
Mengubah file binary langsung:
```
Original:   E8 XX XX XX XX    (call function_A)
Patched:    E9 XX XX XX XX    (jmp function_B)
             90                (nop - if needed)
```

#### b) Memory Patching (Dynamic)
Mengubah memory process saat runtime:
```c
DWORD oldProtect;
VirtualProtect(addr, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
addr[0] = 0xE9;  // JMP opcode
*(DWORD*)(addr+1) = (DWORD)hookFunc - (DWORD)addr - 5;
VirtualProtect(addr, 5, oldProtect, &oldProtect);
```

#### c) Runtime Patching (Hot-patching)
- **detour** (MSVC): Menggunakan trampoline
- **PLT/GOT patching** (Linux): Mengubah function pointer di Procedure Linkage Table
- **VTable hooking**: Mengubah virtual function pointer

---

## 16. Compiler

### Definisi
Compiler adalah program yang menerjemahkan source code bahasa tingkat tinggi menjadi machine code (atau IR → machine code).

### Pipeline Compiler Modern
```
Source Code
    ↓ [Lexical Analysis - Tokenizer]
Tokens
    ↓ [Parsing - Syntax Analysis]
AST (Abstract Syntax Tree)
    ↓ [Semantic Analysis]
Typed AST
    ↓ [IR Generation]
IR (LLVM IR, GCC Gimple)
    ↓ [Optimization Passes]
Optimized IR
    ↓ [Code Generation]
Machine Code / Assembly
    ↓ [Assembly + Linking]
Executable (PE/ELF)
```

### Pentingnya Compiler dalam RE
- **Decompiler** bekerja terbalik (reverse): Machine Code → IR → Pseudocode
- Memahami compiler optimization membantu memahami generated code
- **Compiler identification:** Identifikasi compiler dari binary signature (MSVC, GCC, Clang, LLVM)
- **Optimization level:** `-O0` vs `-O2` vs `-O3` sangat mempengaruhi readable output

---

## 17. Packer

### Definisi
Packer adalah tool yang mengompresi/mentransformasi binary untuk melindungi dari analisis, reverse engineering, atau pembajakan.

### Jenis-jenis Packer

| Kategori | Contoh | Karakteristik |
|----------|--------|---------------|
| **Compressor** | UPX, ASPack | Compress untuk ukuran lebih kecil |
| **Protector** | Themida, VMProtect, Enigma | Enkripsi + anti-debug + anti-RE |
| **Virtualizer** | Themida VM, TinselCode | Kode dijalankan di VM custom |
| **Obfuscator** | Obfuscator-LLVM, Tigress | Sulitkan analisis statis |

### Alur Kerja Packer
```
Original Binary
    ↓ [Packing]
Packed Binary (+ stub loader)
    ↓ [Runtime - Loader executes]
Decrypted/Decompressed → Original Binary in Memory
    ↓ [Execution]
Original Code Runs
```

### Deteksi Packer
- Entropy analysis (entropy tinggi = packed)
- Packer signatures (UPX magic bytes: `UPX!`)
- Import table analysis (packed binary punya import minim)

---

## 18. Root Checker

### Definisi
Root checker adalah mekanisme yang mendeteksi apakah perangkat Android telah di-root (memiliki akses superuser).

### Metode Deteksi

#### a) File-based Detection
```java
String[] paths = {
    "/system/bin/su", "/system/xbin/su",
    "/sbin/su", "/data/local/xbin/su"
};
for (String path : paths) {
    if (new File(path).exists()) {
        return true; // rooted
    }
}
```

#### b) Package-based Detection
```java
String[] packages = {
    "com.topjohnwu.magisk",
    "eu.chainfire.supersu",
    "com.koushikdutta.superuser"
};
// Check via PackageManager
```

#### c) Build Property Detection
```java
String buildTags = Build.TAGS;
if (buildTags.contains("test-keys")) {
    return true; // dev build = likely rooted
}
```

#### d) Binary Execution Detection
```java
Runtime.getRuntime().exec("su -c id");
// Jika exit code == 0 → rooted
```

---

## 19. Anti Debug

### Definisi
Anti debug adalah mekanisme yang mendeteksi/mencegah debugger terhubung ke process.

### Metode Deteksi

#### a) API-based Detection
```c
// Windows
if (IsDebuggerPresent()) { exit(1); }

// Linux
if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1) {
    exit(1);
}
```

#### b) Timing-based Detection
```c
DWORD start = GetTickCount();
// ... code block yang di-debug ...
DWORD end = GetTickCount();
if ((end - start) > THRESHOLD) {
    // Kemungkinan di-debug
}
```

#### c) Hardware Breakpoint Detection
```c
CONTEXT ctx;
ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
GetThreadContext(GetCurrentThread(), &ctx);
if (ctx.Dr0 || ctx.Dr1 || ctx.Dr2 || ctx.Dr3) {
    // Hardware breakpoint detected
}
```

#### d) Exception-based Detection
```c
__try {
    RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
} __except(1) {
    // Debugger menangkap exception = normal
}
```

#### e) Process/Environment Checks
- Cek parent process (debugger biasanya parent)
- Cek environment variables
- Cek window titles (x64dbg, OllyDbg)
- Cek loaded DLLs/modules

---

## 20. Anti Cheat

### Definisi
Anti cheat adalah sistem yang mendeteksi dan mencegah kecurangan dalam game online.

### Tingkatan Anti-Cheat

| Level | Contoh | Metode |
|-------|--------|--------|
| **User-mode** | Custom hooks | Deteksi memory reading/writing |
| **Kernel-mode** | EasyAntiCheat, Vanguard | Driver-level monitoring |
| **Hardware** | TPM, Secure Boot | Hardware attestation |
| **Server-side** | Server validation | Sanity check, behavior analysis |

### Metode Deteksi
1. **Memory scanning:** Deteksi modified values (health, ammo, dll)
2. **Speed hack detection:** Bandingkan game timer vs system timer
3. **Aimbots:** Analisis pattern mouse movement
4. **Wallhacks:** Deteksi increased render distance/FOV
5. **DLL injection:** Cek loaded modules, integrity check
6. **Function hooking:** Cek return address integrity

---

## 21. Play Integrity Check

### Definisi
Play Integrity Check adalah API Google Play yang memverifikasi integritas perangkat dan instalasi app untuk mendeteksi rooting, debugging, dan tampering.

### Tingkatan Verifikasi
```
INTEGRITY_BASIC:
  → Verifikasi bahwa app legitimate (tidak di-repackage)
  → Verifikasi bahwa device tidak rooted

INTEGRITY_DEVICE:
  → + Verifikasi bootloader locked
  → + Verifikasi system integrity (SafetyNet-like)

INTEGRITY_VERDICT → nonce, timestamp, details
```

### Penggunaan
- Menentukan apakah user bisa mengakses fitur tertentu
- Deteksi kecurangan online banking/payment apps
- Compliance (perangkat harus terverifikasi)

---

## 22. SSL Unpinning

### Definisi
SSL unpinning adalah mekanisme yang memaksa aplikasi menerima certificate yang tidak dipinned (tidak di-whitelist), memungkinkan MITM (Man-in-the-Middle) untuk traffic analysis.

### SSL Pinning vs Unpinning
```
Normal (Pinned):
  App → SSL → Server (hanya terima certificate yang di-pin)

Unpinned:
  App → SSL → Proxy (menerima certificate apapun)
  Proxy → SSL → Server (MITM untuk analisis traffic)
```

### Metode Unpinning

#### a) Frida Script (Dynamic)
```javascript
Java.perform(function() {
    var TrustManager = Java.registerClass({
        name: 'com.custom.TrustManager',
        implements: [Java.use('javax.net.ssl.X509TrustManager')],
        methods: {
            checkClientTrusted: function(chain, authType) {},
            checkServerTrusted: function(chain, authType) {},
            getAcceptedIssuers: function() { return []; }
        }
    });
});
```

#### b) Objection (Tool)
```bash
objection -g com.target.app explore
android sslpinning disable
```

#### c) Network Security Config Bypass
```xml
<!-- AndroidManifest.xml -->
<network-security-config>
    <domain-config>
        <domain includeSubdomains="true">target.com</domain>
        <pin-set>
            <pin digest="SHA-256">base64==</pin>
        </pin-set>
    </domain-config>
</network-security-config>
```

#### d) Reverse Engineering Patching
Patch network library (OkHttp, gRPC) untuk bypass pin validation

---

## 23. Signature

### Definisi
Signature adalah identifikasi unik binary berdasarkan hash atau pattern tertentu.

### Jenis-jenis Signature

#### a) File Signature (Magic Bytes)
```
PE:   4D 5A (MZ)
ELF:  7F 45 4C 46
ZIP:  50 4B 03 04
PNG:  89 50 4E 47
```

#### b) Compiler/Library Signature
```python
strings_indicators = {
    "Microsoft Visual C++": "Microsoft Visual C",
    "GCC": "GCC:",
    "Go": "go.buildid",
    "Rust": ".rustc",
    ".NET": "mscoree.dll",
    "Unity": "Unity Engine",
}
```

#### c) Code Signature (Function Hash)
```python
# IDA FLIRT signature
# Membuat signature dari fungsi untuk identifikasi library
```

#### d) Malware Signature
```yara
rule Suspicious_Pattern {
    strings:
        $s1 = "CreateRemoteThread" ascii
        $s2 = "VirtualAllocEx" ascii
    condition:
        all of them
}
```

---

## 24. Deobfuscation

### Definisi
Deobfuscation adalah proses mengembalikan kode yang telah di-obfuscate (dibuat sulit dibaca) menjadi kode yang lebih readable dan dapat dipahami.

### Segmentasi Deobfuscation

#### A. Control Flow Deobfuscation
Mengembalikan alur kontrol yang telah dirusak:
```
Obfuscated:
  if (a > b) goto label_x;
  goto label_y;
  label_x: ...
  label_y: ...

Deobfuscated:
  if (a > b) {
      // do something
  } else {
      // do something else
  }
```

**Metode:**
- **Control Flow Graph (CFG) recovery:** Membangun ulang graph alur kontrol
- **Dominator tree analysis:** Menentukan hierarki blok kode
- **Pattern matching:** Identifikasi pola obfuscation

#### B. Data Flow Deobfuscation
Mengembalikan aliran data yang telah di-obfuscate:
```
Obfuscated:
  x = a ^ 0x42;
  y = x ^ 0x42;
  // y == a

Deobfuscated:
  y = a;  // constant folding
```

**Metode:**
- **Constant propagation:** Ganti variable dengan nilainya
- **Constant folding:** Hitung konstanta di compile time
- **Dead code elimination:** Hapus kode yang tidak berpengaruh

#### C. String Deobfuscation
Mengembalikan string yang telah di-encrypt/encoded:
```
Obfuscated:
  key = 0x5A;
  enc = "Hello" → XOR each byte with key
  runtime: decrypt(enc, key) → "Hello"

Deobfuscated:
  replace_all(calls_to_decrypt, "Hello")
```

**Metode:**
- **Static analysis:** Trace decryption function, extract key
- **Dynamic analysis:** Hook decryption function di runtime
- **Emulation:** Jalankan decryption routine di emulator
- **Pattern recognition:** Identifikasi XOR/AES/ROT13 patterns

#### D. Virtualization Deobfuscation
Mengembalikan kode yang dijalankan di VM custom:
```
Obfuscated:
  VM opcode 0x01 → push imm
  VM opcode 0x02 → pop reg
  VM opcode 0x03 → add reg, reg
  VM opcode 0xFF → exit VM

Deobfuscated:
  a = 5;
  b = 3;
  c = a + b;
```

**Metode:**
- **VM opcode recovery:** Identifikasi setiap opcode
- **Handler analysis:** Analisis fungsi handler untuk setiap opcode
- **IR reconstruction:** Bangun IR dari VM execution

### Metode Deobfuscation

| Metode | Tipe | Kelebihan | Kekurangan |
|--------|------|-----------|------------|
| **Static Analysis** | Tanpa runtime | Cepat, bisa batch | Tidak handle runtime-only obfuscation |
| **Dynamic Analysis** | Runtime | Handle semua tipe | Lambat, perlu environment |
| **Symbolic Execution** | Hybrid | Handle complex branching | Resource intensive |
| **AI/ML-based** | Hybrid | Handle pattern baru | Butuh training data |
| **Emulation** | Static | Handle packing | Lambat untuk binary besar |

---

## 25. Hooking

### Definisi
Hooking adalah teknik untuk intercept/memodifikasi perilaku fungsi pada runtime tanpa mengubah kode sumber.

### Segmentasi Hooking

#### A. User-Mode Hooking
Berjalan di level aplikasi (ring 3):
```
User Space:
  App Function → Hook → Original Function
  (intercepted) (modified behavior)
```

#### B. Kernel-Mode Hooking
Berjalan di level kernel (ring 0):
```
Kernel Space:
  Syscall → Hook → Original Handler
  (intercepted) (modified behavior)
```

### Metode Hooking

#### 1. Import Address Table (IAT) Hooking
**Cara kerja:** Mengubah pointer di Import Address Table untuk mengarah ke hook function.

```
Original IAT:
  MessageBoxA → user32.dll!MessageBoxA

Hooked IAT:
  MessageBoxA → MyHook!MyMessageBox
```

**Contoh Implementasi (Windows):**
```c
HMODULE hModule = GetModuleHandle(NULL);
PIMAGE_IMPORT_DESCRIPTOR importDesc = 
    (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
        hModule, TRUE, 
        IMAGE_DIRECTORY_ENTRY_IMPORT, &size);

while (importDesc->Name) {
    PIMAGE_THUNK_DATA thunk = 
        (PIMAGE_THUNK_DATA)((BYTE*)hModule + 
        importDesc->FirstThunk);
    
    while (thunk->u1.AddressOfData) {
        if (strcmp(((PIMAGE_IMPORT_BY_NAME)
            thunk->u1.AddressOfData)->Name, 
            "MessageBoxA") == 0) {
            thunk->u1.Function = (DWORD_PTR)MyHook;
        }
        thunk++;
    }
    importDesc++;
}
```

#### 2. Inline/Detour Hooking
**Cara kerja:** Menulis instruction JMP ke hook function di awal fungsi target.

```
Original (target function):
  push ebp
  mov ebp, esp
  ... (code)

Patched:
  jmp MyHook          ← 5 bytes (E9 XX XX XX XX)
  mov ebp, esp        ← overwritten
  ... (code)
```

**Trampoline Pattern:**
```
Original Function:
  push ebp         ← overwritten by JMP
  mov ebp, esp
  ...

Trampoline (holds original bytes + JMP back):
  push ebp         ← original bytes
  mov ebp, esp     ← original bytes
  jmp original+5   ← continue original code
```

**Implementasi (Linux):**
```c
#include <sys/mman.h>

void hook_function(void *target, void *hook, void **trampoline) {
    *trampoline = mmap(NULL, 12, PROT_READ | PROT_WRITE | PROT_EXEC,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memcpy(*trampoline, target, 5);
    *(BYTE*)((BYTE*)*trampoline + 5) = 0xE9;
    *(DWORD*)((BYTE*)*trampoline + 6) = 
        (DWORD)((BYTE*)target + 5) - (DWORD)((BYTE*)*trampoline + 10);
    mprotect(target, 4096, PROT_READ | PROT_WRITE | PROT_EXEC);
    *(BYTE*)target = 0xE9;
    *(DWORD*)((BYTE*)target + 1) = 
        (DWORD)hook - (DWORD)target - 5;
}
```

#### 3. VTable Hooking
**Cara kerja:** Mengubah virtual function pointer di vtable objek C++.

```
VTable (Array of function pointers):
  Index 0: → Func_A
  Index 1: → Func_B  ← hooked
  Index 2: → Func_C

Hooked VTable:
  Index 0: → Func_A
  Index 1: → MyHook_B  ← replaced
  Index 2: → Func_C
```

**Contoh:**
```cpp
class Target {
    virtual void Func_A();
    virtual void Func_B(); // target
    virtual void Func_C();
};

void MyHook_B() {
    // pre-processing
    originalFunc(); // call original
    // post-processing
}

DWORD_PTR* vtable = *(DWORD_PTR**)targetObject;
DWORD_PTR originalFunc = vtable[1];
vtable[1] = (DWORD_PTR)MyHook_B;
```

#### 4. PLT/GOT Hooking (Linux)
**Cara kerja:** Mengubah pointer di Global Offset Table (GOT) untuk shared library calls.

```
PLT (Procedure Linkage Table):
  call printf@plt
    → jmp *GOT[printf]  ← GOT entry

GOT (Global Offset Table):
  GOT[printf] → libc!printf  ← original
  GOT[printf] → MyPrintf     ← hooked
```

#### 5. Function Pointer Hooking
**Cara kerja:** Mengganti pointer fungsi secara langsung (untuk fungsi non-virtual).

```c
typedef int (*OriginalFunc)(int, int);
OriginalFunc originalFunc = NULL;

int HookedFunc(int a, int b) {
    printf("Intercepted: %d, %d\n", a, b);
    return originalFunc(a, b);
}

void InstallHook() {
    originalFunc = targetFunc;
    targetFunc = HookedFunc;
}
```

#### 6. Inline Trampoline (Advanced)
**Cara kerja:** Lebih canggih dari detour — handle absolute jumps dan wide instructions.

```
Target (x86-64):
  mov rax, [rbp+0x10]  ← 7 bytes
  add rax, rbx         ← 3 bytes
  
Patched:
  jmp short MyHook     ← 2 bytes (EB XX)
  nop                  ← 1 byte
  add rax, rbx         ← overwritten
  
Trampoline:
  mov rax, [rbp+0x10]  ← saved original
  jmp target+3         ← continue
```

#### 7. IAT Hooking Framework (Open Source)
```c
#include <MinHook.h>

int WINAPI My_MessageBoxW(HWND hWnd, LPCWSTR lpText, 
                          LPCWSTR lpCaption, UINT uType) {
    return MessageBoxW(hWnd, L"Patched!", lpCaption, uType);
}

MH_STATUS status = MH_Initialize();
status = MH_CreateHook(&MessageBoxW, &My_MessageBoxW, 
                       (LPVOID*)&Original_MessageBoxW);
status = MH_EnableHook(&MessageBoxW);
```

### Perbandingan Metode Hooking

| Metode | Level | Stealth | Kompatibilitas | Kompleksitas |
|--------|-------|---------|----------------|--------------|
| IAT Hooking | User | Medium | Terbatas (import) | Rendah |
| Inline/Detour | User | Rendah (deteksi) | Tinggi | Sedang |
| VTable Hooking | User | Medium | C++ virtual only | Sedang |
| PLT/GOT | User | Medium | Shared lib | Sedang |
| Inline Trampoline | User | Tinggi | Tinggi | Tinggi |
| Kernel Hooking | Kernel | Tinggi | Perlu driver | Sangat Tinggi |

---

## 26. Anti-Tampering

### Definisi
Anti-tampering adalah mekanisme pertahanan yang mendeteksi dan mencegah modifikasi tidak sah terhadap binary, data, atau runtime state program. Berbeda dengan anti-debug (fokus pada debugger), anti-tampering berfokus pada integritas kode dan data program.

### Segmentasi Anti-Tampering

#### A. Code Integrity Check
Memverifikasi bahwa kode program belum dimodifikasi:
```
Original Binary:
  Code Section → CRC/Hash → Expected Value

Runtime Check:
  Calculate CRC/Hash → Compare with Expected
  → Mismatch = TAMPERED
```

**Metode:**
- **CRC32/Checksum:** Cepat, tapi rentan bypass
- **SHA-256 Hash:** Lebih aman, tapi lebih lambat
- **Section Hash:** Hash per section (.text, .rdata, .data)
- **Code Patching Detection:** Cek instruksi tertentu (misal: NOP sled)

#### B. Data Integrity Check
Memverifikasi bahwa data runtime belum dimodifikasi:
```
Health: 100 (original)
Modified: 9999 (cheated)

Detection:
  - Encrypted health value
  - Checksummed data structure
  - Obfuscated pointer access
```

**Metode:**
- **Encrypted Variables:** Value di-encrypt dengan key dinamis
- **Checksummed Structures:** Setiap field memiliki checksum
- **Red Herring Variables:** Decoy values yang harus tetap sinkron
- **Memory Encryption:** Data sensitif di-encrypt di memory

#### C. Runtime State Validation
Memverifikasi bahwa runtime state konsisten:
```
Expected: HP=100, Ammo=30, Score=5000
Actual:   HP=100, Ammo=999, Score=999999
→ Mismatch = TAMPERED
```

**Metode:**
- **Cross-Validation:** Bandingkan nilai dari beberapa sumber
- **Timing Check:** Deteksi modifikasi via timing analysis
- **Behavior Analysis:** Deteksi anomali perilaku program
- **Server Validation:** Verifikasi value di server side

### Metode Implementasi

#### 1. Inline Integrity Check
```c
// Simple integrity check
void check_integrity() {
    uint32_t expected = 0x12345678;
    uint32_t actual = calculate_checksum(code_section);
    if (actual != expected) {
        // Tampering detected
        exit(1);
    }
}
```

#### 2. Function Prologue Check
```c
// Check function prologue hasn't been modified
void check_function(void *func) {
    uint8_t *bytes = (uint8_t*)func;
    if (bytes[0] != 0x55 || bytes[1] != 0x89 || bytes[2] != 0xE5) {
        // Function has been hooked/modified
        exit(1);
    }
}
```

#### 3. Self-Modifying Integrity
```c
// Dynamic integrity check that changes each run
static uint32_t integrity_key = 0;
void check_integrity() {
    uint32_t key = generate_dynamic_key();
    uint32_t checksum = calculate_checksum_with_key(key);
    if (checksum != stored_checksum) {
        exit(1);
    }
    integrity_key = key;
}
```

#### 4. Hardware Breakpoint Detection
```c
// Detect hardware breakpoints (used for tampering)
CONTEXT ctx;
ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
GetThreadContext(GetCurrentThread(), &ctx);
if (ctx.Dr0 || ctx.Dr1 || ctx.Dr2 || ctx.Dr3) {
    // Hardware breakpoint = potential tampering
    exit(1);
}
```

#### 5. Return Address Integrity
```c
// Check return address hasn't been modified
void check_return_address() {
    void *ret_addr = __builtin_return_address(0);
    if (!is_valid_address(ret_addr)) {
        // Return address has been tampered
        exit(1);
    }
}
```

### Anti-Tampering dalam Game Development

| Aspek | Implementasi |
|-------|--------------|
| **Score Validation** | Server-side validation, encrypted score values |
| **Health/Ammo** | Checksummed structures, red herring variables |
| **Position** | Physics validation, movement speed checks |
| **Unlockables** | Encrypted unlock flags, server validation |
| **Purchases** | Receipt validation, server-side verification |

### Bypass Techniques (untuk analisis)

1. **NOP Patching:** Ganti check dengan NOP instructions
2. **Jump Patching:** Skip integrity check function
3. **Memory Patching:** Modifikasi checksum/expected value
4. **Hooking:** Intercept check function, return always true
5. **Dynamic Key Extraction:** Extract encryption key dari runtime

---

## 27. Valid 0xFAB11BAF Magic Number

### Definisi
`0xFAB11BAF` adalah magic number (signature) yang digunakan oleh Unity IL2CPP untuk memvalidasi file metadata (`globalmetadata.dat`). Magic number adalah byte sequence unik di awal file yang mengidentifikasi format file.

### Struktur File Metadata Unity IL2CPP

```
globalmetadata.dat
┌─────────────────────────────────────────────────┐
│ Offset 0x00: Magic Number (4 bytes)             │
│   Value: 0xFAB11BAF (little-endian: AF 1B AB FA)│
├─────────────────────────────────────────────────┤
│ Offset 0x04: Version Number (4 bytes)           │
│   Value: 24, 27, 29, etc.                       │
├─────────────────────────────────────────────────┤
│ Offset 0x08: Size (4 bytes)                     │
│   Value: Total file size                        │
├─────────────────────────────────────────────────┤
│ Offset 0x0C: String Literal Offset              │
├─────────────────────────────────────────────────┤
│ Offset 0x10: String Literal Count               │
├─────────────────────────────────────────────────┤
│ ... (more header fields)                        │
├─────────────────────────────────────────────────┤
│ Offset 0xNN: String Literal Data                │
│   (actual string content)                       │
├─────────────────────────────────────────────────┤
│ Offset 0xMM: Type Definition Data               │
├─────────────────────────────────────────────────┤
│ Offset 0xKK: Method Definition Data             │
├─────────────────────────────────────────────────┤
│ Offset 0xLL: Field Definition Data              │
└─────────────────────────────────────────────────┘
```

### Validasi Magic Number

```cpp
// Dari CodeGraph context - Validation::ValidateHeader
bool ValidateHeader(const uint8_t* data, size_t size) {
    if (size < 16) return false;
    
    // Check magic number
    uint32_t magic = *reinterpret_cast<const uint32_t*>(data);
    if (magic != 0xFAB11BAF) {
        return false;  // Invalid magic
    }
    
    // Check version
    uint32_t version = *reinterpret_cast<const uint32_t*>(data + 4);
    if (version < 1 || version > 29) {
        return false;  // Invalid version
    }
    
    // Check size
    uint32_t file_size = *reinterpret_cast<const uint32_t*>(data + 8);
    if (file_size != size) {
        return false;  // Size mismatch
    }
    
    return true;
}
```

### Segmentasi Magic Number

#### A. Magic Byte Analysis
```
0xFAB11BAF dalam berbagai representasi:

Hex:        FA B1 1B AF
Binary:     1111 1010 1011 0001 0001 1011 1010 1111
Decimal:    4205701551
Little-endian: AF 1B AB FA (di file)
Big-endian:    FA B1 1B AF (di memory)
```

#### B. Magic Validation Flow
```
┌─────────────────────────────────────────────────────┐
│                 MAGIC VALIDATION FLOW               │
├─────────────────────────────────────────────────────┤
│                                                     │
│  1. READ FIRST 4 BYTES                              │
│     ↓                                               │
│  2. COMPARE WITH 0xFAB11BAF                         │
│     ├── Match → Continue to version check           │
│     └── No Match → REJECT (not IL2CPP metadata)    │
│                                                     │
│  3. READ NEXT 4 BYTES (version)                     │
│     ↓                                               │
│  4. VALIDATE VERSION RANGE                          │
│     ├── v24 (Unity 5.x - 2017)                     │
│     ├── v27 (Unity 2018 - 2019)                     │
│     ├── v29 (Unity 2020+)                           │
│     └── Invalid → REJECT                            │
│                                                     │
│  5. READ SIZE FIELD                                 │
│     ↓                                               │
│  6. VALIDATE FILE SIZE                              │
│     ├── Match actual size → VALID                   │
│     └── Mismatch → REJECT (corrupted)              │
│                                                     │
└─────────────────────────────────────────────────────┘
```

#### C. Version-Specific Magic
```
Unity IL2CPP Metadata Versions:

Version 14 (Unity 5.x):
  Magic: 0xFAB11BAF
  Features: Basic metadata

Version 19 (Unity 2017.1):
  Magic: 0xFAB11BAF
  Features: Added generic parameters

Version 21 (Unity 2017.3):
  Magic: 0xFAB11BAF
  Features: Added method body offsets

Version 22 (Unity 2018.1):
  Magic: 0xFAB11BAF
  Features: Added debug data

Version 24 (Unity 2018.3):
  Magic: 0xFAB11BAF
  Features: Added generic method specs

Version 27 (Unity 2020.2):
  Magic: 0xFAB11BAF
  Features: Added resource data

Version 29 (Unity 2021.2+):
  Magic: 0xFAB11BAF
  Features: Added assembly reload data
```

### Metode Analisis Magic Number

#### 1. File Signature Scanning
```python
# Scan untuk magic bytes di binary
MAGIC_IL2CPP = b'\xAF\x1B\xAB\xFA'  # 0xFAB11BAF little-endian

def find_il2cpp_metadata(data):
    offset = 0
    while True:
        pos = data.find(MAGIC_IL2CPP, offset)
        if pos == -1:
            break
        print(f"Found IL2CPP metadata at offset: 0x{pos:X}")
        offset = pos + 4
```

#### 2. Header Parsing
```python
import struct

def parse_metadata_header(data):
    magic = struct.unpack('<I', data[0:4])[0]
    version = struct.unpack('<I', data[4:8])[0]
    size = struct.unpack('<I', data[8:12])[0]
    
    if magic != 0xFAB11BAF:
        raise ValueError("Invalid magic number")
    
    return {
        'magic': hex(magic),
        'version': version,
        'size': size
    }
```

#### 3. Cross-Reference Validation
```cpp
// Validate multiple sections
bool validate_metadata(const uint8_t* data, size_t size) {
    // 1. Check magic
    if (*(uint32_t*)data != 0xFAB11BAF) return false;
    
    // 2. Check version
    uint32_t version = *(uint32_t*)(data + 4);
    if (!is_valid_version(version)) return false;
    
    // 3. Check string table offset
    uint32_t string_offset = *(uint32_t*)(data + 0x10);
    if (string_offset >= size) return false;
    
    // 4. Check type definition offset
    uint32_t type_offset = *(uint32_t*)(data + 0x18);
    if (type_offset >= size) return false;
    
    return true;
}
```

### Valid Magic Number Lainnya (Game Engines)

| Magic Value | Format | Engine/System |
|-------------|--------|---------------|
| `0xFAB11BAF` | IL2CPP Metadata | Unity IL2CPP |
| `0xFAB11DAF` | IL2CPP Metadata (v27+) | Unity 2020+ |
| `0x5A4F4F43` ("COOZ") | GameMaker data.win | GameMaker Studio |
| `0x474D5300` ("MSG\x00") | Source 2 VPK | Valve Source 2 |
| `0x05014B50` | ZIP/CraftStudio | CraftStudio |
| `0x416E6472` ("Andr") | Android APK | Android Package |
| `0x504B0304` | ZIP Format | JAR/APK/AAB |
| `0x7F454C46` ("\x7fELF") | ELF Binary | Linux/Android |
| `0x4D5A9000` ("MZ\x90\x00") | PE Binary | Windows DLL/EXE |

### Magic Number dalam Konteks Keamanan

#### 1. Anti-Tampering via Magic
```cpp
// Validate magic sebelum execute
bool validate_before_execute() {
    uint32_t magic = read_metadata_magic();
    if (magic != EXPECTED_MAGIC) {
        // Metadata telah dimodifikasi
        log_security_event("TAMPER_DETECTED");
        terminate_process();
    }
    return true;
}
```

#### 2. Encrypted Magic
```cpp
// Magic di-encrypt untuk mencegah patching
uint32_t get_encrypted_magic() {
    uint32_t encrypted = read_from_file();
    uint32_t key = generate_runtime_key();
    return encrypted ^ key;  // Decrypt magic
}
```

#### 3. Dynamic Magic
```cpp
// Magic berubah setiap build
uint32_t calculate_dynamic_magic(uint32_t build_number) {
    return 0xFAB11BAF ^ (build_number * 0x12345678);
}
```

### Tools untuk Analisis Magic

| Tool | Fungsi |
|------|--------|
| **Il2CppDumper** | Parse metadata dengan magic validation |
| **File** (Linux) | Identifikasi file type via magic |
| **binwalk** | Scan magic bytes di binary |
| **hexdump** | Visual inspection magic bytes |
| **010 Editor** | Template-based magic analysis |

### Troubleshooting Magic Validation

```
Common Errors:

1. "Invalid magic number"
   → File bukan IL2CPP metadata
   → File corrupted
   → File di-encrypt (perlu decrypt dulu)

2. "Invalid version"
   → Versi metadata tidak didukung
   → Perlu update parser untuk versi baru

3. "Size mismatch"
   → File terpotong (incomplete)
   → Header corrupted

4. "Offset out of bounds"
   → Metadata corrupted
   → Pointer manipulation detected
```

---

## 28. Kesimpulan

### Ringkasan Konsep

| Konsep | Tujuan Utama |
|--------|-------------|
| Synthesizer | Generate input yang valid dan terstruktur |
| Taint Analysis | Melacak alur data dari attacker ke critical sink |
| Expression Synthesis | Simplifikasi kode, pattern matching, deobfuscation |
| Memory Segmentation | Memahami layout memori (code vs data vs heap vs stack) |
| Coverage Strategies | Mengukur seberapa jauh fuzzing mengeksplorasi program |
| Pointer Coverage | Mendeteksi memory corruption via analisis pointer |
| Sanitizer | Deteksi runtime buffer overflow, UAF, dll |
| Heap Allocator | Memahami exploitation primitive di heap |
| Metadata (IL2CPP) | Memahami symbolic info di Unity/game binaries |
| Recursive Binary Unpacker | Membongkar binary bertingkat sampai original |
| Section Carver | Mengekstrak section spesifik dari binary PE/ELF |
| Entropy | Mendeteksi encryption/compression via randomitas data |
| IR (Intermediate Representation) | Bahasa perantara untuk decompiler/compiler |
| Stacktrace | Rekaman urutan fungsi untuk crash analysis |
| Patching | Mengubah binary untuk modifikasi perilaku |
| Compiler | Pipeline source code → machine code |
| Packer | Tool proteksi binary dari analisis |
| Root Checker | Mendeteksi perangkat Android yang di-root |
| Anti Debug | Mendeteksi/mencegah debugger terhubung |
| Anti Cheat | Mencegah kecurangan dalam game online |
| Play Integrity Check | Verifikasi integritas perangkat Android |
| SSL Unpinning | Bypass certificate pinning untuk analisis traffic |
| Signature | Identifikasi unik binary berdasarkan pattern |
| Deobfuscation | Mengembalikan kode yang di-obfuscate |
| Hooking | Intercept/modifikasi fungsi pada runtime |
| Anti-Tampering | Mendeteksi/mencegah modifikasi tidak sah |
| Magic Number (0xFAB11BAF) | Validasi format file Unity IL2CPP |

### Keterkaitan Antar Konsep

Semua konsep ini saling terkait dalam workflow reverse engineering:

1. **Synthesizer** menghasilkan input untuk program
2. **Taint Analysis** melacak bagaimana input mempengaruhi program
3. **Coverage Strategies** mengukur efektivitas eksplorasi
4. **Expression Synthesis** membantu memahami kode yang kompleks
5. **Memory Segmentation** memberikan pemahaman tentang layout program
6. **Pointer Coverage** dan **Sanitizer** membantu mendeteksi vulnerability
7. **Heap Allocator** memahami primitive untuk exploitation
8. **Metadata** memberikan symbolic information untuk analisis game/mobile binaries
9. **Entropy** membantu mendeteksi packed/encrypted binary
10. **Recursive Binary Unpacker** membongkar binary bertingkat
11. **Section Carver** mengekstrak bagian spesifik untuk analisis
12. **IR** menjadi jembatan antara machine code dan pseudocode
13. **Compiler** memahami optimasi yang mempengaruhi kode
14. **Packer** memahami proteksi yang perlu dibypass
15. **Stacktrace** membantu crash analysis dan debugging
16. **Patching** memodifikasi binary untuk analisis/modifikasi
17. **Root Checker** memahami deteksi root di Android
18. **Anti Debug** memahami pertahanan terhadap debugger
19. **Anti Cheat** memahami sistem keamanan game
20. **Play Integrity Check** verifikasi integritas perangkat
21. **SSL Unpinning** memungkinkan analisis traffic network
22. **Signature** identifikasi binary dan library
23. **Deobfuscation** mengembalikan kode yang sulit dibaca
24. **Hooking** memodifikasi perilaku program secara dinamis
25. **Anti-Tampering** memahami pertahanan integritas kode
26. **Magic Number** identifikasi format file dan validasi

### Workflow Reverse Engineering Lengkap

```
┌─────────────────────────────────────────────────────────────────┐
│                    WORKFLOW REVERSE ENGINEERING                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. RECONNAISSANCE                                              │
│     ├── File Signature → Identifikasi format binary             │
│     ├── Magic Number Validation → Verifikasi format file        │
│     ├── Entropy Analysis → Deteksi packing/encryption           │
│     └── Section Carver → Ekstrak section untuk analisis         │
│                                                                 │
│  2. STATIC ANALYSIS                                             │
│     ├── Recursive Binary Unpacker → Buang packing layers        │
│     ├── IR (Ghidra/IDA) → Bangun intermediate representation   │
│     ├── Expression Synthesis → Simplifikasi kode kompleks       │
│     └── Compiler Analysis → Pahami optimasi compiler            │
│                                                                 │
│  3. DYNAMIC ANALYSIS                                            │
│     ├── Anti Debug Bypass → lumpuhkan pertahanan debugger       │
│     ├── Hooking → Intercept fungsi untuk monitoring             │
│     ├── SSL Unpinning → Analisis traffic network                │
│     └── Root Checker Bypass → Akses root detection              │
│                                                                 │
│  4. DEOBFUSCATION                                               │
│     ├── Control Flow Recovery → Perbaiki alur kontrol           │
│     ├── Data Flow Analysis → Simplifikasi data propagation      │
│     ├── String Deobfuscation → Decrypt strings tersembunyi      │
│     └── VM Deobfuscation → Reverse virtualization               │
│                                                                 │
│  5. VULNERABILITY ANALYSIS                                      │
│     ├── Taint Analysis → Track data dari source ke sink         │
│     ├── Pointer Coverage → Deteksi memory corruption            │
│     ├── Sanitizer Analysis → Identifikasi vulnerability         │
│     └── Heap Analysis → Pahami exploitation primitive           │
│                                                                 │
│  6. EXPLOITATION / MODIFICATION                                 │
│     ├── Patching → Modifikasi binary                            │
│     ├── Hooking → Runtime behavior modification                 │
│     ├── Anti-Tampering Bypass → Circumvent integrity checks     │
│     ├── Packer Bypass → Anti-protection techniques              │
│     └── Anti Cheat Bypass → Game security circumvention         │
│                                                                 │
│  7. DOCUMENTATION                                               │
│     ├── Metadata Analysis → Extract symbolic information        │
│     ├── Stacktrace Analysis → Document crash behavior           │
│     └── Play Integrity → Understand device attestation          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Relevansi untuk Pengembangan OmniByte

Pemahaman terhadap konsep-konsep ini penting untuk:

#### Analisis Binary
- **Entropy & Signature** → Deteksi format dan proteksi binary
- **Magic Number (0xFAB11BAF)** → Validasi format file IL2CPP
- **Section Carver & Recursive Unpacker** → Ekstrak kode dari binary terproteksi
- **IR & Compiler** → Memahami bagaimana kode dieksekusi

#### Game Engine Analysis
- **Metadata (IL2CPP)** → Extract type/method dari Unity games
- **Magic Number Validation** → Verifikasi dan parse metadata files
- **Hooking** → Intercept fungsi game untuk modifikasi
- **Anti-Tampering** → Memahami pertahanan integritas game
- **Anti Cheat Bypass** → Memahami dan bypass proteksi game

#### Security Research
- **Deobfuscation** → Membongkar kode yang dilindungi
- **Taint Analysis** → Menemukan vulnerability
- **Patching** → Modifikasi perilaku program
- **Anti-Tampering Bypass** → Circumvent integrity checks

#### Android Security
- **Root Checker** → Memahami deteksi root
- **SSL Unpinning** → Analisis traffic aplikasi Android
- **Play Integrity Check** → Verifikasi integritas perangkat

#### Tools Development
- **Hooking Framework** → Membangun tools analisis dinamis
- **Stacktrace Analysis** → Debugging dan crash analysis
- **Packer/Protector Analysis** → Reverse engineering proteksi

---

**Dokumen ini merupakan bagian dari proyek Pengembangan OmniByte dan disusun sebagai referensi teknis untuk tim pengembang.**
