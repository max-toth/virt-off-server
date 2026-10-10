#!/usr/bin/env bash
#
# Smoke tests for the virt-off-server executable.
#
# Usage: smoke_server_test.sh <path-to-virt-off-server>
#
# Each check runs the real binary as a subprocess on a dedicated UDP port,
# waits for startup, sends SIGTERM and verifies a clean exit (code 0) plus
# expected startup output. No test framework: plain checks, exit code signals
# failure so CTest can pick the result up.
set -u

BIN="${1:?usage: smoke_server_test.sh <path-to-virt-off-server>}"

# Dedicated ports so tests never clash with a dev server on the default 7777.
readonly V1_PORT=39217
readonly V2_PORT=39218
readonly ARGS_PORT=39219

FAILURES=0
WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT

# Starts BIN with the given args, waits for startup, sends SIGTERM and waits
# for termination. stdout/stderr are captured into $1; the exit code is
# returned so the caller can assert a clean shutdown.
run_and_stop() {
    local log_file="$1"
    shift

    "$BIN" "$@" >"$log_file" 2>&1 &
    local pid=$!

    sleep 1
    kill -TERM "$pid" 2>/dev/null
    wait "$pid"
}

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

# Asserts that a process exited cleanly (code 0).
check_clean_exit() {
    local code="$1"
    local label="$2"

    if [ "$code" -eq 0 ]; then
        echo "PASS: $label"
    else
        echo "FAIL: $label (exit=$code)"
        FAILURES=$((FAILURES + 1))
    fi
}

run_and_stop "$WORK_DIR/v1.log" --port "$V1_PORT" --threads 2 --api v1
check_clean_exit $? "v1: clean shutdown on SIGTERM"
check_contains "$WORK_DIR/v1.log" "Server is running" "v1: reached running state"

run_and_stop "$WORK_DIR/v2.log" --port "$V2_PORT" --threads 2 --api v2
check_clean_exit $? "v2: clean shutdown on SIGTERM"
check_contains "$WORK_DIR/v2.log" "Server (v2) is running" "v2: reached running state"

run_and_stop "$WORK_DIR/args.log" --bogus --port "$ARGS_PORT" --api v2
check_clean_exit $? "args: clean shutdown on SIGTERM"
check_contains "$WORK_DIR/args.log" "Ignoring unknown argument: --bogus" "args: unknown flag reported"

if [ "$FAILURES" -eq 0 ]; then
    echo "ALL SMOKE TESTS PASSED"
    exit 0
fi

echo "$FAILURES CHECK(S) FAILED"
exit 1
