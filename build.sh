#!/bin/bash
# Quick build script for CI/CD

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR/module"
OUTPUT_DIR="$SCRIPT_DIR/zygisk"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo "Building SoulBypass..."

# Check for NDK
if [ -z "$ANDROID_NDK" ] && [ -z "$ANDROID_NDK_HOME" ]; then
    echo -e "${RED}Error: ANDROID_NDK not set${NC}"
    echo "Please set ANDROID_NDK to your NDK path"
    exit 1
fi

# Use ANDROID_NDK_HOME if ANDROID_NDK not set
if [ -z "$ANDROID_NDK" ]; then
    ANDROID_NDK="$ANDROID_NDK_HOME"
fi

echo "Using NDK: $ANDROID_NDK"

# Check for Dobby
if [ ! -f "$PROJECT_DIR/src/main/cpp/libs/libdobby.a" ]; then
    echo -e "${YELLOW}Warning: Dobby not found, building without hook support${NC}"
    echo "For full functionality, download Dobby from:"
    echo "https://github.com/jmpews/Dobby/releases"
    # Create dummy file
    mkdir -p "$PROJECT_DIR/src/main/cpp/libs"
    touch "$PROJECT_DIR/src/main/cpp/libs/libdobby.a"
fi

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Build with CMake
BUILD_DIR="$PROJECT_DIR/build"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

echo "Configuring CMake..."
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-29 \
    -DCMAKE_BUILD_TYPE=Release

echo "Building..."
cmake --build . -j$(nproc)

# Copy output
if [ -f "$BUILD_DIR/libsoulbypass.so" ]; then
    cp "$BUILD_DIR/libsoulbypass.so" "$OUTPUT_DIR/arm64-v8a.so"
    echo -e "${GREEN}Build successful: $OUTPUT_DIR/arm64-v8a.so${NC}"
    file "$OUTPUT_DIR/arm64-v8a.so"
    
    # Create module zip
    cd "$SCRIPT_DIR"
    ZIP_NAME="SoulBypass-v1.0.0.zip"
    rm -f "$ZIP_NAME"
    zip -r "$ZIP_NAME" \
        module.prop \
        customize.sh \
        zygisk/ \
        README.md \
        > /dev/null
    
    echo -e "${GREEN}Module package: $ZIP_NAME${NC}"
    ls -lh "$ZIP_NAME"
else
    echo -e "${RED}Build failed - output not found${NC}"
    exit 1
fi
