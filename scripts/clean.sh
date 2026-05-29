#!/bin/bash
# ============================================================================
# Clean Script - Remove build artifacts
# ============================================================================

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

echo "Cleaning build directory: ${BUILD_DIR}"
rm -rf "${BUILD_DIR}"
rm -f "${PROJECT_DIR}/compile_commands.json"
echo "Done!"
