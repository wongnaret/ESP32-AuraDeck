import os
import json
import logging
import random
from datetime import datetime, timedelta
from typing import Dict, Any, List, Optional
import httpx

from app.config import settings

logger = logging.getLogger("gcp_billing_service")

# Simulated state per project to provide consistent fallback metrics (Rule 3)
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
        factor = 1.0 + (random.uniform(-0.15, 0.18))
        cost = round(max(5.0, base_daily * factor), 2)
        daily_list.append({
            "date": d.strftime("%d/%m"),
            "cost": cost
        })
    return daily_list


def _parse_bigquery_billing_rows(query_response: Dict[str, Any]) -> List[Dict[str, Any]]:
    """Parses standard BigQuery REST API rows into a list of clean dictionaries."""
    fields = [f.get("name") for f in query_response.get("schema", {}).get("fields", [])]
    rows = query_response.get("rows", [])
    records = []
    for r in rows:
        row_dict = {}
        for idx, col in enumerate(r.get("f", [])):
            if idx < len(fields):
                val = col.get("v")
                fname = fields[idx]
                if fname in ("gross_cost", "credits", "net_cost"):
                    try:
                        val = float(val) if val is not None else 0.0
                    except (ValueError, TypeError):
                        val = 0.0
                row_dict[fname] = val
        records.append(row_dict)
    return records


def _compile_project_billing_from_records(
    project_id: str,
    project_name: str,
    records: List[Dict[str, Any]],
    default_currency: str = "THB"
) -> Dict[str, Any]:
    """Compiles parsed BigQuery records into unified AuraDeck billing telemetry."""
    now = datetime.now()
    day_of_month = max(1, now.day)
    if now.month in [1, 3, 5, 7, 8, 10, 12]:
        total_days = 31
    elif now.month == 2:
        total_days = 29 if (now.year % 4 == 0 and (now.year % 100 != 0 or now.year % 400 == 0)) else 28
    else:
        total_days = 30

    first_day_str = now.strftime("%Y-%m-01")
    currency = default_currency

    mtd_gross = 0.0
    mtd_credits = 0.0
    mtd_net = 0.0
    services_spend: Dict[str, float] = {}

    # Map daily costs by YYYY-MM-DD
    daily_map: Dict[str, float] = {}

    for rec in records:
        rec_date = str(rec.get("usage_date", ""))
        rec_curr = rec.get("currency")
        if rec_curr:
            currency = rec_curr

        net = float(rec.get("net_cost", 0.0))
        gross = float(rec.get("gross_cost", 0.0))
        cred = float(rec.get("credits", 0.0))
        svc = rec.get("service_name") or "Other Services"

        # Month-to-date aggregation
        if rec_date >= first_day_str:
            mtd_gross += gross
            mtd_credits += abs(cred)
            mtd_net += net
            services_spend[svc] = services_spend.get(svc, 0.0) + max(0.0, net)

        # Daily spend map
        daily_map[rec_date] = daily_map.get(rec_date, 0.0) + max(0.0, net)

    mtd_net = round(max(0.0, mtd_net), 2)
    mtd_gross = round(max(0.0, mtd_gross), 2)
    mtd_credits = round(max(0.0, mtd_credits), 2)

    # Forecast calculation
    daily_avg = mtd_net / day_of_month if day_of_month > 0 else 0.0
    forecast = round(daily_avg * total_days, 2)
    forecast = max(mtd_net, forecast)

    # Top 4 Services breakdown
    sorted_services = sorted(services_spend.items(), key=lambda x: x[1], reverse=True)
    total_service_spend = sum(services_spend.values()) if services_spend else 1.0
    service_breakdown = []
    for svc_name, cost in sorted_services[:4]:
        pct = int(round((cost / total_service_spend) * 100)) if total_service_spend > 0 else 0
        service_breakdown.append({
            "service": svc_name,
            "cost": round(cost, 2),
            "pct": pct
        })

    # Fill default services if less than 4
    if not service_breakdown:
        service_breakdown.append({"service": "All Cloud Services", "cost": mtd_net, "pct": 100})

    # 10-day daily costs array
    daily_costs = []
    for i in range(9, -1, -1):
        target_dt = now - timedelta(days=i)
        d_key = target_dt.strftime("%Y-%m-%d")
        d_label = target_dt.strftime("%d/%m")
        d_cost = round(daily_map.get(d_key, 0.0), 2)
        daily_costs.append({
            "date": d_label,
            "cost": d_cost
        })

    return {
        "project_id": project_id,
        "project_name": project_name,
        "currency": currency,
        "cost_mtd": mtd_net,
        "gross_cost_mtd": mtd_gross,
        "savings_mtd": mtd_credits,
        "forecast_end_of_month": forecast,
        "day_of_month": day_of_month,
        "total_days_in_month": total_days,
        "service_breakdown": service_breakdown,
        "daily_costs": daily_costs,
        "source": "bigquery_billing_export",
        "is_live": True
    }


async def _fetch_bigquery_billing(
    project_id: str,
    project_name: str,
    key_path: str,
    default_currency: str = "THB",
    timeout: float = 8.0
) -> Optional[Dict[str, Any]]:
    """
    Attempts to discover and query the BigQuery billing export table.
    Returns real telemetry dict if found, or None if the table is not yet created.
    """
    try:
        from google.oauth2 import service_account
        from google.auth.transport.requests import Request

        scopes = [
            "https://www.googleapis.com/auth/bigquery",
            "https://www.googleapis.com/auth/cloud-platform"
        ]
        creds = service_account.Credentials.from_service_account_file(key_path, scopes=scopes)
        creds.refresh(Request())
        token = creds.token

        candidate_datasets = ["billing_export", "gcp_billing", "billing"]
        found_table = None
        found_dataset = None

        async with httpx.AsyncClient(timeout=timeout) as client:
            headers = {"Authorization": f"Bearer {token}", "Content-Type": "application/json"}

            # 1. Discover table in candidate datasets
            for ds in candidate_datasets:
                tables_url = f"https://bigquery.googleapis.com/bigquery/v2/projects/{project_id}/datasets/{ds}/tables"
                resp = await client.get(tables_url, headers=headers)
                if resp.status_code == 200:
                    tables = resp.json().get("tables", [])
                    for t in tables:
                        t_id = t.get("tableReference", {}).get("tableId", "")
                        if t_id.startswith("gcp_billing_export_v1_"):
                            found_table = t_id
                            found_dataset = ds
                            break
                if found_table:
                    break

            if not found_table:
                logger.debug(f"BigQuery billing export table not yet created in project '{project_id}'")
                return None

            # 2. Query billing data
            query_sql = f"""
            SELECT
                DATE(usage_start_time, 'Asia/Bangkok') AS usage_date,
                service.description AS service_name,
                currency,
                ROUND(SUM(cost), 2) AS gross_cost,
                ROUND(SUM(IFNULL((SELECT SUM(c.amount) FROM UNNEST(credits) c), 0)), 2) AS credits,
                ROUND(SUM(cost) + SUM(IFNULL((SELECT SUM(c.amount) FROM UNNEST(credits) c), 0)), 2) AS net_cost
            FROM `{project_id}.{found_dataset}.{found_table}`
            WHERE usage_start_time >= TIMESTAMP_TRUNC(CURRENT_TIMESTAMP(), MONTH)
               OR usage_start_time >= TIMESTAMP_SUB(CURRENT_TIMESTAMP(), INTERVAL 10 DAY)
            GROUP BY 1, 2, 3
            ORDER BY usage_date ASC
            """
            queries_url = f"https://bigquery.googleapis.com/bigquery/v2/projects/{project_id}/queries"
            body = {
                "query": query_sql,
                "useLegacySql": False,
                "timeoutMs": int(timeout * 1000)
            }
            q_resp = await client.post(queries_url, headers=headers, json=body)
            if q_resp.status_code == 200:
                q_json = q_resp.json()
                records = _parse_bigquery_billing_rows(q_json)
                logger.info(f"✅ Successfully queried {len(records)} live billing records from BigQuery table '{found_table}'")
                return _compile_project_billing_from_records(
                    project_id=project_id,
                    project_name=project_name,
                    records=records,
                    default_currency=default_currency
                )
            else:
                logger.warning(f"BigQuery query failed ({q_resp.status_code}): {q_resp.text}")
                return None
    except Exception as e:
        logger.debug(f"BigQuery billing lookup bypassed for '{project_id}': {e}")
        return None


def _generate_fallback_project(project_id: str, project_name: str, currency: str = "THB", base_mtd: float = 217.0) -> Dict[str, Any]:
    """Generates fallback billing telemetry while BigQuery export is provisioning (Rule 3)."""
    global _simulated_project_costs
    
    now = datetime.now()
    day_of_month = max(1, now.day)
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

    inc = random.choice([0.0, 0.05, 0.15]) if currency == "USD" else random.choice([0.0, 0.5, 1.0])
    current_mtd = round(_simulated_project_costs[cache_key]["cost_mtd"] + inc, 2)
    _simulated_project_costs[cache_key]["cost_mtd"] = current_mtd

    daily_avg = current_mtd / day_of_month
    forecast = round(daily_avg * total_days, 2)

    services = [
        {"service": "Gemini 3.8 & Vertex AI", "pct": 45},
        {"service": "Compute Engine", "pct": 25},
        {"service": "Cloud Run", "pct": 18},
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
        "daily_costs": daily_costs,
        "source": "pending_export",
        "is_live": False
    }


async def get_gcp_multi_project_billing(profile_id: str = "default") -> Dict[str, Any]:
    """
    Fetches and compiles GCP Cloud Billing metrics for all configured GCP projects.
    Attempts live BigQuery billing export query first, with graceful fallback if table is pending.
    """
    configured = _get_configured_projects(profile_id)

    if not configured:
        projects_data = [
            _generate_fallback_project("auradeck-prod", "AuraDeck Prod", currency="THB", base_mtd=217.0),
            _generate_fallback_project("ml-inference-lab", "ML Inference Lab", currency="USD", base_mtd=12.50)
        ]
    else:
        projects_data = []
        for p in configured:
            p_id = p["project_id"]
            p_name = p["project_name"]
            curr = p.get("currency", "THB")
            key_path = p.get("key_path", "")

            # 1. Try querying live BigQuery export
            live_data = None
            if key_path and os.path.exists(key_path):
                live_data = await _fetch_bigquery_billing(
                    project_id=p_id,
                    project_name=p_name,
                    key_path=key_path,
                    default_currency=curr
                )

            # 2. If BigQuery table is live, use real data; otherwise graceful fallback
            if live_data:
                projects_data.append(live_data)
            else:
                base = 217.0 if curr == "THB" else 15.0
                p_billing = _generate_fallback_project(p_id, p_name, currency=curr, base_mtd=base)
                projects_data.append(p_billing)

    return {
        "projects": projects_data,
        "total_projects": len(projects_data),
        "timestamp": datetime.now().isoformat()
    }

