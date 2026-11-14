#!/usr/bin/env python3
"""
cyberdeck_statusline.py - Antigravity CLI Custom Status Line Bridge for AuraDeck

Reads raw Antigravity CLI JSON status from sys.stdin, formats a high-contrast
single-line summary to sys.stdout in <50ms with zero stdout noise on errors, and
relays the normalized payload to AuraDeck via atomic file write, REST POST, or MQTT.

Usage in Antigravity CLI:
    agy /statusline /path/to/cyberdeck_statusline.py
"""

import sys
import os
import json
import time
import argparse

# Default fallback configurations
DEFAULT_CONFIG = {
    "statusline": {
        "use_ansi": True,
        "show_credits": True,
        "show_countdowns": False,
        "max_dynamic_models": 2
    },
    "transports": {
        "file": {
            "enabled": True,
            "path": "backend/tokens/antigravity_data.json",
            "fallback_path": "/tmp/agy_quota.json"
        },
        "rest": {
            "enabled": True,
            "url": "http://127.0.0.1:8000/api/v1/antigravity",
            "timeout_seconds": 0.2
        },
        "mqtt": {
            "enabled": False,
            "host": "127.0.0.1",
            "port": 1883,
            "topic": "auradeck/antigravity",
            "timeout_seconds": 0.2
        }
    },
    "debug": {
        "dump_file": "",
        "log_file": ""
    }
}


def log_debug(message: str, log_file: str = ""):
    """Safely logs debug info to a file if configured; never emits to stdout/stderr."""
    if not log_file:
        return
    try:
        with open(log_file, "a", encoding="utf-8") as f:
            f.write(f"[{time.strftime('%Y-%m-%d %H:%M:%S')}] {message}\n")
    except Exception:
        pass


def _merge_config(base: dict, update: dict) -> dict:
    """Recursively merges update into base dict with strict type safety.
    Never replaces a default dictionary section with a non-dict scalar."""
    if not isinstance(update, dict):
        return base
    for k, v in update.items():
        if k in base:
            if isinstance(base[k], dict):
                if isinstance(v, dict):
                    _merge_config(base[k], v)
                else:
                    # Never replace a default dictionary section with a non-dict scalar
                    pass
            else:
                if not isinstance(v, dict):
                    if isinstance(base[k], bool):
                        if isinstance(v, bool):
                            base[k] = v
                    elif isinstance(base[k], (int, float)):
                        if isinstance(v, (int, float)) and not isinstance(v, bool):
                            base[k] = v
                    elif isinstance(base[k], str):
                        if isinstance(v, str):
                            base[k] = v
                    else:
                        base[k] = v
        else:
            base[k] = v
    return base


def load_config(config_path: str = None) -> dict:
    """Loads configuration cascading from config file, fallback example, and env vars."""
    config = json.loads(json.dumps(DEFAULT_CONFIG))

    search_paths = []
    if config_path:
        search_paths.append(config_path)
    env_cfg = os.environ.get("CYBERDECK_CONFIG")
    if env_cfg:
        search_paths.append(env_cfg)
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    search_paths.append(os.path.join(script_dir, "config.json"))
    search_paths.append(os.path.join(script_dir, "config.example.json"))

    loaded = False
    for path in search_paths:
        if path and os.path.exists(path):
            try:
                with open(path, "r", encoding="utf-8") as f:
                    user_cfg = json.load(f)
                    if isinstance(user_cfg, dict):
                        _merge_config(config, user_cfg)
                        loaded = True
                        break
            except Exception:
                continue

    # Environment variable overrides
    if "CYBERDECK_NO_COLOR" in os.environ or "NO_COLOR" in os.environ:
        if isinstance(config.get("statusline"), dict):
            config["statusline"]["use_ansi"] = False

    if "CYBERDECK_DEBUG_DUMP" in os.environ:
        if isinstance(config.get("debug"), dict):
            config["debug"]["dump_file"] = os.environ["CYBERDECK_DEBUG_DUMP"]

    if "CYBERDECK_LOG_FILE" in os.environ:
        if isinstance(config.get("debug"), dict):
            config["debug"]["log_file"] = os.environ["CYBERDECK_LOG_FILE"]

    if "CYBERDECK_REST_URL" in os.environ:
        if isinstance(config.get("transports"), dict) and isinstance(config["transports"].get("rest"), dict):
            config["transports"]["rest"]["url"] = os.environ["CYBERDECK_REST_URL"]
            config["transports"]["rest"]["enabled"] = True

    if "CYBERDECK_FILE_PATH" in os.environ:
        if isinstance(config.get("transports"), dict) and isinstance(config["transports"].get("file"), dict):
            config["transports"]["file"]["path"] = os.environ["CYBERDECK_FILE_PATH"]
            config["transports"]["file"]["enabled"] = True

    if "CYBERDECK_MQTT_HOST" in os.environ:
        if isinstance(config.get("transports"), dict) and isinstance(config["transports"].get("mqtt"), dict):
            config["transports"]["mqtt"]["host"] = os.environ["CYBERDECK_MQTT_HOST"]
            config["transports"]["mqtt"]["enabled"] = True

    if "CYBERDECK_MQTT_PORT" in os.environ:
        if isinstance(config.get("transports"), dict) and isinstance(config["transports"].get("mqtt"), dict):
            try:
                config["transports"]["mqtt"]["port"] = int(os.environ["CYBERDECK_MQTT_PORT"])
            except ValueError:
                pass

    if "CYBERDECK_MQTT_TOPIC" in os.environ:
        if isinstance(config.get("transports"), dict) and isinstance(config["transports"].get("mqtt"), dict):
            config["transports"]["mqtt"]["topic"] = os.environ["CYBERDECK_MQTT_TOPIC"]

    return config


def format_duration(seconds: int) -> str:
    """Formats seconds into human-readable countdown matching AuraDeck contracts."""
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


def parse_percentage(val: any) -> float:
    """Converts a fraction (0.0-1.0) or percentage (0-100) to 0.0-100.0 clamped float."""
    try:
        f = float(val)
        if 0.0 <= f <= 1.0:
            return round(f * 100.0, 1)
        return round(max(0.0, min(100.0, f)), 1)
    except (TypeError, ValueError):
        return 0.0


def extract_quota_bucket(bucket_data: any) -> dict:
    """Safely extracts remaining_percent, reset_in_seconds, and next_reset from a bucket dict."""
    if not isinstance(bucket_data, dict):
        pct = parse_percentage(bucket_data)
        return {
            "display_name": "",
            "remaining_percent": pct,
            "remaining_fraction": round(pct / 100.0, 4),
            "reset_in_seconds": 0,
            "next_reset": "Full"
        }

    # Extract percentage/fraction
    fraction_val = bucket_data.get("remaining_fraction")
    percent_val = bucket_data.get("remaining_percent", bucket_data.get("percent_remaining", bucket_data.get("percent")))
    
    if fraction_val is not None:
        pct = parse_percentage(fraction_val)
    elif percent_val is not None:
        pct = parse_percentage(percent_val)
    else:
        pct = 0.0

    # Extract reset countdown
    reset_sec = bucket_data.get("reset_in_seconds")
    if reset_sec is not None:
        try:
            reset_sec = int(reset_sec)
        except (ValueError, TypeError):
            reset_sec = 0
    else:
        reset_sec = 0

    next_reset = bucket_data.get("next_reset")
    raw_reset_time = bucket_data.get("reset_time")

    if not next_reset:
        if raw_reset_time and any(unit in str(raw_reset_time).lower() for unit in ["hour", "day", "min", "sec", "full"]):
            next_reset = str(raw_reset_time)
        elif reset_sec > 0:
            next_reset = format_duration(reset_sec)
        elif raw_reset_time:
            next_reset = str(raw_reset_time)
        else:
            next_reset = "Full"

    display_name = bucket_data.get("display_name") or bucket_data.get("name") or ""

    return {
        "display_name": display_name,
        "remaining_percent": pct,
        "remaining_fraction": round(pct / 100.0, 4),
        "reset_in_seconds": reset_sec,
        "next_reset": str(next_reset)
    }


def parse_antigravity_input(raw_json: str) -> dict:
    """Dynamically parses Antigravity stdin JSON payload."""
    try:
        data = json.loads(raw_json)
    except Exception:
        return {}

    if not isinstance(data, dict):
        return {}

    # Extract Plan
    plan = data.get("plan") or data.get("plan_tier") or "Google AI Pro"

    # Extract Credits
    credits_val = data.get("credits", data.get("available_credits", data.get("ai_credits")))
    if credits_val is not None:
        try:
            credits = int(credits_val)
        except (ValueError, TypeError):
            try:
                credits = int(float(credits_val))
            except Exception:
                credits = 0
    else:
        credits = 0

    # Extract Quota dictionary
    quota_dict = data.get("quota") or data.get("quotas")
    if not isinstance(quota_dict, dict):
        # Look if nested models are directly at root
        quota_dict = {}
        for k in ["gemini-5h", "gemini-weekly", "3p-5h", "3p-weekly"]:
            if k in data:
                quota_dict[k] = data[k]

    parsed_buckets = {}
    standard_keys = {
        "gemini-5h": ["gemini-5h", "gemini_5h", "gemini-5-hour", "gemini_5_hour"],
        "gemini-weekly": ["gemini-weekly", "gemini_weekly", "gemini-wk", "gemini_wk"],
        "3p-5h": ["3p-5h", "3p_5h", "3p-5-hour", "claude-gpt-5h", "claude_gpt_5h"],
        "3p-weekly": ["3p-weekly", "3p_weekly", "3p-wk", "claude-gpt-weekly", "claude_gpt_weekly"]
    }

    # Map raw keys to standard keys or dynamic keys
    gemini_5h_bucket = None
    gemini_wk_bucket = None
    three_p_5h_bucket = None
    three_p_wk_bucket = None
    dynamic_buckets = []

    for raw_key, raw_val in quota_dict.items():
        norm_key = raw_key.lower().strip()
        bucket_info = extract_quota_bucket(raw_val)
        parsed_buckets[norm_key] = bucket_info

        # Check standard mappings
        if any(norm_key == alias for alias in standard_keys["gemini-5h"]):
            gemini_5h_bucket = bucket_info
        elif any(norm_key == alias for alias in standard_keys["gemini-weekly"]):
            gemini_wk_bucket = bucket_info
        elif any(norm_key == alias for alias in standard_keys["3p-5h"]):
            three_p_5h_bucket = bucket_info
        elif any(norm_key == alias for alias in standard_keys["3p-weekly"]):
            three_p_wk_bucket = bucket_info
        else:
            # Dynamic model bucket
            display_name = bucket_info.get("display_name")
            if not display_name:
                if "argon" in norm_key and ("wtu" in norm_key or "weekly" in norm_key):
                    display_name = "Gemini 4 Argon"
                else:
                    display_name = norm_key.replace("-", " ").replace("_", " ").title()
            dynamic_buckets.append({
                "key": norm_key,
                "name": display_name,
                "display_name": display_name,
                "weekly_limit_percent": bucket_info["remaining_percent"],
                "remaining_percent": bucket_info["remaining_percent"],
                "next_reset": bucket_info["next_reset"],
                "reset_in_seconds": bucket_info["reset_in_seconds"]
            })

    # Default fallback buckets if not present in input
    if not gemini_5h_bucket:
        gemini_5h_bucket = {"remaining_percent": 100.0, "remaining_fraction": 1.0, "reset_in_seconds": 0, "next_reset": "Full"}
    if not gemini_wk_bucket:
        gemini_wk_bucket = {"remaining_percent": 100.0, "remaining_fraction": 1.0, "reset_in_seconds": 0, "next_reset": "Full"}
    if not three_p_5h_bucket:
        three_p_5h_bucket = {"remaining_percent": 100.0, "remaining_fraction": 1.0, "reset_in_seconds": 0, "next_reset": "Full"}
    if not three_p_wk_bucket:
        three_p_wk_bucket = {"remaining_percent": 100.0, "remaining_fraction": 1.0, "reset_in_seconds": 0, "next_reset": "Full"}

    return {
        "status": data.get("status", "ok"),
        "has_quota": len(parsed_buckets) > 0,
        "plan": plan,
        "credits": credits,
        "available_credits": credits,
        "ai_credits": credits,
        "gemini_5h": gemini_5h_bucket,
        "gemini_weekly": gemini_wk_bucket,
        "3p_5h": three_p_5h_bucket,
        "3p_weekly": three_p_wk_bucket,
        "dynamic_models": dynamic_buckets,
        "raw_quota": parsed_buckets
    }


def format_statusline(parsed_data: dict, config: dict) -> str:
    """Formats a single-line summary string with optional ANSI colors."""
    if not parsed_data or parsed_data.get("status") == "error":
        return "Antigravity Quota: Unavailable"
    if not parsed_data.get("has_quota", True):
        return "Antigravity Quota: Loading..."

    cfg = config.get("statusline") if isinstance(config.get("statusline"), dict) else {}
    use_ansi = cfg.get("use_ansi", True)
    show_credits = cfg.get("show_credits", True)
    show_countdowns = cfg.get("show_countdowns", False)
    max_dyn = cfg.get("max_dynamic_models", 2)

    def colorize(text: str, pct: float) -> str:
        if not use_ansi:
            return text
        if pct >= 50.0:
            color = "\033[32m"  # Green
        elif pct >= 20.0:
            color = "\033[33m"  # Yellow
        else:
            color = "\033[31m"  # Red
        return f"{color}{text}\033[0m"

    parts = []

    # Gemini 5h
    g5h = parsed_data["gemini_5h"]["remaining_percent"]
    g5h_txt = f"{int(round(g5h))}%"
    if show_countdowns and parsed_data["gemini_5h"]["next_reset"] != "Full":
        g5h_txt += f" ({parsed_data['gemini_5h']['next_reset']})"
    parts.append(f"Gemini 5h: {colorize(g5h_txt, g5h)}")

    # Gemini Weekly
    gwk = parsed_data["gemini_weekly"]["remaining_percent"]
    gwk_txt = f"{int(round(gwk))}%"
    if show_countdowns and parsed_data["gemini_weekly"]["next_reset"] != "Full":
        gwk_txt += f" ({parsed_data['gemini_weekly']['next_reset']})"
    parts.append(f"Wk: {colorize(gwk_txt, gwk)}")

    # 3P
    tp_5h = parsed_data["3p_5h"]["remaining_percent"]
    tp_txt = f"{int(round(tp_5h))}%"
    parts.append(f"3P: {colorize(tp_txt, tp_5h)}")

    # Dynamic models (e.g. Gemini 4 Argon)
    dyn_models = parsed_data.get("dynamic_models", [])
    for dm in dyn_models[:max_dyn]:
        short_name = dm["name"]
        if short_name.startswith("Gemini 4 "):
            short_name = short_name.replace("Gemini 4 ", "")
        dm_pct = dm["remaining_percent"]
        dm_txt = f"{int(round(dm_pct))}%"
        parts.append(f"{short_name}: {colorize(dm_txt, dm_pct)}")

    # Credits
    credits = parsed_data.get("credits", 0)
    if show_credits and credits > 0:
        cr_text = f"Credits: {credits}"
        if use_ansi:
            cr_text = f"\033[36m{cr_text}\033[0m"  # Cyan
        parts.append(cr_text)

    delimiter = " | "
    return delimiter.join(parts)


def build_relay_payload(parsed_data: dict) -> dict:
    """Builds the comprehensive normalized payload for backend, MQTT, and atomic file."""
    g5h = parsed_data["gemini_5h"]["remaining_percent"]
    gwk = parsed_data["gemini_weekly"]["remaining_percent"]
    used_5h = round(100.0 - g5h, 1)
    used_wk = round(100.0 - gwk, 1)

    # Dynamic models formatted for both list and dict consumers
    dyn_list = parsed_data.get("dynamic_models", [])
    dyn_dict = {}
    for dm in dyn_list:
        key_sanitized = dm["key"].replace("-", "_")
        dyn_dict[key_sanitized] = {
            "name": dm["name"],
            "weekly_limit_percent": dm["remaining_percent"],
            "remaining_percent": dm["remaining_percent"],
            "next_reset": dm["next_reset"],
            "reset_in_seconds": dm.get("reset_in_seconds", 0)
        }

    return {
        "status": parsed_data.get("status", "ok"),
        "error_message": "",
        "plan": parsed_data.get("plan", "Google AI Pro"),
        "available_credits": parsed_data.get("credits", 0),
        "ai_credits": parsed_data.get("credits", 0),
        "gemini_models": {
            "five_hour_limit_percent": g5h,
            "weekly_limit_percent": gwk,
            "next_reset_5h": parsed_data["gemini_5h"]["next_reset"],
            "next_reset_weekly": parsed_data["gemini_weekly"]["next_reset"],
            "reset_in_seconds_5h": parsed_data["gemini_5h"]["reset_in_seconds"],
            "reset_in_seconds_weekly": parsed_data["gemini_weekly"]["reset_in_seconds"]
        },
        "claude_gpt_models": {
            "five_hour_limit_percent": parsed_data["3p_5h"]["remaining_percent"],
            "weekly_limit_percent": parsed_data["3p_weekly"]["remaining_percent"],
            "next_reset_5h": parsed_data["3p_5h"]["next_reset"],
            "next_reset_weekly": parsed_data["3p_weekly"]["next_reset"]
        },
        "dynamic_models": dyn_list,
        "additional_models": dyn_dict,
        "quota": parsed_data.get("raw_quota", {}),
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
        "next_reset": parsed_data["gemini_5h"]["next_reset"],
        "credit_hours_remaining": float(parsed_data.get("credits", 0)),
        "percent_quota_used": used_5h,
        "timestamp": int(time.time()),
        "source": "cyberdeck_statusline"
    }


def execute_transports(payload: dict, config: dict):
    """Executes configured transport delivery (atomic file, REST POST, MQTT) silently."""
    transports = config.get("transports") if isinstance(config.get("transports"), dict) else {}
    debug_cfg = config.get("debug") if isinstance(config.get("debug"), dict) else {}
    log_file = debug_cfg.get("log_file", "")

    payload_json = json.dumps(payload, indent=2, ensure_ascii=False)

    # 1. Atomic File Transport (instant, ~1ms)
    file_cfg = transports.get("file") if isinstance(transports.get("file"), dict) else {}
    if file_cfg.get("enabled", True):
        dest_path = file_cfg.get("path", "backend/tokens/antigravity_data.json")
        if not os.path.isabs(dest_path):
            script_dir = os.path.dirname(os.path.abspath(__file__))
            dest_path = os.path.join(script_dir, dest_path)

        def try_write(target_path: str) -> bool:
            try:
                parent_dir = os.path.dirname(os.path.abspath(target_path))
                os.makedirs(parent_dir, exist_ok=True)
                temp_file = target_path + f".tmp.{os.getpid()}"
                with open(temp_file, "w", encoding="utf-8") as f:
                    f.write(payload_json)
                os.replace(temp_file, target_path)
                return True
            except Exception as e:
                log_debug(f"Atomic file write failed for {target_path}: {e}", log_file)
                return False

        if not try_write(dest_path):
            fallback_path = file_cfg.get("fallback_path", "/tmp/agy_quota.json")
            try_write(fallback_path)

    # 2. Network Transports (REST & MQTT) dispatched with tight timeout
    def run_network():
        # REST POST Transport (lightweight http.client for minimal latency)
        rest_cfg = transports.get("rest") if isinstance(transports.get("rest"), dict) else {}
        if rest_cfg.get("enabled", True):
            url = rest_cfg.get("url", "http://127.0.0.1:8000/api/v1/antigravity")
            timeout = float(rest_cfg.get("timeout_seconds", 0.08))
            try:
                import http.client
                from urllib.parse import urlsplit
                parsed = urlsplit(url)
                host = parsed.hostname or "127.0.0.1"
                port = parsed.port or (443 if parsed.scheme == "https" else 80)
                path = parsed.path or "/api/v1/antigravity"
                if parsed.query:
                    path += "?" + parsed.query
                conn_cls = http.client.HTTPSConnection if parsed.scheme == "https" else http.client.HTTPConnection
                conn = conn_cls(host, port, timeout=timeout)
                conn.request("POST", path, body=payload_json.encode("utf-8"), headers={"Content-Type": "application/json"})
                conn.getresponse()
                conn.close()
            except Exception as e:
                log_debug(f"REST transport POST to {url} failed: {e}", log_file)

        # MQTT Transport (optional via paho-mqtt)
        mqtt_cfg = transports.get("mqtt") if isinstance(transports.get("mqtt"), dict) else {}
        if mqtt_cfg.get("enabled", False):
            try:
                import paho.mqtt.publish as publish
                host = mqtt_cfg.get("host", "127.0.0.1")
                port = int(mqtt_cfg.get("port", 1883))
                topic = mqtt_cfg.get("topic", "auradeck/antigravity")
                publish.single(
                    topic,
                    payload=payload_json,
                    hostname=host,
                    port=port,
                    qos=1,
                    retain=True
                )
            except Exception as e:
                log_debug(f"MQTT publish failed: {e}", log_file)

    # Run network transports in background thread so offline backend never blocks CLI
    import threading
    net_thread = threading.Thread(target=run_network, daemon=True)
    net_thread.start()
    net_thread.join(timeout=0.08)


def main():
    parser = argparse.ArgumentParser(description="Antigravity Status Line Bridge for AuraDeck", add_help=False)
    parser.add_argument("--debug-dump", nargs="?", const="__DEFAULT__", default=None,
                        help="Dumps raw stdin JSON to specified or default temporary file.")
    parser.add_argument("--config", type=str, default=None, help="Path to configuration JSON.")
    parser.add_argument("--no-color", action="store_true", help="Disable ANSI color codes.")

    # Parse arguments without raising SystemExit on unrecognized flags
    args, unknown = parser.parse_known_args()

    # Load configuration
    config = load_config(args.config)
    if not isinstance(config, dict):
        config = json.loads(json.dumps(DEFAULT_CONFIG))

    statusline_cfg = config.get("statusline") if isinstance(config.get("statusline"), dict) else {}
    if args.no_color:
        statusline_cfg["use_ansi"] = False
        config["statusline"] = statusline_cfg

    # Read raw stdin
    try:
        raw_input = sys.stdin.read()
    except Exception:
        raw_input = ""

    # Check debug dump flag/env
    debug_cfg = config.get("debug") if isinstance(config.get("debug"), dict) else {}
    dump_target = args.debug_dump or debug_cfg.get("dump_file")
    if dump_target == "__DEFAULT__":
        import tempfile
        dump_target = os.path.join(tempfile.gettempdir(), "cyberdeck_debug_dump.json")
    if dump_target:
        try:
            dump_dir = os.path.dirname(os.path.abspath(dump_target))
            os.makedirs(dump_dir, exist_ok=True)
            with open(dump_target, "w", encoding="utf-8") as f:
                f.write(raw_input)
        except Exception:
            pass

    # If input is empty, output neutral fallback line and exit cleanly
    if not raw_input.strip():
        sys.stdout.write("Antigravity Quota: Unavailable\n")
        sys.stdout.flush()
        sys.exit(0)

    # Parse input JSON
    try:
        parsed_data = parse_antigravity_input(raw_input)
    except Exception:
        parsed_data = {}

    # Format and emit single summary line immediately to stdout
    try:
        summary_line = format_statusline(parsed_data, config)
    except Exception:
        summary_line = "Antigravity Quota: Unavailable"

    sys.stdout.write(summary_line + "\n")
    sys.stdout.flush()

    # Build and dispatch relay transports (only if quota buckets were actually populated)
    if parsed_data and parsed_data.get("status") == "ok" and parsed_data.get("has_quota", False):
        try:
            payload = build_relay_payload(parsed_data)
            execute_transports(payload, config)
        except Exception as e:
            debug_cfg = config.get("debug") if isinstance(config.get("debug"), dict) else {}
            log_file = debug_cfg.get("log_file", "")
            log_debug(f"Transport execution error: {e}", log_file)


if __name__ == "__main__":
    try:
        main()
    except Exception:
        try:
            sys.stdout.write("Antigravity Quota: Unavailable\n")
            sys.stdout.flush()
        except Exception:
            pass
        sys.exit(0)
