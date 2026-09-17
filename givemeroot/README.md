# GiveMeRoot - LKM Rootkit untuk Xiaomi Redmi A5

LKM (Loadable Kernel Module) rootkit berbasis [Diamorphine](https://github.com/m0r13n/Diamorphine).

## Device Info

| Parameter | Value |
|-----------|-------|
| Model | Xiaomi Redmi A5 / POCO C71 |
| Codename | serenity |
| SoC | Unisoc T7250 (T606) |
| Architecture | arm64 |
| Kernel | 5.15.178-android13-8-00006-g0c6055fd2d8b-ab13363910 |
| Android | 15 (Go Edition) |
| Build | A15.0.11.0.VGWIDXM |

## Features

| Signal | Function |
|--------|----------|
| `kill -63 0` | Grant root (UID=0, GID=0) |
| `kill -62 0` | Toggle module visibility in lsmod |

## Prerequisites

### 1. Linux Environment

Build harus dilakukan di **Linux** (Ubuntu/Debian recommended). Tidak bisa di Windows/macOS.

### 2. Cross-Compiler

```bash
# Ubuntu/Debian
sudo apt install gcc-aarch64-linux-gnu

# Atau gunakan Android NDK
```

### 3. GKI Kernel Source

```bash
# Clone GKI kernel android13-5.15
git clone https://android.googlesource.com/kernel/common/ -b android13-5.15 ~/android-kernel/common
```

## Quick Start

### Step 1: Setup (hanya sekali)

```bash
cd /root/OmniByte/givemeroot
make setup
```

### Step 2: Build

```bash
make
```

### Step 3: Deploy ke Device

```bash
make deploy
```

### Step 4: Load di Device

```bash
# Via ADB shell
adb shell
su
insmod /sdcard/givemeroot.ko

# Atau
adb shell su -c "insmod /sdcard/givemeroot.ko"
```

### Step 5: Gunakan

```bash
# Dari shell biasa (setelah insmod)
kill -63 0
id
# uid=0(root) gid=0(root)

# Sembunyikan module
kill -62 0
```

## Manual Build Instructions

### Step 1: Install Dependencies

```bash
# Ubuntu/Debian
sudo apt install build-essential gcc-aarch64-linux-gnu bc bison flex libssl-dev libelf-dev

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install gcc-aarch64-linux-gnu bc bison flex openssl-devel elfutils-libelf-devel
```

### Step 2: Download GKI Kernel

```bash
# Clone kernel source
git clone https://android.googlesource.com/kernel/common/ -b android13-5.15 ~/android-kernel/common

# Atau download dari:
# https://android.googlesource.com/kernel/common/+refs
```

### Step 3: Build Module

```bash
cd /root/OmniByte/givemeroot

# Set environment
export KDIR=~/android-kernel/common
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

# Build
make
```

### Step 4: Transfer ke Device

```bash
# Via ADB
adb push givemeroot.ko /sdcard/

# Atau via Termux
# Copy file ke /sdcard/ using file manager
```

### Step 5: Load Module

```bash
# Via ADB shell
adb shell
su
insmod /sdcard/givemeroot.ko

# Via Termux (jika sudah root)
su
insmod /sdcard/givemeroot.ko
```

## Troubleshooting

### Error: "Invalid module format"

**Penyebab:** Kernel version mismatch

**Solusi:**
```bash
# Cek kernel version di device
adb shell uname -r
# Output: 5.15.178-android13-8-00006-g0c6055fd2d8b-ab13363910

# Pastikan kernel source version match
# GKI android13-5.15 seharusnya compatible
```

### Error: "Operation not permitted"

**Penyebab:** Tidak ada root atau SELinux enforcement

**Solusi:**
```bash
# Set SELinux permissive (temporer)
adb shell su -c "setenforce 0"

# Atau build dengan module signature disabled
```

### Error: "Required key not available"

**Penyebab:** Module signature verification aktif

**Solusi:**
```bash
# Disable module signing di kernel config
# Atau sign module dengan key yang sama
```

### Error: "Cannot find sys_call_table"

**Penyebab:** Kernel config berbeda

**Solusi:**
```bash
# Cek apakah sys_call_table ada
adb shell cat /proc/kallsyms | grep sys_call_table

# Jika tidak ada, kernel mungkin menggunakan syscall filter
```

## Usage Examples

### Grant Root

```bash
# Cek status awal
$ id
uid=1001(shell) gid=1001(shell) groups=1001(shell)

# Grant root
$ kill -63 0

# Verifikasi
$ id
uid=0(root) gid=0(root) groups=0(root),100(shell),1001(shell)

# Buka root shell
$ su
# whoami
root
```

### Hide Module

```bash
# Cek module status
$ lsmod | grep givemeroot
givemeroot    16384    0

# Sembunyikan
$ kill -62 0

# Verifikasi tersembunyi
$ lsmod | grep givemeroot
(hanya tidak muncul)

# Tampilkan kembali
$ kill -62 0
```

## Security Notice

⚠️ **Tool ini untuk authorized security testing only.**

- Gunakan hanya pada device yang kamu milik atau memiliki izin resmi
- Gunakan untuk penetrasi testing, bug bounty, atau edukasi
- Jangan gunakan untuk aktivitas ilegal

## Known Issues

1. **Kernel Source Tidak Tersedia** - Xiaomi belum merilis kernel source untuk Redmi A5. GKI common kernel digunakan sebagai base.

2. **Driver Unisoc T606** - Beberapa driver mungkin tidak tersedia di GKI common kernel.

3. **Module Signature** - GKI kernels mungkin memerlukan module signing.

## References

- [Diamorphine](https://github.com/m0r13n/Diamorphine) - Original rootkit
- [Android GKI](https://source.android.com/docs/core/architecture/kernel/generic-kernel-image) - Generic Kernel Image
- [Xiaomi Kernel Source Request](https://github.com/MiCode/Xiaomi_Kernel_OpenSource/issues/40999) - Open issue

## License

GPL-2.0
