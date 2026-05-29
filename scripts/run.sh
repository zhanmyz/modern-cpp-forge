#!/bin/bash
# ============================================================================
# Run Script - Execute a specified binary
# Usage: ./scripts/run.sh <target_name>
# Example: ./scripts/run.sh ref_concurrency_thread_basics
# ============================================================================

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

if [ -z "$1" ]; then
    echo "Usage: $0 <target_name>"
    echo ""
    echo "Available targets:"
    find "${BUILD_DIR}/bin" -type f -executable 2>/dev/null | sort | while read -r f; do
        echo "  $(basename "$f")"
    done
    exit 1
fi

TARGET="$1"
BINARY=$(find "${BUILD_DIR}/bin" -name "${TARGET}" -type f -executable 2>/dev/null | head -1)

if [ -z "${BINARY}" ]; then
    echo "Error: Target '${TARGET}' not found."
    echo "Did you build first? Run: ./scripts/build.sh"
    exit 1
fi

echo "Running: ${BINARY}"
echo "============================================"
exec "${BINARY}"
