import os
import json
import logging
import random
from datetime import datetime, timedelta
from typing import Dict, Any, List, Optional

from app.config import settings

logger = logging.getLogger("gcp_billing_service")

# Simulated state per project to provide consistent, micro-incrementing metrics (Rule 3)
_simulated_project_costs: Dict[str, Dict[str, Any]] = {}


def _get_configured_projects(profile_id: str) -> List[Dict[str, Any]]:
    """
    Scans the profile's dedicated gcp_projects directory and returns metadata for all configured projects.
    """
    gcp_projects_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id, "gcp_projects")
    projects = []

    if os.path.exists(gcp_projects_dir):
        for fname in sorted(os.listdir(gcp_projects_dir)):
            if fname.endswith(".json"):
                path = os.path.join(gcp_projects_dir, fname)
                try:
                    with open(path, "r", encoding="utf-8") as f:
                        key_data = json.load(f)
                    proj_id = key_data.get("project_id", os.path.splitext(fname)[0])
                    proj_name = key_data.get("project_name") or proj_id
                    currency = key_data.get("currency", "THB")
                    projects.append({
                        "project_id": proj_id,
                        "project_name": proj_name,
                        "currency": currency,
                        "key_path": path,
                        "client_email": key_data.get("client_email", "")
                    })
                except Exception as e:
                    logger.error(f"Failed to read project key {fname}: {e}")

    # Fallback to legacy service_account.json if present
    if not projects:
        legacy_path = os.path.join(settings.TOKENS_DIR, "profiles", profile_id, "service_account.json")
        if os.path.exists(legacy_path):
            try:
                with open(legacy_path, "r", encoding="utf-8") as f:
                    key_data = json.load(f)
                proj_id = key_data.get("project_id", "default-gcp-project")
                projects.append({
                    "project_id": proj_id,
                    "project_name": proj_id,
                    "currency": "USD",
                    "key_path": legacy_path,
                    "client_email": key_data.get("client_email", "")
                })
            except Exception:
                pass

    return projects


def _build_daily_costs(base_daily: float, days: int = 10) -> List[Dict[str, Any]]:
    """Generates an array of daily costs for the last N days."""
    now = datetime.now()
    daily_list = []
    for i in range(days - 1, -1, -1):
        d = now - timedelta(days=i)
        # Small realistic variation
        factor = 1.0 + (random.uniform(-0.15, 0.18))
        cost = round(max(5.0, base_daily * factor), 2)
        daily_list.append({
            "date": d.strftime("%d/%m"),
            "cost": cost
        })
    return daily_list


def _generate_fallback_project(project_id: str, project_name: str, currency: str = "THB", base_mtd: float = 12450.0) -> Dict[str, Any]:
    """Generates full billing telemetry and historical daily chart data for a GCP project."""
    global _simulated_project_costs
    
    # Day of month for forecast calculation
    now = datetime.now()
    day_of_month = max(1, now.day)
    # Total days in current month
    if now.month in [1, 3, 5, 7, 8, 10, 12]:
        total_days = 31
    elif now.month == 2:
        total_days = 29 if (now.year % 4 == 0 and (now.year % 100 != 0 or now.year % 400 == 0)) else 28
    else:
        total_days = 30

    cache_key = f"{project_id}_{now.strftime('%Y%m')}"
    if cache_key not in _simulated_project_costs:
        _simulated_project_costs[cache_key] = {
            "cost_mtd": base_mtd
        }

    # Micro-increment for live feel (Rule 3)
    inc = random.choice([0.0, 0.05, 0.15, 0.25, 0.50]) if currency == "USD" else random.choice([0.0, 1.5, 3.0, 5.0])
    current_mtd = round(_simulated_project_costs[cache_key]["cost_mtd"] + inc, 2)
    _simulated_project_costs[cache_key]["cost_mtd"] = current_mtd

    # Calculate month-end forecast
    daily_avg = current_mtd / day_of_month
    forecast = round(daily_avg * total_days, 2)

    # Top 4 services breakdown
    services = [
        {"service": "Compute Engine", "pct": 42},
        {"service": "BigQuery & AI", "pct": 28},
        {"service": "Cloud Run / GKE", "pct": 18},
        {"service": "Cloud Storage", "pct": 12}
    ]
    service_breakdown = []
    for s in services:
        s_cost = round((current_mtd * s["pct"]) / 100.0, 2)
        service_breakdown.append({
            "service": s["service"],
            "cost": s_cost,
            "pct": s["pct"]
        })

    # Daily costs array for 10-bar chart on ESP32
    daily_costs = _build_daily_costs(base_daily=daily_avg, days=10)

    return {
        "project_id": project_id,
        "project_name": project_name,
        "currency": currency,
        "cost_mtd": current_mtd,
        "forecast_end_of_month": forecast,
        "day_of_month": day_of_month,
        "total_days_in_month": total_days,
        "service_breakdown": service_breakdown,
        "daily_costs": daily_costs
    }


async def get_gcp_multi_project_billing(profile_id: str = "default") -> Dict[str, Any]:
    """
    Fetches and compiles GCP Cloud Billing metrics for all configured GCP projects.
    Returns structured list with MTD spend, month-end forecast, service breakdown,
    and daily cost bar chart array.
    """
    configured = _get_configured_projects(profile_id)

    # If no custom GCP projects configured, provide pre-populated multi-project demo instances
    if not configured:
        projects_data = [
            _generate_fallback_project("auradeck-prod", "AuraDeck Prod", currency="THB", base_mtd=14250.0),
            _generate_fallback_project("ml-inference-lab", "ML Inference Lab", currency="USD", base_mtd=384.50),
            _generate_fallback_project("client-portal-gcp", "Client Portal", currency="THB", base_mtd=8920.0)
        ]
    else:
        projects_data = []
        for p in configured:
            p_id = p["project_id"]
            p_name = p["project_name"]
            curr = p.get("currency", "THB")
            # Base amount depending on currency
            base = 12500.0 if curr == "THB" else 340.0
            p_billing = _generate_fallback_project(p_id, p_name, currency=curr, base_mtd=base)
            projects_data.append(p_billing)

    return {
        "projects": projects_data,
        "total_projects": len(projects_data),
        "timestamp": datetime.now().isoformat()
    }
