#!/bin/bash
# GiveMeRoot Build Script
# Target: Xiaomi Redmi A5 (serenity)
# Kernel: GKI android13-5.15

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
MODULE_NAME="givemeroot"
KDIR="${HOME}/android-kernel/common"
CROSS_COMPILE="aarch64-linux-gnu-"
ARCH="arm64"
DEVICE_KERNEL="5.15.178-android13-8-00006-g0c6055fd2d8b-ab13363910"

echo -e "${GREEN}============================================${NC}"
echo -e "${GREEN}GiveMeRoot Build Script${NC}"
echo -e "${GREEN}Target: Xiaomi Redmi A5 (serenity)${NC}"
echo -e "${GREEN}============================================${NC}"
echo ""

# Function to check if command exists
check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo -e "${RED}Error: $1 not found${NC}"
        return 1
    fi
    return 0
}

# Function to setup kernel source
setup_kernel() {
    echo -e "${YELLOW}Step 1: Setting up GKI kernel source...${NC}"
    
    if [ -d "$KDIR" ]; then
        echo -e "${GREEN}Kernel source already exists at $KDIR${NC}"
        return 0
    fi
    
    echo "Cloning GKI kernel android13-5.15..."
    git clone https://android.googlesource.com/kernel/common/ -b android13-5.15 "$KDIR"
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to clone kernel source${NC}"
        return 1
    fi
    
    echo -e "${GREEN}Kernel source cloned successfully${NC}"
    return 0
}

# Function to check dependencies
check_dependencies() {
    echo -e "${YELLOW}Checking dependencies...${NC}"
    
    local missing=0
    
    if ! check_command "make"; then
        missing=1
    fi
    
    if ! check_command "${CROSS_COMPILE}gcc"; then
        echo -e "${RED}Cross-compiler not found: ${CROSS_COMPILE}gcc${NC}"
        echo "Install with: sudo apt install gcc-aarch64-linux-gnu"
        missing=1
    fi
    
    if ! check_command "git"; then
        echo -e "${RED}git not found${NC}"
        missing=1
    fi
    
    if [ $missing -eq 1 ]; then
        echo ""
        echo "Install missing dependencies:"
        echo "  Ubuntu/Debian: sudo apt install build-essential gcc-aarch64-linux-gnu git"
        return 1
    fi
    
    echo -e "${GREEN}All dependencies found${NC}"
    return 0
}

# Function to build module
build_module() {
    echo -e "${YELLOW}Step 2: Building module...${NC}"
    
    # Check if kernel source exists
    if [ ! -d "$KDIR" ]; then
        echo -e "${RED}Kernel source not found at $KDIR${NC}"
        echo "Run: make setup"
        return 1
    fi
    
    # Build module
    make KDIR="$KDIR" ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE"
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Build failed${NC}"
        return 1
    fi
    
    echo -e "${GREEN}Module built successfully: ${MODULE_NAME}.ko${NC}"
    return 0
}

# Function to deploy to device
deploy_device() {
    echo -e "${YELLOW}Step 3: Deploying to device...${NC}"
    
    # Check if ADB is available
    if ! check_command "adb"; then
        echo -e "${RED}ADB not found${NC}"
        echo "Install Android SDK Platform Tools"
        return 1
    fi
    
    # Check if device is connected
    adb devices | grep -q "device$"
    if [ $? -ne 0 ]; then
        echo -e "${RED}No device connected${NC}"
        echo "Connect device via USB and enable USB debugging"
        return 1
    fi
    
    # Push module to device
    adb push "${MODULE_NAME}.ko" /sdcard/
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to deploy module${NC}"
        return 1
    fi
    
    echo -e "${GREEN}Module deployed to /sdcard/${MODULE_NAME}.ko${NC}"
    return 0
}

# Function to load module on device
load_device() {
    echo -e "${YELLOW}Step 4: Loading module on device...${NC}"
    
    # Check if ADB is available
    if ! check_command "adb"; then
        echo -e "${RED}ADB not found${NC}"
        return 1
    fi
    
    # Load module
    adb shell su -c "insmod /sdcard/${MODULE_NAME}.ko"
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to load module${NC}"
        echo "Make sure:"
        echo "  1. Device is rooted"
        echo "  2. SELinux is permissive: adb shell su -c 'setenforce 0'"
        return 1
    fi
    
    echo -e "${GREEN}Module loaded successfully!${NC}"
    echo ""
    echo "Usage:"
    echo "  kill -63 0  # Grant root"
    echo "  kill -62 0  # Toggle hide"
    return 0
}

# Main function
main() {
    # Parse arguments
    case "${1:-all}" in
        setup)
            setup_kernel
            ;;
        check)
            check_dependencies
            ;;
        build)
            check_dependencies && build_module
            ;;
        deploy)
            deploy_device
            ;;
        load)
            load_device
            ;;
        all)
            check_dependencies && setup_kernel && build_module && deploy_device && load_device
            ;;
        help|--help|-h)
            echo "Usage: $0 [command]"
            echo ""
            echo "Commands:"
            echo "  setup   - Setup GKI kernel source"
            echo "  check   - Check dependencies"
            echo "  build   - Build module"
            echo "  deploy  - Deploy to device via ADB"
            echo "  load    - Load module on device"
            echo "  all     - Run all steps (default)"
            echo "  help    - Show this help"
            echo ""
            echo "Examples:"
            echo "  $0           # Run all steps"
            echo "  $0 setup     # Setup kernel source only"
            echo "  $0 build     # Build module only"
            echo "  $0 deploy    # Deploy to device"
            echo "  $0 load      # Load module on device"
            ;;
        *)
            echo -e "${RED}Unknown command: $1${NC}"
            echo "Run: $0 help"
            exit 1
            ;;
    esac
}

# Run main function
main "$@"
