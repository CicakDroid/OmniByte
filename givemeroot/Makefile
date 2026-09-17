# GiveMeRoot - LKM Rootkit Makefile
# Target: Android GKI 5.15, arm64
# Device: Xiaomi Redmi A5 (serenity)

# Kernel source path - GKI android13-5.15
# Download from: https://android.googlesource.com/kernel/common/ -b android13-5.15
KDIR ?= $(HOME)/android-kernel/common

# Cross-compiler for arm64 Android
# Option 1: Use Android NDK toolchain (recommended)
# Option 2: Use gcc-aarch64-linux-gnu
CROSS_COMPILE ?= aarch64-linux-gnu-

# Architecture
ARCH ?= arm64

# Module name
MODULE_NAME := givemeroot

# Source files
obj-m := $(MODULE_NAME).o

# Build flags
EXTRA_CFLAGS += -D__KERNEL__
EXTRA_CFLAGS += -DMODULE
EXTRA_CFLAGS += -DCONFIG_MODULES=1

.PHONY: all clean install load unload check setup

all: check
	@echo "============================================"
	@echo "Building $(MODULE_NAME) for $(ARCH)"
	@echo "Target: Xiaomi Redmi A5 (serenity)"
	@echo "Kernel: GKI android13-5.15"
	@echo "============================================"
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules
	@echo ""
	@echo "Build complete: $(MODULE_NAME).ko"
	@echo "Transfer to device: adb push $(MODULE_NAME).ko /sdcard/"
	@echo "Load on device: su -c 'insmod /sdcard/$(MODULE_NAME).ko'"

check:
	@if [ ! -d "$(KDIR)" ]; then \
		echo "ERROR: Kernel source not found at $(KDIR)"; \
		echo ""; \
		echo "Setup instructions:"; \
		echo "  make setup"; \
		echo ""; \
		echo "Or manually:"; \
		echo "  git clone https://android.googlesource.com/kernel/common/ -b android13-5.15 ~/android-kernel/common"; \
		exit 1; \
	fi

setup:
	@echo "Setting up GKI kernel source..."
	@echo ""
	@echo "Step 1: Clone GKI kernel"
	git clone https://android.googlesource.com/kernel/common/ -b android13-5.15 $(HOME)/android-kernel/common
	@echo ""
	@echo "Step 2: Install cross-compiler"
	@echo "  Ubuntu/Debian: sudo apt install gcc-aarch64-linux-gnu"
	@echo "  macOS: brew install aarch64-elf-gcc"
	@echo "  Or use Android NDK"
	@echo ""
	@echo "Step 3: Build this module"
	@echo "  make"
	@echo ""
	@echo "Setup complete!"

clean:
	@if [ -d "$(KDIR)" ]; then \
		$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean; \
	fi
	rm -f *.o *.ko *.mod.c *.mod *.mod.o modules.order Module.symvers .*.cmd

# Load module (requires root on device)
load: check
	@echo "Loading $(MODULE_NAME)..."
	sudo insmod $(MODULE_NAME).ko
	@echo "Module loaded. Use 'kill -63 0' for root"

# Unload module (requires root on device)
unload:
	@echo "Unloading $(MODULE_NAME)..."
	sudo rmmod $(MODULE_NAME)
	@echo "Module unloaded"

# Deploy to device via ADB
deploy:
	@echo "Deploying to device..."
	adb push $(MODULE_NAME).ko /sdcard/
	@echo "Deployed to /sdcard/$(MODULE_NAME).ko"
	@echo "Load on device: su -c 'insmod /sdcard/$(MODULE_NAME).ko'"

# Load on device via ADB
load-device:
	@echo "Loading module on device..."
	adb shell su -c "insmod /sdcard/$(MODULE_NAME).ko"
	@echo "Module loaded on device"

# Check module status
status:
	@echo "Module status:"
	@lsmod | grep $(MODULE_NAME) || echo "Module not loaded"

# Help
help:
	@echo "GiveMeRoot - Simple LKM Rootkit"
	@echo ""
	@echo "Targets:"
	@echo "  all         - Build the kernel module"
	@echo "  setup       - Setup GKI kernel source"
	@echo "  clean       - Clean build files"
	@echo "  deploy      - Deploy to device via ADB"
	@echo "  load-device - Load module on device via ADB"
	@echo "  load        - Load module locally (for testing)"
	@echo "  unload      - Unload module locally"
	@echo "  status      - Check if module is loaded"
	@echo "  help        - Show this help"
	@echo ""
	@echo "Configuration:"
	@echo "  KDIR          - Path to kernel source (default: ~/android-kernel/common)"
	@echo "  CROSS_COMPILE - Cross-compiler prefix (default: aarch64-linux-gnu-)"
	@echo "  ARCH          - Target architecture (default: arm64)"
	@echo ""
	@echo "Device Info:"
	@echo "  Model:    Xiaomi Redmi A5 / POCO C71"
	@echo "  Codename: serenity"
	@echo "  SoC:      Unisoc T7250 (T606)"
	@echo "  Kernel:   5.15.178-android13-8-00006-g0c6055fd2d8b-ab13363910"
	@echo "  Android:  15 (Go Edition)"
	@echo "  Build:    A15.0.11.0.VGWIDXM"
	@echo ""
	@echo "Usage on Device:"
	@echo "  kill -63 0  # Grant root"
	@echo "  kill -62 0  # Toggle module visibility"
