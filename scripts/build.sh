#!/bin/bash
# ============================================================================
# Build Script
# Usage: ./scripts/build.sh [debug|release]
# ============================================================================

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_TYPE="${1:-Debug}"

# Normalize build type
case "${BUILD_TYPE,,}" in
    debug)   BUILD_TYPE="Debug" ;;
    release) BUILD_TYPE="Release" ;;
    *)       echo "Usage: $0 [debug|release]"; exit 1 ;;
esac

BUILD_DIR="${PROJECT_DIR}/build"

echo "============================================"
echo "  Building: ${BUILD_TYPE}"
echo "  Directory: ${BUILD_DIR}"
echo "============================================"

# Configure
cmake -B "${BUILD_DIR}" -S "${PROJECT_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
cmake --build "${BUILD_DIR}" --parallel "$(nproc)"

# Link compile_commands.json to project root (for IDE support)
ln -sf "${BUILD_DIR}/compile_commands.json" "${PROJECT_DIR}/compile_commands.json" 2>/dev/null || true

echo "============================================"
echo "  Build complete!"
echo "  Binaries: ${BUILD_DIR}/bin/"
echo "============================================"
