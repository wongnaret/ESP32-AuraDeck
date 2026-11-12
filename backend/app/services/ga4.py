import os
import json
import asyncio
import logging
import random
import httpx
from typing import Dict, Any, List, Optional

from app.config import settings

logger = logging.getLogger("ga4_service")

# Module-level cache and simulated state per property for smooth fallback (Rule 3)
_simulated_property_states: Dict[str, Dict[str, Any]] = {}


def _get_configured_ga4_properties(profile_id: str) -> List[Dict[str, Any]]:
    """
    Scans the profile's dedicated ga4_properties directory and returns metadata for all configured properties.
    """
    ga4_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id, "ga4_properties")
    properties = []

    if os.path.exists(ga4_dir):
        for fname in sorted(os.listdir(ga4_dir)):
            if fname.endswith(".json"):
                path = os.path.join(ga4_dir, fname)
                try:
                    with open(path, "r", encoding="utf-8") as f:
                        key_data = json.load(f)
                    prop_id = str(key_data.get("property_id") or os.path.splitext(fname)[0])
                    prop_name = key_data.get("property_name") or f"Property {prop_id}"
                    properties.append({
                        "property_id": prop_id,
                        "property_name": prop_name,
                        "key_path": path,
                        "client_email": key_data.get("client_email", "")
                    })
                except Exception as e:
                    logger.error(f"Failed to read GA4 property key {fname}: {e}")

    # Fallback to single ga_property_id from settings.json if no multi-properties configured
    if not properties:
        from app.services.google_auth import load_profile_settings
        prof_settings = load_profile_settings(profile_id)
        single_prop = prof_settings.get("ga_property_id") or settings.GA4_PROPERTY_ID
        if single_prop and single_prop != "mock_property_id":
            properties.append({
                "property_id": str(single_prop),
                "property_name": "Main Website",
                "key_path": None,
                "client_email": ""
            })

    return properties


def _get_ga4_access_token_for_property(profile_id: str, key_path: Optional[str] = None) -> Optional[str]:
    """
    Attempts to get a valid OAuth access token for GA4 using:
    1. Dedicated Service Account JSON key for this property
    2. GCP Project Service Account Key (Priority 2)
    3. Profile general Service Account Key (Priority 3)
    4. Google User OAuth2 Refresh Token (Priority 4)
    """
    # 1. Direct key_path provided
    sa_path = key_path if (key_path and os.path.exists(key_path)) else None

    # 2. Try GCP Projects Service Account
    if not sa_path:
        gcp_projects_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id, "gcp_projects")
        if os.path.exists(gcp_projects_dir):
            files = [f for f in os.listdir(gcp_projects_dir) if f.endswith(".json")]
            if files:
                sa_path = os.path.join(gcp_projects_dir, files[0])

    # 3. Try Profile SA Key
    if not sa_path:
        profile_sa = os.path.join(settings.TOKENS_DIR, "profiles", profile_id, "service_account.json")
        if os.path.exists(profile_sa):
            sa_path = profile_sa

    if not sa_path and settings.GCP_SERVICE_ACCOUNT_PATH and os.path.exists(settings.GCP_SERVICE_ACCOUNT_PATH):
        sa_path = settings.GCP_SERVICE_ACCOUNT_PATH

    if sa_path and os.path.exists(sa_path):
        try:
            from google.oauth2 import service_account
            from google.auth.transport.requests import Request
            scopes = ["https://www.googleapis.com/auth/analytics.readonly"]
            creds = service_account.Credentials.from_service_account_file(sa_path, scopes=scopes)
            creds.refresh(Request())
            if creds.token:
                return creds.token
        except Exception as e:
            logger.error(f"Service account token refresh failed ({sa_path}): {e}")

    # 4. Try Google User OAuth credentials
    try:
        from app.services.google_auth import GoogleAuthService
        auth_service = GoogleAuthService(profile_id)
        token = auth_service.get_valid_token()
        if token:
            return token
    except Exception as e:
        logger.debug(f"User OAuth token fetch failed for profile {profile_id}: {e}")

    return None


def _format_count(num: int) -> str:
    """Formats large integer counts into compact strings like 12.4K or 1.2M."""
    if num >= 1_000_000:
        return f"{num / 1_000_000:.1f}M"
    if num >= 1_000:
        return f"{num / 1_000:.1f}K"
    return str(num)


def _format_duration(seconds: float) -> str:
    """Formats duration in seconds into 'Xm Ys' format."""
    total_sec = int(seconds)
    mins = total_sec // 60
    secs = total_sec % 60
    return f"{mins}m {secs:02d}s"


def _generate_fallback_ga4_property(prop_id: str, prop_name: str) -> Dict[str, Any]:
    """Generates realistic, fluctuating GA4 metrics for preview or fallback (Rule 3)."""
    global _simulated_property_states
    
    if prop_id not in _simulated_property_states:
        base_30m = random.randint(2, 18)
        _simulated_property_states[prop_id] = {
            "active_users_30m": base_30m,
            "active_7d_users": "139",
            "new_users": "69",
            "sessions": "216",
            "page_views": "669",
            "avg_engagement_time": "2m 18s",
            "bounce_rate": "26.2%",
            "event_count": "2.1K"
        }

    st = _simulated_property_states[prop_id]
    fluctuation = random.choice([-1, 0, 1])
    cur_30m = max(0, min(250, st["active_users_30m"] + fluctuation))
    st["active_users_30m"] = cur_30m

    city_list = [
        {"city": "Bangkok", "active_users": 47},
        {"city": "Pattaya City", "active_users": 13},
        {"city": "Surat Thani", "active_users": 6},
        {"city": "Nakhon Ratchasima", "active_users": 5},
        {"city": "Prachuap Khiri Khan", "active_users": 5}
    ]
    device_list = [
        {"device": "Mobile", "active_users": 73, "pct": 68.2},
        {"device": "Desktop", "active_users": 33, "pct": 30.8},
        {"device": "Tablet", "active_users": 1, "pct": 0.9}
    ]

    return {
        "property_id": prop_id,
        "property_name": prop_name,
        "is_demo": True,
        "active_users_30m": cur_30m,
        "active_7d_users": st["active_7d_users"],
        "active_28d_users": st["active_7d_users"],
        "new_users": st["new_users"],
        "sessions": st["sessions"],
        "page_views": st["page_views"],
        "avg_engagement_time": st["avg_engagement_time"],
        "bounce_rate": st["bounce_rate"],
        "event_count": st["event_count"],
        "top_cities": city_list,
        "devices": device_list,
        "daily_trend": [68, 90, 80, 65, 102, 165, 82]
    }


async def _fetch_single_property_analytics(profile_id: str, prop: Dict[str, Any]) -> Dict[str, Any]:
    """Fetches 30m real-time active users and 7-day analytics reports for a single GA4 property."""
    prop_id = prop["property_id"]
    prop_name = prop["property_name"]
    key_path = prop.get("key_path")

    token = _get_ga4_access_token_for_property(profile_id, key_path)
    if not token or not prop_id or prop_id == "mock_property_id":
        return _generate_fallback_ga4_property(prop_id, prop_name)

    headers = {"Authorization": f"Bearer {token}", "Content-Type": "application/json"}
    realtime_url = f"https://analyticsdata.googleapis.com/v1beta/properties/{prop_id}:runRealtimeReport"
    report_url = f"https://analyticsdata.googleapis.com/v1beta/properties/{prop_id}:runReport"

    # 1. Realtime (30 minutes) - active users only (0 is a valid live state!)
    realtime_payload = {
        "metrics": [{"name": "activeUsers"}],
        "minuteRanges": [{"name": "0-29 minutes ago", "startMinutesAgo": 29}]
    }

    # 2. 7-Day Summary Metrics
    summary_payload = {
        "dateRanges": [{"startDate": "7daysAgo", "endDate": "yesterday"}],
        "metrics": [
            {"name": "activeUsers"},
            {"name": "newUsers"},
            {"name": "sessions"},
            {"name": "screenPageViews"},
            {"name": "userEngagementDuration"},
            {"name": "bounceRate"},
            {"name": "eventCount"}
        ]
    }

    # 3. 7-Day Top Cities
    cities_payload = {
        "dateRanges": [{"startDate": "7daysAgo", "endDate": "yesterday"}],
        "dimensions": [{"name": "city"}],
        "metrics": [{"name": "activeUsers"}],
        "orderBys": [{"metric": {"metricName": "activeUsers"}, "desc": True}],
        "limit": 5
    }

    # 4. 7-Day Device Breakdown
    devices_payload = {
        "dateRanges": [{"startDate": "7daysAgo", "endDate": "yesterday"}],
        "dimensions": [{"name": "deviceCategory"}],
        "metrics": [{"name": "activeUsers"}],
        "orderBys": [{"metric": {"metricName": "activeUsers"}, "desc": True}],
        "limit": 3
    }

    # 5. 7-Day Daily Traffic Trend (Page Views per day)
    trend_payload = {
        "dateRanges": [{"startDate": "7daysAgo", "endDate": "yesterday"}],
        "dimensions": [{"name": "date"}],
        "metrics": [{"name": "screenPageViews"}],
        "orderBys": [{"dimension": {"dimensionName": "date"}, "desc": False}]
    }

    realtime_ok = False
    report_ok = False
    active_users_30m = 0

    active_7d_users_str = "0"
    new_users_str = "0"
    sessions_str = "0"
    page_views_str = "0"
    avg_engagement_str = "0m 00s"
    bounce_rate_str = "0.0%"
    event_count_str = "0"
    top_cities: List[Dict[str, Any]] = []
    devices: List[Dict[str, Any]] = []
    daily_trend: List[int] = []

    async with httpx.AsyncClient(timeout=8.0) as client:
        results = await asyncio.gather(
            client.post(realtime_url, headers=headers, json=realtime_payload),
            client.post(report_url, headers=headers, json=summary_payload),
            client.post(report_url, headers=headers, json=cities_payload),
            client.post(report_url, headers=headers, json=devices_payload),
            client.post(report_url, headers=headers, json=trend_payload),
            return_exceptions=True
        )

    resp_rt, resp_sum, resp_cities, resp_dev, resp_trend = results

    # Parse 1: Realtime 30m
    if isinstance(resp_rt, Exception):
        logger.error(f"Error querying GA4 Realtime API for property {prop_id}: {resp_rt}")
    elif resp_rt.status_code == 200:
        realtime_ok = True
        rows = resp_rt.json().get("rows", [])
        if rows and rows[0].get("metricValues"):
            active_users_30m = int(float(rows[0]["metricValues"][0].get("value", "0")))
    else:
        logger.warning(f"GA4 Realtime API status {resp_rt.status_code} for property {prop_id}: {resp_rt.text}")

    # Parse 2: 7-Day Summary
    if isinstance(resp_sum, Exception):
        logger.error(f"Error querying GA4 7D Summary API for property {prop_id}: {resp_sum}")
    elif resp_sum.status_code == 200:
        report_ok = True
        rows = resp_sum.json().get("rows", [])
        if rows and rows[0].get("metricValues"):
            mv = rows[0]["metricValues"]
            u7 = int(float(mv[0].get("value", "0"))) if len(mv) > 0 else 0
            nu = int(float(mv[1].get("value", "0"))) if len(mv) > 1 else 0
            sess = int(float(mv[2].get("value", "0"))) if len(mv) > 2 else 0
            views = int(float(mv[3].get("value", "0"))) if len(mv) > 3 else 0
            dur_total = float(mv[4].get("value", "0.0")) if len(mv) > 4 else 0.0
            bounce_ratio = float(mv[5].get("value", "0.0")) if len(mv) > 5 else 0.0
            events = int(float(mv[6].get("value", "0"))) if len(mv) > 6 else 0

            active_7d_users_str = _format_count(u7)
            new_users_str = _format_count(nu)
            sessions_str = _format_count(sess)
            page_views_str = _format_count(views)
            event_count_str = _format_count(events)

            avg_sec = (dur_total / u7) if u7 > 0 else 0.0
            avg_engagement_str = _format_duration(avg_sec) if avg_sec > 0 else "0m 00s"
            bounce_rate_str = f"{bounce_ratio * 100.0:.1f}%"
    else:
        logger.warning(f"GA4 7D Summary API status {resp_sum.status_code} for property {prop_id}: {resp_sum.text}")

    # Only fall back to simulated demo data if BOTH realtime and 7-day summary APIs failed
    if not realtime_ok and not report_ok:
        return _generate_fallback_ga4_property(prop_id, prop_name)

    # Parse 3: 7-Day Top Cities
    if isinstance(resp_cities, Exception):
        logger.error(f"Error querying GA4 Top Cities API for property {prop_id}: {resp_cities}")
    elif resp_cities.status_code == 200:
        for r in resp_cities.json().get("rows", []):
            c_name = r.get("dimensionValues", [{}])[0].get("value", "Unknown")
            c_users = int(float(r.get("metricValues", [{}])[0].get("value", "0")))
            top_cities.append({"city": c_name, "active_users": c_users})
    else:
        logger.warning(f"GA4 Top Cities API status {resp_cities.status_code} for property {prop_id}: {resp_cities.text}")

    # Parse 4: 7-Day Device Breakdown
    if isinstance(resp_dev, Exception):
        logger.error(f"Error querying GA4 Devices API for property {prop_id}: {resp_dev}")
    elif resp_dev.status_code == 200:
        raw_devs = []
        total_dev_users = 0
        for r in resp_dev.json().get("rows", []):
            d_name = r.get("dimensionValues", [{}])[0].get("value", "other").capitalize()
            d_users = int(float(r.get("metricValues", [{}])[0].get("value", "0")))
            raw_devs.append((d_name, d_users))
            total_dev_users += d_users
        for d_name, d_users in raw_devs:
            pct = round((d_users * 100.0 / total_dev_users), 1) if total_dev_users > 0 else 0.0
            devices.append({"device": d_name, "active_users": d_users, "pct": pct})
    else:
        logger.warning(f"GA4 Devices API status {resp_dev.status_code} for property {prop_id}: {resp_dev.text}")

    # Parse 5: 7-Day Daily Trend
    if isinstance(resp_trend, Exception):
        logger.error(f"Error querying GA4 Trend API for property {prop_id}: {resp_trend}")
    elif resp_trend.status_code == 200:
        for r in resp_trend.json().get("rows", []):
            v = int(float(r.get("metricValues", [{}])[0].get("value", "0")))
            daily_trend.append(v)
    else:
        logger.warning(f"GA4 Trend API status {resp_trend.status_code} for property {prop_id}: {resp_trend.text}")

    return {
        "property_id": prop_id,
        "property_name": prop_name,
        "is_demo": False,
        "active_users_30m": active_users_30m,
        "active_7d_users": active_7d_users_str,
        "active_28d_users": active_7d_users_str,
        "new_users": new_users_str,
        "sessions": sessions_str,
        "page_views": page_views_str,
        "avg_engagement_time": avg_engagement_str,
        "bounce_rate": bounce_rate_str,
        "event_count": event_count_str,
        "top_cities": top_cities,
        "devices": devices,
        "daily_trend": daily_trend
    }


async def get_ga4_analytics(profile_id: str = "default") -> Dict[str, Any]:
    """
    Fetches 30-minute real-time and 7-day analytics for all configured GA4 properties.
    Returns complete multi-property structure for ESP32 Screen 6 (GA4).
    """
    props = _get_configured_ga4_properties(profile_id)
    if not props:
        fallback = _generate_fallback_ga4_property("default", "AuraDeck Demo")
        return {
            "total_properties": 1,
            "properties": [fallback],
            **fallback  # Backward compatibility with single-property parsers
        }

    results = []
    for p in props:
        data = await _fetch_single_property_analytics(profile_id, p)
        results.append(data)

    primary = results[0] if results else _generate_fallback_ga4_property("default", "AuraDeck Demo")

    return {
        "total_properties": len(results),
        "properties": results,
        **primary  # Backward compatibility
    }


async def test_ga4_connection(profile_id: str, property_id: str, key_path: Optional[str] = None) -> Dict[str, Any]:
    """
    Tests direct connection and authentication permissions with GA4 for the given Property ID.
    Returns status, error details, and sample active user count.
    """
    # If key_path is not given, search in configured ga4_properties
    if not key_path:
        props = _get_configured_ga4_properties(profile_id)
        for p in props:
            if p["property_id"] == property_id and p.get("key_path"):
                key_path = p["key_path"]
                break

    token = _get_ga4_access_token_for_property(profile_id, key_path)
    if not token:
        msg = "No valid Google Service Account Key or Google User OAuth credentials found for this property."
        logger.warning(f"GA4 test failed for profile '{profile_id}' property '{property_id}': {msg}")
        return {
            "success": False,
            "error": msg
        }

    url = f"https://analyticsdata.googleapis.com/v1beta/properties/{property_id}:runRealtimeReport"
    headers = {"Authorization": f"Bearer {token}", "Content-Type": "application/json"}
    payload = {
        "metrics": [{"name": "activeUsers"}],
        "minuteRanges": [{"name": "0-29 minutes ago", "startMinutesAgo": 29}]
    }

    try:
        async with httpx.AsyncClient() as client:
            resp = await client.post(url, headers=headers, json=payload, timeout=8.0)
            if resp.status_code == 200:
                data = resp.json()
                rows = data.get("rows", [])
                users = 0
                if rows and rows[0].get("metricValues"):
                    users = int(rows[0]["metricValues"][0].get("value", "0"))
                return {
                    "success": True,
                    "message": f"Successfully connected to GA4 Property {property_id}! Active users: {users}",
                    "active_users_30m": users
                }
            elif resp.status_code == 403:
                msg = "Permission Denied (403). Please make sure your Service Account email is added as a 'Viewer' in GA4 Property Access Management."
                logger.warning(f"GA4 test 403 for property '{property_id}': {resp.text}")
                return {
                    "success": False,
                    "error": msg
                }
            elif resp.status_code == 404:
                msg = f"Property Not Found (404). Please verify that GA4 Property ID '{property_id}' is correct (use numbers only, e.g. 453120000)."
                logger.warning(f"GA4 test 404 for property '{property_id}': {resp.text}")
                return {
                    "success": False,
                    "error": msg
                }
            else:
                msg = f"GA4 API returned HTTP {resp.status_code}: {resp.text}"
                logger.warning(f"GA4 test failed for property '{property_id}': {msg}")
                return {
                    "success": False,
                    "error": msg
                }
    except Exception as e:
        logger.error(f"GA4 test connection error for property '{property_id}': {e}")
        return {
            "success": False,
            "error": f"Connection error: {str(e)}"
        }
