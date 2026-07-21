#!/usr/bin/env bash
# Runs the same checks CI would run: clean build with zero warnings/errors,
# then the full test suite. Exits non-zero on the first failure so it can be
# used as a pre-push gate. Mirrors CLAUDE.md's non-negotiable rules #1/#2.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

BUILD_DIR="${1:-build}"

echo "==> Configuring ($BUILD_DIR)"
cmake -S . -B "$BUILD_DIR" >/dev/null

echo "==> Building (checking for zero warnings/errors)"
BUILD_LOG="$(mktemp)"
trap 'rm -f "$BUILD_LOG"' EXIT

if ! cmake --build "$BUILD_DIR" --parallel "$(nproc)" > "$BUILD_LOG" 2>&1; then
    echo "FAIL: build failed" >&2
    tail -60 "$BUILD_LOG" >&2
    exit 1
fi

WARNING_COUNT="$(grep -c "warning:" "$BUILD_LOG" || true)"
ERROR_COUNT="$(grep -c "error:" "$BUILD_LOG" || true)"

if [ "$WARNING_COUNT" -ne 0 ] || [ "$ERROR_COUNT" -ne 0 ]; then
    echo "FAIL: build produced $WARNING_COUNT warning(s) and $ERROR_COUNT error(s)" >&2
    grep -E "warning:|error:" "$BUILD_LOG" >&2
    exit 1
fi
echo "    build clean: 0 warnings, 0 errors"

echo "==> Running tests"
TEST_BIN="$BUILD_DIR/SharpRuntimeTests"
if [ ! -x "$TEST_BIN" ]; then
    echo "FAIL: test binary not found at $TEST_BIN" >&2
    exit 1
fi

TEST_LOG="$(mktemp)"
trap 'rm -f "$BUILD_LOG" "$TEST_LOG"' EXIT

if ! "$TEST_BIN" > "$TEST_LOG" 2>&1; then
    echo "FAIL: test suite failed" >&2
    grep -E "FAILED|\[  FAILED  \]" "$TEST_LOG" >&2 || true
    tail -40 "$TEST_LOG" >&2
    exit 1
fi

SUMMARY_LINE="$(grep -E "^\[==========\] .* ran\." "$TEST_LOG" | tail -1)"
echo "    $SUMMARY_LINE"
echo "    all tests passed"

echo "==> Local CI check passed"
