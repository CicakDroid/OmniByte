# Laporan Riset: Android Root Ecosystem — Build, Distribusi & Hiding Frameworks

**Tanggal:** 2026-09-06
**Status:** Research selesai

---

## 1. AnyKernel3 (osm0sis/AnyKernel3)

### Ringkasan
Flashable zip template standar untuk distribusi custom kernel ke semua device Android. Ini adalah **de facto standard** untuk distribusi kernel zip di ekosistem Android rooting.

### Sumber Resmi
- **Repository:** https://github.com/osm0sis/AnyKernel3
- **Author:** osm0sis @ xda-developers
- **Stars:** 1k+
- **Basis:** Magiskboot (topjohnwu)

### Struktur
```
AnyKernel3/
├── update-binary          # Script flasher utama (menggunakan magiskboot)
├── anykernel.sh           # Config + custom install commands
├── tools/                 # magiskboot, busybox, utilitas
├── ramdisk/               # File ramdisk untuk dimasukkan ke boot image
├── vendor_ramdisk/        # Untuk multi-partition vendor_boot v3
├── modules/               # Kernel modules (full path: /modules/system/lib/modules)
├── patch/                 # Partial files untuk ramdisk modifications
├── banner                 # Banner yang ditampilkan saat flash
└── version                # Version info yang ditampilkan saat flash
```

### Command Utama

| Command | Deskripsi |
|---------|-----------|
| `dump_boot` | Unpack full boot image (kernel + ramdisk) |
| `write_boot` | Repack dan flash boot image |
| `split_boot` | Unpack hanya kernel, skip ramdisk |
| `unpack_ramdisk` | Unpack ramdisk saja |
| `repack_ramdisk` | Repack ramdisk saja |
| `flash_boot` | Flash kernel saja tanpa ramdisk changes |
| `flash_generic` | Flash image ke partition (dtbo, vendor_dlkm, dll) |

### Alur Kerja
```
Input: boot.img (atau partition path)
  ↓
magiskboot: unpack → kernel image + ramdisk
  ↓
Replace kernel image (Image.gz-dtb dari custom kernel)
  ↓
Apply ramdisk modifications (dari /ramdisk/, /patch/)
  ↓
magiskboot: repack → new boot.img
  ↓
Patch Magisk root (otomatis detect & pertahankan)
  ↓
Flash ke /boot partition
```

### Konfigurasi (anykernel.sh)
```bash
### AnyKernel3 Ramdisk Mod Script
### osm0sis @ xda-developers

### AnyKernel setup
kernel.string=CustomKernel by developer @ xda
do.devicecheck=1
device.name1=device_codename
device.name2=
supported.versions=

### AnyKernel install
dump_boot
# ... ramdisk modifications ...
write_boot
```

### Multi-Partition Support
- **Standard:** Single `/boot` partition
- **Multi-partition (vendor_boot v3):** Separate `/vendor_ramdisk`, `/vendor_patch`
- **OG AK mode:** Skip ramdisk changes entirely

---

## 2. WildKernels/GKI_KernelSU_SUSFS

### Ringkasan
Pre-built custom kernel images GKI 2.0 (Android 12+) dengan **KernelSU-Next** + **SUSFS** + **Baseband Guard** sudah ter-patch, siap flash ke device.

### Sumber Resmi
- **Repository:** https://github.com/WildKernels/GKI_KernelSU_SUSFS
- **Kategori:** Pre-built kernel images, bukan source code
- **Versi terakhir:** Release 8 (2026-08-12), v2.0.0-r19 (stable)

### Komponen

| Komponen | Versi | Deskripsi |
|----------|-------|-----------|
| **KernelSU-Next** | Latest | Root solution yang bekerja di kernel space |
| **SUSFS** | v2.2.0 | Kernel patch + userspace module untuk sembunyikan root |
| **Baseband Guard (BBG)** | - | LSM ringan yang blokir write ke critical partitions |
| **DroidSpaces-OSS** | - | Lightweight Linux container support |
| **NTSync** | - | NTFS sync support |
| **IPSet/Wireguard/BBR** | - | Networking improvements |

### Kernel Versi yang Didukung
Semua kernel GKI 2.0 dengan KMI versi 5.10+:
- 5.10.x (Android 12-13)
- 5.15.x (Android 13-14)
- 6.1.x (Android 14-15)
- 6.6+ (Android 15+)

### Bypass Builds
Modifikasi untuk bypass module check — memaksa load kernel module meskipun version mismatch, missing dependencies, atau signature verification gagal.

### Cara Install
```bash
# 1. Download AnyKernel3 zip yang match KMI version
# 2. Flash via salah satu metode:
#    - KernelSU-Next app → Install from storage
#    - TWRP recovery → Install zip
#    - Kernel Flasher app (perlu root existing)
#    - ADB sideload: adb sideload AnyKernel3-*.zip

# 3. Install SUSFS module
ksud module install ksu_module_susfs.zip

# 4. Install KernelSU-Next manager app
```

### Build Output Format
```
5.10.107-android13-2022-11-AnyKernel3.zip    (20.8 MB)
5.15.123-android14-2024-01-AnyKernel3.zip    (~21 MB)
6.1.56-android15-2024-11-AnyKernel3.zip      (~22 MB)
```

---

## 3. Numbersf/Action-Build

### Ringkasan
GitHub Actions workflow yang fully automates build custom kernel untuk **semua device OnePlus** (QCOM & MTK, GKI 2.0), output: AnyKernel3 flashable zip.

### Sumber Resmi
- **Repository:** https://github.com/Numbersf/Action-Build
- **Target devices:** OnePlus (Ace, Nord, 10/11/12/13/15 series, Pad), Oppo, Realme
- **Supported kernels:** KernelSU-Next, SukiSU-Ultra, ReSukiSU
- **Telegram:** https://t.me/AndroidCoreLayer

### Feature Toggle

| Category | Feature | Options |
|----------|---------|---------|
| **Root** | Kernel Manager | SukiSU-Ultra, KernelSU-Next, ReSukiSU |
| **Hiding** | SUSFS | CI / Release / N/A / -1 (disable) |
| **Hiding** | SUSFS hash rollback | Commit hash, count, atau -1 |
| **Modules** | KPM | KPM / KPN / N/A |
| **Protection** | Baseband Guard (BBG) | boolean |
| **Storage** | ZRAM | `0/lz4kd/8589934592` |
| **Network** | IPSet, Wireguard, BBRv1, BBRv3, ECN | boolean |
| **Container** | DroidSpaces | boolean |
| **Scheduler** | Fengchi Driver (HMBIRD) | boolean |
| **Kernel** | Re-Kernel | boolean |
| **Performance** | Fast Build (ccache-ECS) | boolean |
| **Spoofing** | Custom suffix | string |
| **Spoofing** | SUBLEVEL spoofing | string |

### Device Configs (50+ OnePlus devices)
```
oneplus_nord_n30_se_5g_v     oneplus_10r_v
oneplus_nord_3_b              oneplus_ace_v
oneplus_ace_race_v            oneplus_10_pro_b
oneplus_10t_v                 oneplus_11r_b
oneplus_ace2_b                oneplus_pad_lite_b
oneplus_pad_mt6983_b          oneplus_ace_2v_b
oneplus_ace_pro_v             oneplus_11_b
oneplus_12r_b                 oneplus_ace2_pro_b
oneplus_ace3_b                oneplus_open_b
oneplus_nord_ce4_b            oneplus_n6_b
oneplus_n6x_b                 oneplus_12_b
oneplus_pad_go_2_b            oneplus_turbo_6x_b
oneplus_nord_ce4_lite_5g_b   oneplus_nord_ce6
oneplus_nord_ce6_lite_b       oneplus_turbo_6v
oneplus_nord_4_b              oneplus_ace_3v_b
oneplus_pad_mt6897_b          oneplus_13r_b
oneplus_ace3_pro_b            oneplus_ace5_b
oneplus_pad_pro_b             oneplus_pad2_b
oneplus_nord_ce5_b            oneplus_nord_5_b
oneplus_ace5_pro_b            oneplus_13_b
oneplus_13t_b                 oneplus_13s_b
oneplus_pad_2_pro_b           oneplus_pad_3_b
oneplus_ace5_race_b           oneplus_ace5_ultra_b
oneplus_pad2_mt6991_b         oneplus_ace_6
oneplus_turbo_6               oneplus_nord_6
oneplus_ace_6t                oneplus_15r
oneplus_15                    oneplus_15t
oneplus_pad_3_pro             oneplus_pad_4
oneplus_ace6_ultra
```

### Build Output Format
```
AnyKernel3_SukiSUUltra_34895_OnePlusAce2Pro_Android16.0.0(5.15.180)_KPM_BBG_ILH_DS_REKER.zip
AnyKernel3_SukiSUUltra_34895_OnePlus11_Android14.1.0(5.15.123)_KPM_BBG_ILH_DS_REKER.zip
AnyKernel3_SukiSUUltra_34895_OnePlus15(AOSP)_Android16.0.0(6.12.23)_KPM_BBG_ILH_DS_REKER.zip
```

### Build Time Reference

| Mode | Duration |
|------|----------|
| Fast Build (semua device) | 41min ~ 42min |
| 5.10-5.15 (official script) | 29min ~ 45min |
| 6.1-6.12 (official script) | 1h12min ~ 1h28min |

### Cara Pakai
```bash
# 1. Fork repo Numbersf/Action-Build
# 2. Enable GitHub Actions di fork
# 3. Trigger workflow → pilih device config + features
# 4. Download AnyKernel3 zip dari GitHub Releases
# 5. Flash ke device via KernelSU app / TWRP / Kernel Flasher
```

---

## 4. MMRLApp/DEXMO

### Ringkasan
Gradle plugin untuk compile Java/Kotlin source files + dependencies ke format **DEX** (Android's executable format) menggunakan D8 compiler.

### Sumber Resmi
- **Repository:** https://github.com/MMRLApp/DEXMO
- **Plugin ID:** `dev.mmrl.dexmo`
- **Version:** 1.0.0
- **Gradle Portal:** https://plugins.gradle.org/plugin/dev.mmrl.dexmo

### Cara Pakai

#### Basic Setup
```kotlin
plugins {
    id("com.android.library") version "8.3.0"   // atau com.android.application
    kotlin("android") version "2.0.0"
    id("dev.mmrl.dexmo") version "1.0.0"
}
```

#### Tambah Dependencies
```kotlin
dependencies {
    // Dependencies ini akan di-compile dan dimasukkan ke DEX output
    dexmoInclude("com.example:library:1.0.0")
    dexmoInclude("org.jetbrains.kotlin:kotlin-stdlib:2.0.0")
}
```

#### Build
```bash
./gradlew build
# Output: .dex file dari source + dependencies
```

### Published Plugin Coordinates

| Property | Value |
|----------|-------|
| Plugin ID | `dev.mmrl.dexmo` |
| Group | `dev.mmrl.dexmo` |
| Artifact | `dexmo` |
| Version | `1.0.0` |

### Integration dengan MMRL Ecosystem
```
MMRLApp/
├── MMRL          # Main app (module manager) - 2051 stars
├── DEXMO         # Gradle plugin untuk build DEX
├── MMRL-Util     # CLI untuk build module repositories
├── MMRL-CLI      # CLI untuk install modules
├── mmrl_install_tools  # Required module untuk install dari Explore
└── template-repository  # Template untuk create module repository
```

---

## 5. Root Hiding Frameworks (8 Project)

### 5.1 MMRL/RootThread

| Field | Value |
|-------|-------|
| Stars | 6 |
| Architecture | Magisk module (post-fs-data) |
| Approach | Shizuku API-based root server |
| Stealth | Medium |
| Status | Active, 2026 |

Magisk module yang berjalan sebagai post-fs-data, mengimplementasikan Shizuku-compatible root server. Process-level hiding menggunakan `PR_SET_NAME` untuk menyamar sebagai `servicemanager`.

### 5.2 libsu (topjohnwu)

| Field | Value |
|-------|-------|
| Stars | 8.1k |
| Architecture | Java library |
| Approach | Embedded `su` binary + root process |
| Stealth | N/A (library, bukan hiding) |
| Status | Stable (v6.0.0) |

Java library oleh Magisk author untuk embed `su` binary di APK. Programmatic root access untuk apps.

```java
Shell.Builder.create()
    .setFlags(Shell.FLAG_MOUNT_MASTER)
    .setInitializers(...)
    .build();
```

### 5.3 Sui (XiaoTong6666/Sui)

| Field | Value |
|-------|-------|
| Stars | 534 |
| Architecture | Zygisk module |
| Approach | Java root server via Shizuku API |
| Stealth | High (app-level) |
| Status | Active fork |

Zygisk module yang inject ke system_server, hooks `Binder#execTransact`, routes root binder berdasarkan UID permissions. Hidden UIDs blocked dari root bridge.

### 5.4 SukiSU-Ultra

| Field | Value |
|-------|-------|
| Stars | 8k |
| Architecture | KernelSU fork (kernel-level) |
| Approach | Kernel patching, Zygisk modules |
| Stealth | Very High (kernel-level) |
| Status | Active, 2026 |

KernelSU fork dengan enhanced hiding, SUSFS support, custom kernel patches. Kernel-level patching via `supercall` syscall, KPM loader, obfuscated directory structure, process masquerading.

### 5.5 Diamorphine (M0nad)

| Field | Value |
|-------|-------|
| Stars | 2k |
| Architecture | LKM (Loadable Kernel Module) |
| Approach | Kernel syscall hooking |
| Stealth | Very High (kernel-level rootkit) |
| Status | Stable, minimal updates |

Classic Linux rootkit untuk Android. Hook `getdents64` (directory listing), `kill` (magic commands), self-hiding dari `/proc/modules`. Tidak work di modern GKI kernels.

### 5.6 Bypasser (LRFP-Team)

| Field | Value |
|-------|-------|
| Stars | 67 |
| Architecture | Magisk/KernelSU module |
| Approach | A/B shell scripts, HMA + Tricky Store |
| Stealth | High (multi-vector) |
| Status | Active, 2026 |

Systematic environment hardening module. Fitur bitmask:

| Bit | Feature |
|-----|---------|
| 0b000001 | Welcome (built-in configs) |
| 0b000010 | Update (dynamic updates) |
| 0b000100 | Zygisk Traces |
| 0b001000 | HMA Configurations |
| 0b010000 | Tricky Store |
| 0b100000 | Shell commands |

Shell commands: disable sensitive Google apps, handle SELinux, patch system config XML, hide desktop icons, enforce SELinux, check kernel version.

### 5.7 stealth-poc

| Field | Value |
|-------|-------|
| Status | POC (Proof-of-Concept) |
| Architecture | Kernel-level |
| Approach | Syscall interception |

Tidak ditemukan di GitHub publik (kemungkinan repo privat). POC stage, tidak production-ready.

### 5.8 Perbandingan

| Framework | Stars | Level | Approach | Stealth | Status |
|-----------|-------|-------|----------|---------|--------|
| MMRL/RootThread | 6 | Userspace | Shizuku API | Medium | Active |
| libsu | 8.1k | Library | Embedded su | N/A | Stable |
| Sui | 534 | App-level | Zygisk + Binder | High | Active |
| SukiSU-Ultra | 8k | Kernel | KernelSU fork | Very High | Active |
| Diamorphine | 2k | Kernel | LKM hooks | Very High | Stable |
| Bypasser | 67 | Module | Multi-vector | High | Active |
| stealth-poc | N/A | Kernel | Syscall intercept | Experimental | POC |

---

## 6. Integrasi & Alur Komplit

```
Kernel Source (e.g., OnePlus kernel source)
  │
  ▼ [Action-Build: GitHub Actions]
  │
  ├── Apply patches: SUSFS + KPM + BBG + networking
  ├── Compile: Clang/LLVM toolchain
  │
  ▼ [Action-Build: output]
  │
  ├── Kernel image: Image.gz-dtb
  ├── Modules: *.ko files
  │
  ▼ [AnyKernel3 packaging]
  │
  ├── Update-binary (magiskboot)
  ├── anykernel.sh (config)
  ├── /ramdisk/ (ramdisk modifications)
  ├── /modules/ (kernel modules)
  │
  ▼ [ZIP output]
  │
  ├── AnyKernel3_SukiSUUltra_device_name.zip
  │
  ▼ [Distribution]
  │
  ├── WildKernels (pre-built releases)
  ├── GitHub Releases
  ├── Telegram channels
  │
  ▼ [Install di device]
  │
  ├── Flash via KernelSU app
  ├── Flash via TWRP
  ├── Flash via Kernel Flasher
  │
  ▼ [Device boot]
  │
  ├── KernelSU-Next: root granted dari kernel
  ├── SUSFS: sembunyikan root dari detection
  ├── BBG: protect critical partitions
  │
  ▼ [Module ecosystem]
  │
  ├── MMRL: manage modules
  ├── DEXMO: build module packages
  └── User installs modules
```

### Relevansi dengan OmniByte

| Project | Relevansi |
|---------|-----------|
| **WildKernels** | Referensi artifact yang diproduksi |
| **AnyKernel3** | Format distribusi yang harus dipahami |
| **Action-Build** | Template untuk automated build pipeline |
| **DEXMO** | Tool untuk build modules dengan Java/Kotlin code |
| **Root Hiding (SUSFS/SukiSU)** | Attack vectors untuk signature detection |
| **Diamorphine** | Historical reference untuk rootkit patterns |
| **Bypasser** | Multi-vector hardening approach |

---

## 7. libsu Programmatic Root Access

### Ringkasan
libsu (topjohnwu) adalah Java library yang menyediakan **programmatic root access** — artinya app bisa menjalankan root commands dari Java/Kotlin code tanpa user harus manual ketik di terminal.

**Koreksi umum**: libsu **TIDAK embed su binary ke APK**. Yang dilakukan:
```
App (OmniByte) → panggil "su" → root shell process → execute commands
                    ↑
              su binary dari Magisk/KernelSU/SukiSU (sudah terinstall di device)
```

### Sumber Resmi
- **Repository:** https://github.com/topjohnwu/libsu
- **Author:** topjohnwu (Magisk author)
- **Stars:** 8.1k
- **Versi terakhir:** v6.0.0
- **Install:** JitPack (`com.github.topjohnwu.libsu:core:6.0.0`, `:service:6.0.0`)

### Arsitektur

```
┌─────────────────────────────────────────────────────┐
│  OmniByte App Process (com.omnibyte.app)            │
│                                                     │
│  Shell.cmd("cat /proc/1/maps").exec()               │
│       │                                             │
│       ▼                                             │
│  Shell.Builder.build()                              │
│       │                                             │
│       ├── Coba: Runtime.exec("su")                  │
│       │         → root shell process (uid=0)        │
│       │                                             │
│       └── Fallback: Runtime.exec("sh")              │
│                 → non-root shell (uid=app_uid)      │
│                                                     │
│  Root shell execute: cat /proc/1/maps               │
│  Return: Shell.Result { stdout, stderr, exitCode }  │
└─────────────────────────────────────────────────────┘
```

### Level 1: Shell API (_root shell execution_)

Dari Java/Kotlin code, app bisa execute root commands tanpa user intervensi:

```kotlin
// Static API — jalankan command di root shell utama
val result = Shell.cmd("cat /proc/$pid/maps").exec()
val maps = result.out  // list of memory mappings
val exitCode = result.code

// Chained commands
Shell.cmd("su -c 'ls -la /data'").exec()

// Async execution
Shell.cmd("cat /proc/kallsyms").exec { result ->
    // Callback when done
}
```

**Key classes:**
- `Shell` — manages a Unix shell process (root atau non-root)
- `Shell.Builder` — configure shell creation behavior
- `Shell.Result` — stdout, stderr, exitCode dari executed commands
- `Shell.Job` — async shell job

### Level 2: RootService (_root process dengan IPC_)

Jalankan Java/Kotlin/C++ code di **root process** via Binder IPC:

```
┌──────────────────────┐          ┌──────────────────────────┐
│  OmniByte App        │  Binder  │  Root Service Process    │
│  (non-root)          │◄────────►│  (uid=0, root)           │
│                      │   IPC    │                          │
│  // Client side      │          │  // Runs as root         │
│  RootService.bind()  │          │  RootService.onBind()    │
│                      │          │  └─ load native lib      │
│                      │          │  └─ execute C++ code     │
└──────────────────────┘          └──────────────────────────┘
```

**Use case untuk OmniByte:**
- MemoryIO operations (read/write target process memory)
- ProcessManager (inspect/modify target process)
- SymbolResolver (resolve symbols di process memory)
- ZigZag stealth operations

```kotlin
// Server side — runs in root process
class HookService : RootService() {
    override fun onBind(intent: Intent): IBinder {
        System.loadLibrary("omnibyte_native")  // Load C++ code
        return HookBinder.Stub.asInterface(this)
    }
}

// Client side — bind dari app
RootService.bind(intent, object : ServiceConnection {
    override fun onServiceConnected(name: ComponentName, service: IBinder) {
        val hookBinder = HookBinder.Stub.asInterface(service)
        hookBinder.attachToProcess(targetPid)  // Execute with root
    }
})
```

### Level 3: Remote NIO (_root file I/O_)

```java
// Dari app process (non-root)
ExtendedFile boot = remoteFS.getFile("/dev/block/by-name/boot");
InputStream in = boot.newInputStream();  // akses via root process
```

### Programmatic vs Manual

| Manual (tanpa libsu) | Programmatic (dengan libsu) |
|---|---|
| User buka terminal, ketik `su` | App otomatis panggil `su` saat startup |
| User ketik command manual | App execute command dari Kotlin/Java code |
| User copy-paste output | App baca output via `Shell.Result` |
| Tidak bisa integrate ke UI | Root operations integrate ke app flow |

### Alur: App → Root Access

```
User buka OmniByte
    │
    ▼
SplashActivity: Shell.getShell(callback)
    │
    ▼
Shell.Builder.build():
    ├── 1. exec("su") → SUCCESS → root shell created
    │       ↓
    │   Shell.getStatus() == ROOT_SHELL
    │   Shell.isRoot() == true
    │
    └── 2. exec("su") → FAILED → exec("sh") → non-root
            ↓
        Shell.getStatus() == NON_ROOT_SHELL
        Shell.isRoot() == false
    │
    ▼
App sekarang bisa:
    ├── Shell.cmd("...").exec()        ← root commands
    ├── RootService.bind(...)          ← root process
    └── Shell.isAppGrantedRoot()       ← cek status
```

### Setup untuk OmniByte

**Opsi A: Gradle Dependency (recommended)**
```kotlin
// gradle/libs.versions.toml
[versions]
libsu = "6.0.0"

[libraries]
libsu-core = { group = "com.github.topjohnwu.libsu", name = "core", version.ref = "libsu" }
libsu-service = { group = "com.github.topjohnwu.libsu", name = "service", version.ref = "libsu" }

// app/build.gradle.kts
dependencies {
    implementation(libs.libsu.core)
    implementation(libs.libsu.service)
}
```

**Opsi B: Source Embed**
```
common/
├── Math/
├── Serialization/
└── libsu/          ← clone atau copy source di sini
    ├── core/
    └── service/
```

**Location**: `common/` directory (sejajar Math/, Serialization/) jika embed source.

---

## 8. Root Strategies: Tiered Approach

### 3-Tier Root Access Strategy

| Tier | Strategy | Requires | Success Rate | Notes |
|------|----------|----------|--------------|-------|
| **1** | Boot Image Patching | Unlocked Bootloader + Root Manager | 99% (device compatible) | Magisk, KernelSU, SukiSU-Ultra |
| **2** | Kernel Exploit | Unlocked Bootloader | Device-specific | shizuku, KernelSU without manager |
| **3** | Virtual Environment | None (app-level) | Limited | Parallel Space, VMOS, VirtualXposed |

### Tier 1: Boot Image Patching (Most Common)
```
User has:
  ├── Unlocked Bootloader (OEM unlock via fastboot)
  ├── Custom Recovery (TWRP) atau Flash via KernelSU app
  └── Root Manager (Magisk/KernelSU/SukiSU-Ultra)

Flash flow:
  boot.img → patch → new boot.img → flash via recovery/app
```

**Supported managers:**
- Magisk (topjohnwu) — most universal
- KernelSU-Next — kernel-level, better hiding
- SukiSU-Ultra — fork with SUSFS, enhanced stealth

### Tier 2: Kernel Exploit
```
User has:
  ├── Unlocked Bootloader (but no root manager installed)
  └── Specific device + kernel version

Exploit:
  kernel vulnerability → gain root → install root manager
```

**Examples:**
- CVE-2021-1048 (efuse) — Samsung devices
- CVE-2023-26083 (GPU) — Qualcomm devices
- Per-device, per-kernel version — not universal

### Tier 3: Virtual Environment (No Root Required)
```
User has:
  ├── Unlocked Bootloader (or even locked)
  └── Virtual environment app

Virtual env:
  Parallel Space / VMOS / VirtualXposed
  → Runs app in isolated container
  → Container has root access (built-in)
  → Target app thinks it's rooted
```

**Limitations:**
- Performance overhead (full VM/container)
- Not all apps work in virtual environment
- Detection by advanced anti-cheat (GameGuardian can detect)

### Locked Bootloader: The Hard Limit

| Scenario | Root Possible? | Notes |
|----------|----------------|-------|
| Unlocked BL + Root Manager | ✅ Yes | Standard approach |
| Unlocked BL + No Root | ✅ Yes | Install root manager |
| Locked BL + Kernel Exploit | ⚠️ Device-specific | Need known CVE |
| Locked BL + No Exploit | ❌ No universal solution | All public exploits patched |
| Fully locked (Samsung Knox, Pixel Titan) | ❌ No | Hardware-backed security |

**Conclusion**: Locked bootloader + no known kernel exploit = **tidak ada solusi universal**.

---

## 9. GameGuardian Mechanics

### Cara Kerja GameGuardian

GameGuardian adalah memory editor untuk game. Memahami mekanismenya membantu OmniByte design anti-detection.

### Root Mode (Standard)

```
┌─────────────────────────────────────────────────────┐
│  GameGuardian Process (root)                        │
│                                                     │
│  1. ptrace(PTRACE_ATTACH, target_pid)              │
│     → Attach ke target game process                 │
│                                                     │
│  2. open("/proc/<pid>/mem", O_RDWR)                │
│     → Buka direct memory access                     │
│                                                     │
│  3. lseek() + read()/write()                        │
│     → Read/write game memory values                 │
│                                                     │
│  4. Search for values (scan memory)                 │
│     → Find game variables (health, gold, etc.)     │
│                                                     │
│  5. Modify values                                   │
│     → Write new values ke memory                    │
└─────────────────────────────────────────────────────┘
```

**Key operations:**
- `ptrace()` — attach ke target process
- `/proc/<pid>/mem` — direct memory access
- `lseek()` + `read()`/`write()` — scan and modify memory

### Non-Root Mode (Virtual Environment)

```
┌─────────────────────────────────────────────────────┐
│  Parallel Space / VMOS / VirtualXposed              │
│                                                     │
│  1. Create virtual container                        │
│     → Isolated Android environment                  │
│                                                     │
│  2. Install both:                                   │
│     ├── GameGuardian (inside container)             │
│     └── Target Game (inside container)              │
│                                                     │
│  3. Container has built-in root                     │
│     → GameGuardian gets root inside container       │
│                                                     │
│  4. Container-level isolation                       │
│     → Real device doesn't see root                  │
│     → Game thinks it's in normal environment        │
└─────────────────────────────────────────────────────┘
```

**Key insight**: Virtual environment provides root **inside the container** without root on real device.

### Detection Vectors

| Vector | How It Works | Bypass Difficulty |
|--------|--------------|-------------------|
| `/proc/self/maps` | Check for suspicious memory mappings | Medium |
| `ptrace` detection | Check if process is being traced | Medium |
| `/proc/<pid>/status` | Check TracerPid field | Medium |
| SELinux context | Check process context | Hard |
| Signature check | Verify file signatures | Hard |
| Kernel module check | Check loaded kernel modules | Very Hard |

### Relevansi untuk OmniByte

GameGuardian pattern shows:
1. **Root mode**: Direct ptrace + `/proc/<pid>/mem` — **what ZigZag backends do**
2. **Non-root mode**: Virtual environment — **alternative strategy for unrooted devices**
3. **Detection vectors**: What anti-cheat looks for — **what ZigZag stealth must bypass**

---

## 10. Virtual Environment Strategy untuk OmniByte

### Concept

Seperti GameGuardian, OmniByte bisa menjalankan operasi runtime di dalam virtual environment untuk device yang tidak rooted.

### Potensial Virtual Environment

| Tool | Type | Root Built-in | Detection Risk | Notes |
|------|------|---------------|----------------|-------|
| **Parallel Space** | App cloner | Yes (in container) | Medium | Popular, but detectable |
| **VMOS** | Full VM | Yes (in VM) | Low | Heavy, full Android in Android |
| **VirtualXposed** | Xposed framework | Yes | Medium | For Xposed modules |
| **DroidSpaces** | Lightweight container | Configurable | Low | Newer, less detection |

### Architecture Option

```
┌─────────────────────────────────────────────────────┐
│  Real Device (unrooted)                             │
│                                                     │
│  ┌─────────────────────────────────────────────┐   │
│  │  Virtual Container (VMOS/DroidSpaces)       │   │
│  │                                             │   │
│  │  ┌─────────────────────────────────────┐   │   │
│  │  │  OmniByte Runtime (inside container)│   │   │
│  │  │                                     │   │   │
│  │  │  ├── ZigZag Manager                 │   │   │
│  │  │  ├── Backend Adapters               │   │   │
│  │  │  ├── ProcessManager                 │   │   │
│  │  │  └── MemoryIO                       │   │   │
│  │  │                                     │   │   │
│  │  │  Root access: BUILT-IN (container)  │   │   │
│  │  └─────────────────────────────────────┘   │   │
│  │                                             │   │
│  │  ┌─────────────────────────────────────┐   │   │
│  │  │  Target App (game/process)          │   │   │
│  │  └─────────────────────────────────────┘   │   │
│  └─────────────────────────────────────────────┘   │
│                                                     │
│  Real device: NO ROOT                               │
│  Container: HAS ROOT                                │
└─────────────────────────────────────────────────────┘
```

### Limitations

1. **Performance overhead** — full VM adds latency
2. **Compatibility** — not all apps run in containers
3. **Detection** — advanced anti-cheat can detect virtual environments
4. **Resource usage** — container consumes significant RAM/CPU

### Decision Matrix

| Scenario | Recommended Strategy |
|----------|---------------------|
| User has rooted device | Use ZigZag with root (standard) |
| User has unrooted + unlocked BL | Suggest boot patching (Tier 1) |
| User has unrooted + locked BL | Virtual environment (Tier 3) — if feasible |
| User wants stealth | ZigZag + SUSFS on rooted device |

---

## 11. OmniByte Integration Strategy

### Root Access Flow

```
app/src/main/java/com/omnibyte/app/
├── RootInitializer.kt          ← Shell.getShell() di splash screen
├── runtime/
│   ├── RootShell.kt            ← wrapper Shell.cmd() untuk memory ops
│   └── HookService.kt          ← RootService untuk native code
```

### Initialization Sequence

```
SplashActivity
    │
    ├── Shell.getShell(callback)
    │       ↓
    │   Shell.Builder.build()
    │       ├── try "su" → root shell
    │       └── fallback "sh" → non-root
    │
    ├── Update UI based on root status
    │   ├── Root: Show full features
    │   └── Non-root: Show limited features + suggest virtual env
    │
    └── Initialize ZigZag Manager
            ├── Load backends based on config
            ├── Set priority order
            └── Ready for stealth operations
```

### Backend Selection Based on Root Status

| Root Status | Available Backends | Recommended |
|-------------|-------------------|-------------|
| Root shell | All (Diamorphine, Bypasser, etc.) | ZigZag with priority |
| Non-root | Virtual environment only | VMOS/DroidSpaces |
| No root, no env | None | Show error/suggestion |

---

**Terakhir diperbarui:** 2026-09-06
**Research Topics Covered:**
1. AnyKernel3
2. WildKernels/GKI_KernelSU_SUSFS
3. Numbersf/Action-Build
4. MMRLApp/DEXMO
5. Root Hiding Frameworks (8 projects)
6. libsu Programmatic Root Access
7. Root Strategies (3-tier approach)
8. GameGuardian Mechanics
9. Virtual Environment Strategy
10. OmniByte Integration Strategy
