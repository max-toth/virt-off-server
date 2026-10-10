#!/usr/bin/env bash
#
# Multiplayer integration test for virt-off-server (V2 API).
#
# Usage: integration_test.sh <server-binary> <test-client-binary>
#
# Starts a real v2 server, connects a listener and a sender, and asserts that:
#   - both clients get a "player-..." id assigned;
#   - the listener receives the sender's relayed voice packet;
#   - the sender does NOT receive its own relayed packet.
#
# No framework: plain checks, non-zero exit on failure so CTest can report it.
set -u

SERVER_BIN="${1:?usage: integration_test.sh <server-binary> <test-client-binary>}"
CLIENT_BIN="${2:?usage: integration_test.sh <server-binary> <test-client-binary>}"

readonly PORT=39221
readonly ID_PREFIX="CONNECTED player-"
readonly VOICE_LINE="RECV VOICE hello-voice"

FAILURES=0
WORK_DIR="$(mktemp -d)"
SERVER_PID=""

# Stops the server (if running) and removes the work directory.
cleanup() {
    if [ -n "$SERVER_PID" ]; then
        kill -TERM "$SERVER_PID" 2>/dev/null
        wait "$SERVER_PID" 2>/dev/null
    fi
    rm -rf "$WORK_DIR"
}
trap cleanup EXIT

# Asserts that a log file contains the expected substring.
check_contains() {
    local log_file="$1"
    local needle="$2"
    local label="$3"

    if grep -qF -- "$needle" "$log_file"; then
        echo "PASS: $label"
    else
        echo "FAIL: $label (missing: '$needle')"
        FAILURES=$((FAILURES + 1))
    fi
}

# Asserts that a log file does NOT contain the given substring.
check_absent() {
    local log_file="$1"
    local needle="$2"
    local label="$3"

    if grep -qF -- "$needle" "$log_file"; then
        echo "FAIL: $label (unexpected: '$needle')"
        FAILURES=$((FAILURES + 1))
    else
        echo "PASS: $label"
    fi
}

"$SERVER_BIN" --port "$PORT" --threads 2 --api v2 >"$WORK_DIR/server.log" 2>&1 &
SERVER_PID=$!
sleep 1

"$CLIENT_BIN" --port "$PORT" --payload hello-voice --listen 4 >"$WORK_DIR/listener.log" 2>&1 &
LISTENER_PID=$!
sleep 1

"$CLIENT_BIN" --port "$PORT" --send voice --payload hello-voice --listen 2 >"$WORK_DIR/sender.log" 2>&1 &
SENDER_PID=$!

wait "$LISTENER_PID"
wait "$SENDER_PID"

check_contains "$WORK_DIR/listener.log" "$ID_PREFIX" "listener: assigned id"
check_contains "$WORK_DIR/listener.log" "$VOICE_LINE" "listener: received relayed voice"
check_contains "$WORK_DIR/sender.log" "$ID_PREFIX" "sender: assigned id"
check_absent "$WORK_DIR/sender.log" "$VOICE_LINE" "sender: did not receive its own voice"

if [ "$FAILURES" -eq 0 ]; then
    echo "ALL INTEGRATION TESTS PASSED"
    exit 0
fi

echo "$FAILURES CHECK(S) FAILED"
exit 1
