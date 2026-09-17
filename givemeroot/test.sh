#!/bin/bash
# GiveMeRoot Test Script
# Test build environment and device connectivity

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}============================================${NC}"
echo -e "${GREEN}GiveMeRoot Test Script${NC}"
echo -e "${GREEN}============================================${NC}"
echo ""

# Test 1: Check if module file exists
echo -e "${YELLOW}Test 1: Checking module file...${NC}"
if [ -f "givemeroot.ko" ]; then
    echo -e "${GREEN}Module file exists: givemeroot.ko${NC}"
    file givemeroot.ko
else
    echo -e "${RED}Module file not found${NC}"
    echo "Build with: make"
fi
echo ""

# Test 2: Check kernel info in module
echo -e "${YELLOW}Test 2: Checking module kernel info...${NC}"
if [ -f "givemeroot.ko" ]; then
    modinfo givemeroot.ko 2>/dev/null || echo "modinfo not available"
else
    echo -e "${RED}Module file not found${NC}"
fi
echo ""

# Test 3: Check ADB connection
echo -e "${YELLOW}Test 3: Checking ADB connection...${NC}"
if command -v adb &> /dev/null; then
    echo "ADB found: $(which adb)"
    
    # Check if device is connected
    DEVICE_COUNT=$(adb devices | grep -c "device$")
    if [ "$DEVICE_COUNT" -gt 0 ]; then
        echo -e "${GREEN}Device connected${NC}"
        echo "Device info:"
        adb shell getprop ro.product.model
        adb shell getprop ro.build.display.id
        adb shell uname -r
    else
        echo -e "${RED}No device connected${NC}"
        echo "Connect device via USB and enable USB debugging"
    fi
else
    echo -e "${RED}ADB not found${NC}"
    echo "Install Android SDK Platform Tools"
fi
echo ""

# Test 4: Check device kernel version
echo -e "${YELLOW}Test 4: Checking device kernel version...${NC}"
if command -v adb &> /dev/null; then
    KERNEL_VERSION=$(adb shell uname -r 2>/dev/null || echo "N/A")
    echo "Device kernel: $KERNEL_VERSION"
    
    # Check if it matches expected
    EXPECTED_KERNEL="5.15.178-android13-8-00006-g0c6055fd2d8b-ab13363910"
    if [ "$KERNEL_VERSION" = "$EXPECTED_KERNEL" ]; then
        echo -e "${GREEN}Kernel version matches${NC}"
    else
        echo -e "${YELLOW}Kernel version mismatch${NC}"
        echo "Expected: $EXPECTED_KERNEL"
        echo "Got: $KERNEL_VERSION"
        echo "Module may still work with GKI kernel"
    fi
else
    echo -e "${RED}ADB not available${NC}"
fi
echo ""

# Test 5: Check if module is already loaded
echo -e "${YELLOW}Test 5: Checking if module is loaded...${NC}"
if command -v adb &> /dev/null; then
    MODULE_LOADED=$(adb shell "lsmod | grep givemeroot" 2>/dev/null || echo "")
    if [ -z "$MODULE_LOADED" ]; then
        echo -e "${YELLOW}Module not loaded${NC}"
    else
        echo -e "${GREEN}Module is loaded${NC}"
        echo "$MODULE_LOADED"
    fi
else
    echo -e "${RED}ADB not available${NC}"
fi
echo ""

# Test 6: Check root access
echo -e "${YELLOW}Test 6: Checking root access...${NC}"
if command -v adb &> /dev/null; then
    ROOT_STATUS=$(adb shell "id" 2>/dev/null || echo "N/A")
    echo "Current user: $ROOT_STATUS"
    
    # Check if we can get root
    ROOT_CHECK=$(adb shell "su -c 'id'" 2>/dev/null || echo "N/A")
    if echo "$ROOT_CHECK" | grep -q "uid=0"; then
        echo -e "${GREEN}Root access available${NC}"
    else
        echo -e "${RED}Root access not available${NC}"
        echo "Device must be rooted to load kernel modules"
    fi
else
    echo -e "${RED}ADB not available${NC}"
fi
echo ""

# Summary
echo -e "${GREEN}============================================${NC}"
echo -e "${GREEN}Test Summary${NC}"
echo -e "${GREEN}============================================${NC}"
echo ""
echo "If all tests passed, you can deploy the module:"
echo "  make deploy"
echo "  make load-device"
echo ""
echo "Or use the build script:"
echo "  ./build.sh deploy"
echo "  ./build.sh load"
echo ""
