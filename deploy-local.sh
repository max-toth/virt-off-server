#!/usr/bin/env bash
set -euo pipefail

# ============================================================
# virt-off-server local deploy script
#
# Builds an x86_64 binary on the current machine (arm64 → x86_64
# cross-compile via Apple Clang) and deploys it to a target macOS
# host via rsync+ssh. The server is started as a background process.
#
# Prerequisites:
#   - CMake + Xcode Command Line Tools installed locally
#
# Usage:
#   ./deploy-local.sh [port]
#
# On first run you may need to accept the host key:
#   ssh max_tolstykh@192.168.1.111 "echo ok"
#
# Environment:
#   REMOTE_USER   — remote username (default: max_tolstykh)
#   REMOTE_HOST   — remote host     (default: 192.168.1.111)
#   REMOTE_DIR    — remote directory (default: ~/virt-off-server)
#   PORT          — UDP port        (default: first arg or 7777)
#   BINARY_NAME   — binary name     (default: virt-off-server-x86_64)
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BINARY_NAME="${BINARY_NAME:-virt-off-server-x86_64}"
PORT="${1:-7777}"

REMOTE_USER="${REMOTE_USER:-max_tolstykh}"
REMOTE_HOST="${REMOTE_HOST:-192.168.1.111}"
REMOTE_DIR="${REMOTE_DIR:-virt-off-server}"

REMOTE="${REMOTE_USER}@${REMOTE_HOST}"

SSH_OPTS=(-o StrictHostKeyChecking=no -o ConnectTimeout=10)

# ---- Step 1: Build x86_64 binary ----
echo "=== Building ${BINARY_NAME} for x86_64 ==="
cmake -B "${SCRIPT_DIR}/build-x86_64" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES=x86_64 \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
    -S "$SCRIPT_DIR" \
    --log-level=ERROR

cmake --build "${SCRIPT_DIR}/build-x86_64" --parallel 2>&1

# Rename: CMake outputs virt-off-server, but we want x86_64 in the name
mv "${SCRIPT_DIR}/build-x86_64/virt-off-server" "${SCRIPT_DIR}/build-x86_64/${BINARY_NAME}"

BINARY="${SCRIPT_DIR}/build-x86_64/${BINARY_NAME}"
echo "  Binary: $(file "$BINARY" | sed 's/.*: //')"

# ---- Step 2: Create remote dir ----
echo "=== Preparing remote ==="
ssh "${SSH_OPTS[@]}" "$REMOTE" "mkdir -p \"$REMOTE_DIR\""

# ---- Step 3: Copy binary ----
echo "=== Deploying ==="
rsync -az --no-perms "$BINARY" "${REMOTE}:${REMOTE_DIR}/${BINARY_NAME}"

# ---- Step 4: Kill old process, start new one ----
echo "=== Starting on remote host:${PORT} ==="
ssh "${SSH_OPTS[@]}" "$REMOTE" << REMOTE_CMDS
    set -e
    BIN="${REMOTE_DIR}/${BINARY_NAME}"

    # Kill existing instance
    if pgrep -x "${BINARY_NAME}" >/dev/null 2>&1; then
        echo "  Stopping existing ${BINARY_NAME}..."
        pkill -x "${BINARY_NAME}" 2>/dev/null || true
        sleep 1
    fi

    chmod +x "\$BIN"

    echo "  PID: \$BIN"
    nohup "\$BIN" --port "${PORT}" > ~/virt-off-server.log 2>&1 &
    echo "  Started (PID \$!) on port ${PORT}"
REMOTE_CMDS

echo "=== Done ==="
