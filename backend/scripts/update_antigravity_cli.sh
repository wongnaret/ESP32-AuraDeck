#!/bin/bash
# AuraDeck Antigravity CLI Status Updater Script
# This script executes the Antigravity CLI on the Host machine / Raspberry Pi
# and saves the JSON status directly to backend/tokens/antigravity_data.json
# so that the containerized backend reads real-time host quota & credit metrics.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOKENS_DIR="$(cd "$SCRIPT_DIR/../tokens" 2>/dev/null && pwd || echo "$SCRIPT_DIR/../tokens")"
mkdir -p "$TOKENS_DIR"

OUTPUT_FILE="$TOKENS_DIR/antigravity_data.json"

# Check if agy CLI exists on host
if command -v agy &> /dev/null; then
    agy status --json > "$OUTPUT_FILE.tmp" 2>/dev/null
    if [ $? -eq 0 ] && [ -s "$OUTPUT_FILE.tmp" ]; then
        mv "$OUTPUT_FILE.tmp" "$OUTPUT_FILE"
        echo "[$(date)] Successfully updated Antigravity CLI status -> $OUTPUT_FILE"
        exit 0
    else
        rm -f "$OUTPUT_FILE.tmp"
        echo "[$(date)] Error: 'agy status --json' failed or returned empty output."
        exit 1
    fi
else
    echo "[$(date)] Warning: 'agy' CLI command not found on host. Please install or add to PATH."
    exit 1
fi
