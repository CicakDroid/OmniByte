# Laporan Penelitian: Konsep Inti Reverse Engineering untuk Pengembangan OmniByte

**Nama Proyek:** Pengembangan OmniByte
**Tanggal:** 2026-09-05
**Status:** Final

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
10. [Kesimpulan](#10-kesimpulan)

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

## 10. Kesimpulan

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

### Relevansi untuk Pengembangan OmniByte

Pemahaman terhadap konsep-konsep ini penting untuk:
- Mengembangkan tools analisis binary yang efektif
- Membangun fuzzer yang dapat menemukan vulnerability kompleks
- Memahami teknik exploitation modern
- Menganalisis binary tanpa source code
- Membangun sistem pertahanan yang robust
- Menganalisis game/mobile applications (Unity IL2CPP, Android APK)

---

**Dokumen ini merupakan bagian dari proyek Pengembangan OmniByte dan disusun sebagai referensi teknis untuk tim pengembang.**
