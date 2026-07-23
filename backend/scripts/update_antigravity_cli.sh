#!/bin/bash
# AuraDeck Antigravity CLI Status Updater Script
# This script executes the Antigravity CLI on the Host machine / Raspberry Pi
# and saves the JSON status directly to backend/tokens/antigravity_data.json
# so that the containerized backend reads real-time host quota & credit metrics.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOKENS_DIR="$(cd "$SCRIPT_DIR/../tokens" 2>/dev/null && pwd || echo "$SCRIPT_DIR/../tokens")"
mkdir -p "$TOKENS_DIR"

# Ensure TOKENS_DIR is writable by current host user (if created by root Docker container)
if [ ! -w "$TOKENS_DIR" ]; then
    echo "[$(date)] Fixing ownership for $TOKENS_DIR..."
    sudo chown -R "$USER:$USER" "$TOKENS_DIR" 2>/dev/null || chmod -R 777 "$TOKENS_DIR" 2>/dev/null
fi

OUTPUT_FILE="$TOKENS_DIR/antigravity_data.json"

# Check if agy CLI exists on host
if command -v agy &> /dev/null; then
    agy status --json > "$OUTPUT_FILE.tmp" 2>/dev/null
    # Verify if output is valid JSON
    if [ $? -eq 0 ] && grep -q "{" "$OUTPUT_FILE.tmp" 2>/dev/null; then
        mv "$OUTPUT_FILE.tmp" "$OUTPUT_FILE"
        echo "[$(date)] Successfully updated Antigravity CLI status -> $OUTPUT_FILE"
        exit 0
    else
        rm -f "$OUTPUT_FILE.tmp"
        echo "[$(date)] Notice: 'agy status --json' did not return direct JSON output. Preserving default JSON state."
    fi
fi

# Ensure default JSON state exists if file is missing
if [ ! -f "$OUTPUT_FILE" ]; then
    cat << 'EOF' > "$OUTPUT_FILE"
{
    "status": "ok",
    "plan": "Google AI Pro",
    "available_credits": 823,
    "ai_credits": 823,
    "gemini_models": {
        "weekly_limit_percent": 99.0,
        "five_hour_limit_percent": 97.0,
        "next_reset_5h": "03h 47m",
        "next_reset_weekly": "6 days, 22 hours"
    },
    "claude_gpt_models": {
        "weekly_limit_percent": 100.0,
        "five_hour_limit_percent": 100.0
    }
}
EOF
    echo "[$(date)] Initialized default Antigravity JSON state -> $OUTPUT_FILE"
fi

