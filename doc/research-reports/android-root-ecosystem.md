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

**Terakhir diperbarui:** 2026-09-06
