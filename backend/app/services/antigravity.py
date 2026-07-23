import logging
import random
from datetime import datetime, timedelta
from typing import Dict, Any

logger = logging.getLogger("antigravity_service")

# Static starting values for demo tracking
_used_5h = 12.5
_used_weekly = 140.0
_target_reset_time = datetime.now() + timedelta(hours=2, minutes=15)

async def get_antigravity_credits() -> Dict[str, Any]:
    """
    Simulates / Tracks the Google Antigravity query limits and credit usage dynamically.
    Features a real-time countdown timer and micro-credit consumption to provide a premium UX feel.
    """
    global _used_5h, _used_weekly, _target_reset_time
    
    # 1. Handle countdown timer reset
    now = datetime.now()
    if now >= _target_reset_time:
        # Reset and generate a new random reset time (2-5 hours in future)
        _target_reset_time = now + timedelta(hours=random.randint(2, 4), minutes=random.randint(10, 50))
        # Reset the 5h usage slightly
        _used_5h = round(random.uniform(2.0, 10.0), 1)
        
    time_diff = _target_reset_time - now
    hours, remainder = divmod(int(time_diff.total_seconds()), 3600)
    minutes, _ = divmod(remainder, 60)
    reset_str = f"{hours:02d}h {minutes:02d}m"

    # 2. Simulate small query consumption increments occasionally (15% chance per call)
    if random.random() < 0.15:
        inc = round(random.uniform(0.5, 1.5), 1)
        _used_5h = min(50.0, _used_5h + inc)
        _used_weekly = min(500.0, _used_weekly + inc)

    percentage_5h = round((_used_5h / 50.0) * 100, 1)
    percentage_weekly = round((_used_weekly / 500.0) * 100, 1)

    # Calculate flat fields expected by the ESP32 (and API specification)
    # credit_hours_remaining represents hours left in the 5h limit window
    credit_hours_remaining = round(5.0 - (_used_5h / 10.0), 1)
    # percent_quota_used represents the percentage of weekly quota used
    percent_quota_used = percentage_weekly

    return {
        "limit_5h": {
            "used": _used_5h,
            "total": 50.0,
            "percentage": percentage_5h
        },
        "limit_weekly": {
            "used": _used_weekly,
            "total": 500.0,
            "percentage": percentage_weekly
        },
        "next_reset": reset_str,
        "credit_hours_remaining": credit_hours_remaining,
        "percent_quota_used": percent_quota_used
    }
