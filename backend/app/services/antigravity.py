import logging
import os
import json
import copy
import time
import asyncio
from typing import Dict, Any, List, Optional
from app.config import settings

logger = logging.getLogger("antigravity_service")

# Storage file path for Antigravity state
TOKENS_DIR = getattr(settings, "TOKENS_DIR", os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(__file__))), "tokens"))
STATE_FILE = getattr(settings, "ANTIGRAVITY_STATE_FILE", os.path.join(TOKENS_DIR, "antigravity_data.json"))
if not os.path.isabs(STATE_FILE):
    # Resolve relative to repo root if needed
    repo_root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    if not os.path.exists(STATE_FILE) and os.path.exists(os.path.join(repo_root, STATE_FILE)):
        STATE_FILE = os.path.join(repo_root, STATE_FILE)

# Default values aligned with user's Google AI Pro Antigravity account state
DEFAULT_ANTIGRAVITY_STATE: Dict[str, Any] = {
    "status": "ok",
    "error_message": "",
    "plan": "Google AI Pro",
    "available_credits": 823,
    "ai_credits": 823,
    "gemini_models": {
        "weekly_limit_percent": 99.0,
        "five_hour_limit_percent": 97.0,
        "next_reset_5h": "03h 47m",
        "next_reset_weekly": "6 days, 22 hours",
        "reset_in_seconds_5h": 13620,
        "reset_in_seconds_weekly": 597600
    },
    "claude_gpt_models": {
        "weekly_limit_percent": 100.0,
        "five_hour_limit_percent": 100.0,
        "next_reset_5h": "Full",
        "next_reset_weekly": "Full",
        "reset_in_seconds_5h": 0,
        "reset_in_seconds_weekly": 0
    },
    "dynamic_models": [],
    "additional_models": {},
    "limit_5h": {
        "used": 3.0,
        "total": 100.0,
        "percentage": 3.0
    },
    "limit_weekly": {
        "used": 1.0,
        "total": 100.0,
        "percentage": 1.0
    },
    "next_reset": "03h 47m",
    "credit_hours_remaining": 823.0,
    "percent_quota_used": 3.0,
    "timestamp": 0
}


def format_countdown(seconds: int) -> str:
    """Formats countdown seconds into human-readable duration."""
    if seconds <= 0:
        return "Full"
    if seconds < 60:
        return f"{seconds}s"
    if seconds < 3600:
        return f"{seconds // 60}m"
    if seconds < 86400:
        h = seconds // 3600
        m = (seconds % 3600) // 60
        return f"{h:02d}h {m:02d}m"
    days = seconds // 86400
    rem_hours = (seconds % 86400) // 3600
    if days == 1 and rem_hours == 1:
        return "1 day, 1 hour"
    elif days == 1:
        return f"1 day, {rem_hours} hours" if rem_hours > 0 else "1 day"
    elif rem_hours > 0:
        return f"{days} days, {rem_hours} hours"
    else:
        return f"{days} days"


def parse_fraction_or_percent(val: Any) -> float:
    """Converts a fraction (0.0-1.0) or percentage (0.0-100.0) to clamped float."""
    try:
        f = float(val)
        if 0.0 <= f <= 1.0:
            return round(f * 100.0, 1)
        return round(max(0.0, min(100.0, f)), 1)
    except (TypeError, ValueError):
        return 0.0


def extract_bucket_data(bucket: Any) -> Dict[str, Any]:
    """Extracts remaining_percent, reset_in_seconds, and next_reset from raw bucket."""
    if not isinstance(bucket, dict):
        pct = parse_fraction_or_percent(bucket)
        return {
            "display_name": "",
            "remaining_percent": pct,
            "reset_in_seconds": 0,
            "next_reset": "Full"
        }

    pct_raw = bucket.get("remaining_fraction")
    if pct_raw is None:
        pct_raw = bucket.get("remaining_percent", bucket.get("percent_remaining", bucket.get("percent")))
    pct = parse_fraction_or_percent(pct_raw)

    sec_raw = bucket.get("reset_in_seconds")
    sec = 0
    if sec_raw is not None:
        try:
            sec = int(sec_raw)
        except (ValueError, TypeError):
            sec = 0

    next_reset = bucket.get("next_reset")
    raw_time = bucket.get("reset_time")
    if not next_reset:
        if raw_time and any(u in str(raw_time).lower() for u in ["hour", "day", "min", "sec", "full"]):
            next_reset = str(raw_time)
        elif sec > 0:
            next_reset = format_countdown(sec)
        elif raw_time:
            next_reset = str(raw_time)
        else:
            next_reset = "Full"

    display_name = bucket.get("display_name") or bucket.get("name") or ""

    return {
        "display_name": display_name,
        "remaining_percent": pct,
        "reset_in_seconds": sec,
        "next_reset": str(next_reset)
    }


def normalize_antigravity_payload(raw_data: Optional[Dict[str, Any]]) -> Dict[str, Any]:
    """
    Normalizes any Antigravity payload (raw CLI quota dict or pre-processed state)
    into the unified AuraDeck contract schema with full backward compatibility.
    """
    if not raw_data or not isinstance(raw_data, dict):
        return DEFAULT_ANTIGRAVITY_STATE.copy()

    status = raw_data.get("status", "ok")
    error_msg = raw_data.get("error_message", "")
    plan = raw_data.get("plan") or raw_data.get("plan_tier") or "Google AI Pro"
    credits_val = raw_data.get("available_credits")
    if credits_val is None:
        credits_val = raw_data.get("ai_credits")
    if credits_val is None:
        credits_val = raw_data.get("credits")
    if credits_val is not None:
        try:
            credits = int(float(credits_val))
        except (ValueError, TypeError):
            credits = None
    else:
        credits = None

    # 1. Check if raw_data has a 'quota' or 'quotas' dictionary
    quota_dict = raw_data.get("quota") or raw_data.get("quotas")
    
    # Standard keys aliases
    aliases_5h = ["gemini-5h", "gemini_5h", "gemini-5-hour", "gemini_5_hour"]
    aliases_wk = ["gemini-weekly", "gemini_weekly", "gemini-wk", "gemini_wk"]
    aliases_3p_5h = ["3p-5h", "3p_5h", "3p-5-hour", "claude-gpt-5h", "claude_gpt_5h"]
    aliases_3p_wk = ["3p-weekly", "3p_weekly", "3p-wk", "claude-gpt-weekly", "claude_gpt_weekly"]
    # Deprecated standalone Argon bucket (now unified under Gemini Models)
    deprecated_keys = {"gemini-argon-wtu-weekly", "gemini_argon_wtu_weekly"}

    gemini_5h = None
    gemini_wk = None
    three_p_5h = None
    three_p_wk = None
    dynamic_models_list: List[Dict[str, Any]] = []
    additional_models_dict: Dict[str, Any] = {}
    cleaned_quota_dict: Dict[str, Any] = {}

    if isinstance(quota_dict, dict):
        for raw_k, raw_v in quota_dict.items():
            k_norm = str(raw_k).lower().strip()
            if k_norm in deprecated_keys:
                continue
            cleaned_quota_dict[raw_k] = raw_v
            bucket = extract_bucket_data(raw_v)
            if any(k_norm == a for a in aliases_5h):
                gemini_5h = bucket
            elif any(k_norm == a for a in aliases_wk):
                gemini_wk = bucket
            elif any(k_norm == a for a in aliases_3p_5h):
                three_p_5h = bucket
            elif any(k_norm == a for a in aliases_3p_wk):
                three_p_wk = bucket
            else:
                if bucket.get("display_name"):
                    name = bucket["display_name"]
                else:
                    name = k_norm.replace("-", " ").replace("_", " ").title()
                model_entry = {
                    "key": k_norm,
                    "name": name,
                    "display_name": name,
                    "weekly_limit_percent": bucket["remaining_percent"],
                    "remaining_percent": bucket["remaining_percent"],
                    "next_reset": bucket["next_reset"],
                    "reset_in_seconds": bucket["reset_in_seconds"]
                }
                dynamic_models_list.append(model_entry)
                additional_models_dict[k_norm.replace("-", "_")] = model_entry.copy()

    # 2. Check if nested model dicts exist (e.g. from existing state or previous normalization)
    raw_gemini = raw_data.get("gemini_models", {})
    if isinstance(raw_gemini, dict):
        if gemini_5h is None and "five_hour_limit_percent" in raw_gemini:
            sec_5h = raw_gemini.get("reset_in_seconds_5h", 0)
            reset_5h = raw_gemini.get("next_reset_5h") or (format_countdown(sec_5h) if sec_5h > 0 else "03h 47m")
            gemini_5h = {
                "remaining_percent": parse_fraction_or_percent(raw_gemini.get("five_hour_limit_percent", 97.0)),
                "next_reset": reset_5h,
                "reset_in_seconds": sec_5h
            }
        if gemini_wk is None and "weekly_limit_percent" in raw_gemini:
            sec_wk = raw_gemini.get("reset_in_seconds_weekly", 0)
            reset_wk = raw_gemini.get("next_reset_weekly") or (format_countdown(sec_wk) if sec_wk > 0 else "6 days, 22 hours")
            gemini_wk = {
                "remaining_percent": parse_fraction_or_percent(raw_gemini.get("weekly_limit_percent", 99.0)),
                "next_reset": reset_wk,
                "reset_in_seconds": sec_wk
            }

    raw_3p = raw_data.get("claude_gpt_models", {})
    if isinstance(raw_3p, dict):
        if three_p_5h is None and "five_hour_limit_percent" in raw_3p:
            sec_3p_5h = raw_3p.get("reset_in_seconds_5h", 0)
            three_p_5h = {
                "remaining_percent": parse_fraction_or_percent(raw_3p.get("five_hour_limit_percent", 100.0)),
                "next_reset": raw_3p.get("next_reset_5h") or (format_countdown(sec_3p_5h) if sec_3p_5h > 0 else "Full"),
                "reset_in_seconds": sec_3p_5h
            }
        if three_p_wk is None and "weekly_limit_percent" in raw_3p:
            sec_3p_wk = raw_3p.get("reset_in_seconds_weekly", 0)
            three_p_wk = {
                "remaining_percent": parse_fraction_or_percent(raw_3p.get("weekly_limit_percent", 100.0)),
                "next_reset": raw_3p.get("next_reset_weekly") or (format_countdown(sec_3p_wk) if sec_3p_wk > 0 else "Full"),
                "reset_in_seconds": sec_3p_wk
            }

    # Also capture any existing additional_models or dynamic_models
    raw_dyn = raw_data.get("dynamic_models")
    if isinstance(raw_dyn, list) and not dynamic_models_list:
        for item in raw_dyn:
            if isinstance(item, dict):
                k = str(item.get("key", "dynamic-model"))
                if k.lower().strip() in deprecated_keys:
                    continue
                name = item.get("name") or item.get("display_name") or k.replace("-", " ").title()
                pct = parse_fraction_or_percent(item.get("remaining_percent", item.get("weekly_limit_percent", 0.0)))
                entry = {
                    "key": k,
                    "name": name,
                    "display_name": name,
                    "weekly_limit_percent": pct,
                    "remaining_percent": pct,
                    "next_reset": item.get("next_reset", "Full"),
                    "reset_in_seconds": item.get("reset_in_seconds", 0)
                }
                dynamic_models_list.append(entry)
                additional_models_dict[k.replace("-", "_")] = entry.copy()

    raw_add = raw_data.get("additional_models")
    if isinstance(raw_add, dict) and not additional_models_dict:
        for k, item in raw_add.items():
            if isinstance(item, dict):
                if str(k).lower().strip() in deprecated_keys:
                    continue
                name = item.get("name") or k.replace("_", " ").title()
                pct = parse_fraction_or_percent(item.get("weekly_limit_percent", item.get("remaining_percent", 0.0)))
                entry = {
                    "key": k.replace("_", "-"),
                    "name": name,
                    "display_name": name,
                    "weekly_limit_percent": pct,
                    "remaining_percent": pct,
                    "next_reset": item.get("next_reset", "Full"),
                    "reset_in_seconds": item.get("reset_in_seconds", 0)
                }
                dynamic_models_list.append(entry)
                additional_models_dict[k] = entry.copy()

    # Fill defaults for anything still None
    if gemini_5h is None:
        gemini_5h = {"remaining_percent": 97.0, "next_reset": "03h 47m", "reset_in_seconds": 13620}
    if gemini_wk is None:
        gemini_wk = {"remaining_percent": 99.0, "next_reset": "6 days, 22 hours", "reset_in_seconds": 597600}
    if three_p_5h is None:
        three_p_5h = {"remaining_percent": 100.0, "next_reset": "Full", "reset_in_seconds": 0}
    if three_p_wk is None:
        three_p_wk = {"remaining_percent": 100.0, "next_reset": "Full", "reset_in_seconds": 0}

    # Derived quota usage percentages
    used_5h = round(100.0 - gemini_5h["remaining_percent"], 1)
    used_wk = round(100.0 - gemini_wk["remaining_percent"], 1)

    ts_raw = raw_data.get("timestamp")
    try:
        timestamp = int(ts_raw) if ts_raw is not None else 0
    except (ValueError, TypeError):
        timestamp = 0

    return {
        "status": status,
        "error_message": error_msg,
        "plan": plan,
        "available_credits": credits,
        "ai_credits": credits,
        "gemini_models": {
            "five_hour_limit_percent": gemini_5h["remaining_percent"],
            "weekly_limit_percent": gemini_wk["remaining_percent"],
            "next_reset_5h": gemini_5h["next_reset"],
            "next_reset_weekly": gemini_wk["next_reset"],
            "reset_in_seconds_5h": gemini_5h.get("reset_in_seconds", 0),
            "reset_in_seconds_weekly": gemini_wk.get("reset_in_seconds", 0)
        },
        "claude_gpt_models": {
            "five_hour_limit_percent": three_p_5h["remaining_percent"],
            "weekly_limit_percent": three_p_wk["remaining_percent"],
            "next_reset_5h": three_p_5h["next_reset"],
            "next_reset_weekly": three_p_wk["next_reset"],
            "reset_in_seconds_5h": three_p_5h.get("reset_in_seconds", 0),
            "reset_in_seconds_weekly": three_p_wk.get("reset_in_seconds", 0)
        },
        "dynamic_models": dynamic_models_list,
        "additional_models": additional_models_dict,
        "quota": cleaned_quota_dict,
        
        # Legacy/Nested fields for Web Dashboard compatibility
        "limit_5h": {
            "used": used_5h,
            "total": 100.0,
            "percentage": used_5h
        },
        "limit_weekly": {
            "used": used_wk,
            "total": 100.0,
            "percentage": used_wk
        },
        "next_reset": gemini_5h["next_reset"],
        
        # Flat fields expected by ESP32-S3 screen client
        "credit_hours_remaining": float(credits) if credits is not None else -1.0,
        "percent_quota_used": used_5h,
        "timestamp": timestamp
    }


def _advance_pair(
    pct_5h: float,
    sec_5h: int,
    rst_5h: str,
    pct_wk: float,
    sec_wk: int,
    rst_wk: str,
    elapsed: int,
) -> tuple[float, int, str, float, int, str]:
    """Advances a (5h, weekly) quota pair by `elapsed` seconds.

    Weekly resets to 100% / "Full" when its countdown expires.
    5h resets to 100% / "Full" ONLY when weekly quota is > 0% (never overrides
    an exhausted weekly limit).
    """
    new_sec_wk = sec_wk
    new_pct_wk = pct_wk
    new_rst_wk = rst_wk
    if sec_wk > 0:
        new_sec_wk = max(0, sec_wk - elapsed)
        if new_sec_wk == 0:
            new_pct_wk = 100.0
            new_rst_wk = "Full"
        else:
            new_rst_wk = format_countdown(new_sec_wk)

    new_sec_5h = sec_5h
    new_pct_5h = pct_5h
    new_rst_5h = rst_5h
    if sec_5h > 0:
        new_sec_5h = max(0, sec_5h - elapsed)
        if new_sec_5h == 0:
            if new_pct_wk > 0.0:
                new_pct_5h = 100.0
                new_rst_5h = "Full"
            else:
                new_rst_5h = "0m"
        else:
            new_rst_5h = format_countdown(new_sec_5h)

    return new_pct_5h, new_sec_5h, new_rst_5h, new_pct_wk, new_sec_wk, new_rst_wk


def apply_elapsed_time(state: Dict[str, Any], now_ts: Optional[int] = None) -> Dict[str, Any]:
    """Returns a copy of `state` with countdowns adjusted for elapsed time since `timestamp`."""
    if not isinstance(state, dict):
        return state
    if state.get("status") in ("unauthenticated", "error"):
        return state

    ts = state.get("timestamp") or 0
    try:
        ts = int(ts)
    except (ValueError, TypeError):
        ts = 0

    now = int(time.time()) if now_ts is None else int(now_ts)
    result = copy.deepcopy(state)
    result["server_time"] = now

    if ts <= 0:
        return result

    elapsed = max(0, now - ts)
    if elapsed == 0:
        return result

    gm = result.get("gemini_models")
    if isinstance(gm, dict):
        g5h, g5h_s, g5h_r, gwk, gwk_s, gwk_r = _advance_pair(
            float(gm.get("five_hour_limit_percent", 100.0)),
            int(gm.get("reset_in_seconds_5h", 0) or 0),
            str(gm.get("next_reset_5h", "Full")),
            float(gm.get("weekly_limit_percent", 100.0)),
            int(gm.get("reset_in_seconds_weekly", 0) or 0),
            str(gm.get("next_reset_weekly", "Full")),
            elapsed,
        )
        gm["five_hour_limit_percent"] = g5h
        gm["reset_in_seconds_5h"] = g5h_s
        gm["next_reset_5h"] = g5h_r
        gm["weekly_limit_percent"] = gwk
        gm["reset_in_seconds_weekly"] = gwk_s
        gm["next_reset_weekly"] = gwk_r

        used_5h = round(100.0 - g5h, 1)
        used_wk = round(100.0 - gwk, 1)
        result["limit_5h"] = {"used": used_5h, "total": 100.0, "percentage": used_5h}
        result["limit_weekly"] = {"used": used_wk, "total": 100.0, "percentage": used_wk}
        result["next_reset"] = g5h_r
        result["percent_quota_used"] = used_5h

    cg = result.get("claude_gpt_models")
    if isinstance(cg, dict):
        t5h, t5h_s, t5h_r, twk, twk_s, twk_r = _advance_pair(
            float(cg.get("five_hour_limit_percent", 100.0)),
            int(cg.get("reset_in_seconds_5h", 0) or 0),
            str(cg.get("next_reset_5h", "Full")),
            float(cg.get("weekly_limit_percent", 100.0)),
            int(cg.get("reset_in_seconds_weekly", 0) or 0),
            str(cg.get("next_reset_weekly", "Full")),
            elapsed,
        )
        cg["five_hour_limit_percent"] = t5h
        cg["reset_in_seconds_5h"] = t5h_s
        cg["next_reset_5h"] = t5h_r
        cg["weekly_limit_percent"] = twk
        cg["reset_in_seconds_weekly"] = twk_s
        cg["next_reset_weekly"] = twk_r

    seen_ids = set()
    dyn = result.get("dynamic_models")
    if isinstance(dyn, list):
        for item in dyn:
            if isinstance(item, dict) and id(item) not in seen_ids:
                seen_ids.add(id(item))
                sec = int(item.get("reset_in_seconds", 0) or 0)
                if sec > 0:
                    new_sec = max(0, sec - elapsed)
                    item["reset_in_seconds"] = new_sec
                    if new_sec == 0:
                        item["weekly_limit_percent"] = 100.0
                        item["remaining_percent"] = 100.0
                        item["next_reset"] = "Full"
                    else:
                        item["next_reset"] = format_countdown(new_sec)

    add = result.get("additional_models")
    if isinstance(add, dict):
        for item in add.values():
            if isinstance(item, dict) and id(item) not in seen_ids:
                seen_ids.add(id(item))
                sec = int(item.get("reset_in_seconds", 0) or 0)
                if sec > 0:
                    new_sec = max(0, sec - elapsed)
                    item["reset_in_seconds"] = new_sec
                    if new_sec == 0:
                        item["weekly_limit_percent"] = 100.0
                        item["remaining_percent"] = 100.0
                        item["next_reset"] = "Full"
                    else:
                        item["next_reset"] = format_countdown(new_sec)

    return result


def load_antigravity_state() -> Dict[str, Any]:
    """Loads state file and returns normalized data."""
    os.makedirs(os.path.dirname(os.path.abspath(STATE_FILE)), exist_ok=True)
    if os.path.exists(STATE_FILE):
        try:
            with open(STATE_FILE, "r", encoding="utf-8") as f:
                data = json.load(f)
                return normalize_antigravity_payload(data)
        except Exception as e:
            logger.error(f"Error loading {STATE_FILE}: {e}")
    
    # Save default state if missing
    save_antigravity_state(DEFAULT_ANTIGRAVITY_STATE)
    return DEFAULT_ANTIGRAVITY_STATE.copy()


def save_antigravity_state(state: Dict[str, Any]) -> None:
    """Atomically saves state using temporary file and atomic replace."""
    dest_path = os.path.abspath(STATE_FILE)
    os.makedirs(os.path.dirname(dest_path), exist_ok=True)
    tmp_path = dest_path + f".tmp.{os.getpid()}"
    try:
        with open(tmp_path, "w", encoding="utf-8") as f:
            json.dump(state, f, indent=4, ensure_ascii=False)
        os.replace(tmp_path, dest_path)
    except Exception as e:
        logger.error(f"Error saving {dest_path}: {e}")
        # Clean up temp file on failure
        if os.path.exists(tmp_path):
            try:
                os.remove(tmp_path)
            except Exception:
                pass


def ingest_antigravity_payload(raw_payload: Dict[str, Any]) -> Dict[str, Any]:
    """
    Ingests an incoming payload from CLI bridge or external REST caller,
    normalizes the schema, atomically persists to disk, and returns normalized state.
    """
    normalized = normalize_antigravity_payload(raw_payload)
    if not normalized.get("timestamp"):
        normalized["timestamp"] = int(time.time())
    save_antigravity_state(normalized)
    normalized["server_time"] = int(time.time())
    return normalized


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
            normalized = ingest_antigravity_payload(parsed)
            logger.info("Successfully fetched Antigravity state via direct CLI command")
            return normalized
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
    Reads mounted tokens/antigravity_data.json first for instant response;
    falls back to direct CLI execution if state file is missing.
    """
    # 1. Read mounted state file first if present
    if os.path.exists(STATE_FILE):
        state = load_antigravity_state()
    else:
        # 2. Try direct CLI execution if file missing
        cli_result = await execute_cli_command()
        if cli_result.get("status") == "ok":
            state = cli_result
        else:
            state = load_antigravity_state()
            state["status"] = "unauthenticated"
            state["error_message"] = cli_result.get("error_message", "Antigravity CLI unauthenticated or unavailable")

    return apply_elapsed_time(normalize_antigravity_payload(state))
