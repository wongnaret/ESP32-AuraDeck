#!/bin/bash
# AuraDeck Antigravity CLI Zero-Quota Status Refresher
# Triggers the `/usage` panel inside a detached GNU `screen` session running `agy`,
# waits for `agy` to refresh quota and invoke `cyberdeck_statusline.py`, then presses
# `Esc` to close the panel — consuming ZERO LLM tokens, model quota, or AI credits.

export PATH="$HOME/.local/bin:/usr/local/bin:/usr/bin:/bin:$PATH"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
TOKENS_DIR="$REPO_DIR/backend/tokens"
OUTPUT_FILE="$TOKENS_DIR/antigravity_data.json"

SCREEN_NAME="${AGY_SCREEN_NAME:-agy}"
AGY_BIN="${AGY_BIN:-$(command -v agy 2>/dev/null || echo "$HOME/.local/bin/agy")}"
SCREEN_BIN="${SCREEN_BIN:-$(command -v screen 2>/dev/null || echo "/usr/bin/screen")}"
USAGE_WAIT_SECS="${AGY_USAGE_WAIT_SECS:-5}"
BOOT_WAIT_SECS="${AGY_BOOT_WAIT_SECS:-10}"

mkdir -p "$TOKENS_DIR"

if [ ! -x "$SCREEN_BIN" ]; then
    echo "[$(date)] Error: 'screen' binary not found at $SCREEN_BIN."
    exit 1
fi

if [ ! -x "$AGY_BIN" ]; then
    echo "[$(date)] Error: 'agy' binary not found at $AGY_BIN."
    exit 1
fi

get_json_timestamp() {
    if [ -f "$OUTPUT_FILE" ]; then
        grep -oE '"timestamp"[[:space:]]*:[[:space:]]*[0-9]+' "$OUTPUT_FILE" 2>/dev/null | head -n 1 | grep -oE '[0-9]+'
    fi
}

BEFORE_TS="$(get_json_timestamp)"

# Ensure the detached screen session running `agy` exists
if ! "$SCREEN_BIN" -list 2>/dev/null | grep -qE "[0-9]+\.${SCREEN_NAME}[[:space:]]"; then
    echo "[$(date)] Starting detached screen session '${SCREEN_NAME}' with ${AGY_BIN}..."
    "$SCREEN_BIN" -dmS "$SCREEN_NAME" bash -c "cd \"$REPO_DIR\" && \"$AGY_BIN\""
    sleep "$BOOT_WAIT_SECS"
fi

echo "[$(date)] Refreshing Antigravity quota via /usage (zero-quota) in screen '${SCREEN_NAME}'..."

# 1. Dismiss any open modal and clear any stray input
"$SCREEN_BIN" -S "$SCREEN_NAME" -p 0 -X stuff $'\033'
sleep 0.5
"$SCREEN_BIN" -S "$SCREEN_NAME" -p 0 -X stuff $'\025'
sleep 0.2

# 2. Open /usage panel to trigger quota refresh & statusLine bridge
"$SCREEN_BIN" -S "$SCREEN_NAME" -p 0 -X stuff $'/usage\r'
sleep "$USAGE_WAIT_SECS"

# 3. Press Esc to close the /usage panel cleanly
"$SCREEN_BIN" -S "$SCREEN_NAME" -p 0 -X stuff $'\033'

AFTER_TS="$(get_json_timestamp)"
if [ -n "$AFTER_TS" ] && [ "$AFTER_TS" != "$BEFORE_TS" ]; then
    echo "[$(date)] Quota refreshed successfully (timestamp: ${BEFORE_TS:-none} -> ${AFTER_TS})."
else
    echo "[$(date)] Sent /usage + Esc (timestamp unchanged: ${AFTER_TS:-none})."
fi
