#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

cd "$SCRIPT_DIR"

echo "=== Configuring ==="
cmake -B build -DCMAKE_BUILD_TYPE=Release

echo "=== Building ==="
cmake --build build --parallel

echo "=== Starting server on port ${1:-7777} ==="
exec ./build/virt-off-server "${1:-7777}"
