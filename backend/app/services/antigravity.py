import logging
import os
import json
import asyncio
from typing import Dict, Any
from app.config import settings

logger = logging.getLogger("antigravity_service")

# Storage file path for Antigravity state
TOKENS_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), "tokens")
STATE_FILE = os.path.join(TOKENS_DIR, "antigravity_data.json")

# Default values aligned with user's Google AI Pro Antigravity account state
DEFAULT_ANTIGRAVITY_STATE = {
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

def load_antigravity_state() -> Dict[str, Any]:
    os.makedirs(TOKENS_DIR, exist_ok=True)
    if os.path.exists(STATE_FILE):
        try:
            with open(STATE_FILE, "r", encoding="utf-8") as f:
                data = json.load(f)
                merged = DEFAULT_ANTIGRAVITY_STATE.copy()
                merged.update(data)
                return merged
        except Exception as e:
            logger.error(f"Error loading {STATE_FILE}: {e}")
    
    # Save default state if missing
    save_antigravity_state(DEFAULT_ANTIGRAVITY_STATE)
    return DEFAULT_ANTIGRAVITY_STATE

def save_antigravity_state(state: Dict[str, Any]) -> None:
    os.makedirs(TOKENS_DIR, exist_ok=True)
    try:
        with open(STATE_FILE, "w", encoding="utf-8") as f:
            json.dump(state, f, indent=4, ensure_ascii=False)
    except Exception as e:
        logger.error(f"Error saving {STATE_FILE}: {e}")

async def execute_cli_command() -> Dict[str, Any]:
    """
    Attempts to execute the configured CLI command directly via subprocess shell.
    """
    cmd = getattr(settings, "ANTIGRAVITY_CLI_CMD", "agy status --json")
    try:
        proc = await asyncio.create_subprocess_shell(
            cmd,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=10.0)
        
        if proc.returncode == 0 and stdout:
            out_str = stdout.decode("utf-8").strip()
            parsed = json.loads(out_str)
            parsed["status"] = "ok"
            save_antigravity_state(parsed)
            logger.info("Successfully fetched Antigravity state via direct CLI command")
            return parsed
        else:
            err_msg = stderr.decode("utf-8").strip() if stderr else "CLI command returned non-zero exit code"
            logger.warning(f"CLI command failed ({proc.returncode}): {err_msg}")
            return {"status": "error", "error_message": err_msg}
    except asyncio.TimeoutError:
        logger.warning("CLI command timed out after 10s")
        return {"status": "error", "error_message": "CLI execution timed out"}
    except Exception as e:
        logger.debug(f"Direct CLI execution unavailable or failed: {e}")
        return {"status": "error", "error_message": str(e)}

async def get_antigravity_credits() -> Dict[str, Any]:
    """
    Fetches the Google Antigravity query limits and credit usage.
    Tries direct CLI execution first; falls back to loading mounted tokens/antigravity_data.json.
    """
    # 1. Try direct CLI execution
    cli_result = await execute_cli_command()
    if cli_result.get("status") == "ok":
        state = cli_result
    else:
        # 2. Fallback to reading mounted state file
        state = load_antigravity_state()
        if not os.path.exists(STATE_FILE) and cli_result.get("status") != "ok":
            state["status"] = "unauthenticated"
            state["error_message"] = cli_result.get("error_message", "Antigravity CLI unauthenticated or unavailable")

    status = state.get("status", "ok")
    error_msg = state.get("error_message", "")
    plan = state.get("plan", "Google AI Pro")
    credits = state.get("available_credits", state.get("ai_credits", 823))
    
    gemini = state.get("gemini_models", {})
    gemini_5h = gemini.get("five_hour_limit_percent", 97.0)
    gemini_weekly = gemini.get("weekly_limit_percent", 99.0)
    reset_5h = gemini.get("next_reset_5h", "03h 47m")
    
    # Derived quota usage percentages
    used_5h_pct = round(100.0 - gemini_5h, 1)
    used_weekly_pct = round(100.0 - gemini_weekly, 1)

    return {
        "status": status,
        "error_message": error_msg,
        "plan": plan,
        "available_credits": credits,
        "ai_credits": credits,
        "gemini_models": gemini,
        "claude_gpt_models": state.get("claude_gpt_models", {}),
        
        # Legacy/Nested fields for Web Dashboard compatibility
        "limit_5h": {
            "used": used_5h_pct,
            "total": 100.0,
            "percentage": used_5h_pct
        },
        "limit_weekly": {
            "used": used_weekly_pct,
            "total": 100.0,
            "percentage": used_weekly_pct
        },
        "next_reset": reset_5h,
        
        # Flat fields expected by ESP32-S3 screen client
        "credit_hours_remaining": float(credits),
        "percent_quota_used": used_5h_pct
    }


