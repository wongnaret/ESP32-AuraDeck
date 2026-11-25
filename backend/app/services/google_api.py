import logging
import asyncio
import httpx
from urllib.parse import quote
from datetime import datetime, timedelta, date, timezone
from typing import Dict, Any, List, Optional

from app.services.google_auth import ProfileTokenManager, refresh_google_token, load_profile_settings

logger = logging.getLogger("google_api_service")

async def get_google_task_lists(profile_id: str) -> List[Dict[str, Any]]:
    """
    Fetches all available Google Task lists for a specific profile.
    Utilizes access token auto-refresh and returning robust fallbacks.
    """
    mgr = ProfileTokenManager(profile_id, "Google")
    tokens = mgr.load_tokens()
    if not tokens or "access_token" not in tokens:
        logger.debug(f"Google credentials are not configured or missing for profile {profile_id}.")
        return []

    access_token = tokens["access_token"]
    url = "https://tasks.googleapis.com/tasks/v1/users/@me/lists"

    async def make_request(tok: str) -> httpx.Response:
        headers = {"Authorization": f"Bearer {tok}"}
        async with httpx.AsyncClient() as client:
            return await client.get(url, headers=headers, timeout=5.0)

    try:
        res = await make_request(access_token)
        if res.status_code == 401:
            logger.info(f"Google access token expired for profile {profile_id}. Attempting token refresh...")
            new_tok = await refresh_google_token(profile_id)
            if new_tok:
                res = await make_request(new_tok)
            else:
                return []

        if res.status_code == 200:
            data = res.json()
            return [{"id": item["id"], "title": item["title"]} for item in data.get("items", [])]
        else:
            logger.warning(f"Google Tasks lists API returned status {res.status_code} for profile {profile_id}")
            return []
    except Exception as e:
        logger.error(f"Error fetching Google Task lists for profile {profile_id}: {e}")
        return []


async def get_google_calendars(profile_id: str) -> List[Dict[str, Any]]:
    """
    Fetches all non-hidden Google Calendars (Primary, Shared, Subscribed) for a profile.
    """
    mgr = ProfileTokenManager(profile_id, "Google")
    tokens = mgr.load_tokens()
    if not tokens or "access_token" not in tokens:
        logger.debug(f"Google credentials are not configured or missing for profile {profile_id}.")
        return []

    access_token = tokens["access_token"]
    url = "https://www.googleapis.com/calendar/v3/users/me/calendarList"
    params = {"showHidden": "false"}

    async def make_request(tok: str) -> httpx.Response:
        headers = {"Authorization": f"Bearer {tok}"}
        async with httpx.AsyncClient() as client:
            return await client.get(url, headers=headers, params=params, timeout=5.0)

    try:
        res = await make_request(access_token)
        if res.status_code == 401:
            logger.info(f"Google access token expired for profile {profile_id}. Attempting token refresh...")
            new_tok = await refresh_google_token(profile_id)
            if new_tok:
                res = await make_request(new_tok)
            else:
                return []

        if res.status_code == 200:
            data = res.json()
            calendars = []
            for item in data.get("items", []):
                if item.get("hidden") or item.get("deleted"):
                    continue
                cal_id = item.get("id")
                if not cal_id:
                    continue
                summary = item.get("summaryOverride") or item.get("summary") or cal_id
                calendars.append({
                    "id": cal_id,
                    "summary": summary,
                    "primary": bool(item.get("primary", False)),
                    "backgroundColor": item.get("backgroundColor", "#3b82f6"),
                    "accessRole": item.get("accessRole", "reader"),
                })
            calendars.sort(key=lambda c: (0 if c["primary"] else 1, c["summary"].lower()))
            return calendars
        else:
            logger.warning(f"Google CalendarList API returned status {res.status_code} for profile {profile_id}")
            return []
    except Exception as e:
        logger.error(f"Error fetching Google Calendars for profile {profile_id}: {e}")
        return []


def parse_calendar_items(
    items: List[Dict[str, Any]],
    today_local: date,
    max_events: int = 15,
) -> Dict[str, Any]:
    """
    Parses raw Google Calendar API items (from one or multiple calendars) into AuraDeck's calendar payload.
    Only includes today and future events (no past events):
      - Deduplicates identical events across calendars (`iCalUID` + date or `(date, time, title)`).
      - Sorts chronologically with all-day events first per date.
      - `month_days_with_events`: sorted day numbers in the current month (>= today).
      - `events`: up to `max_events` upcoming events with date labels.
    """
    month_days_set = set()
    parsed_entries = []
    seen_keys = set()

    for item in items:
        summary = item.get("summary", "No Title")
        start = item.get("start", {})
        end = item.get("end", {})

        start_dt_str = start.get("dateTime") or start.get("date")
        if not start_dt_str:
            continue

        is_all_day = "dateTime" not in start

        if is_all_day:
            event_date = datetime.strptime(start_dt_str[:10], "%Y-%m-%d").date()
            # Handle multi-day all-day events that started before today but still span today
            end_date_str = end.get("date")
            if event_date < today_local and end_date_str:
                end_exclusive = datetime.strptime(end_date_str[:10], "%Y-%m-%d").date()
                if end_exclusive > today_local:
                    event_date = today_local
            event_time_str = "All Day"
        else:
            dt_part = start_dt_str[:16]  # YYYY-MM-DDTHH:MM
            event_datetime = datetime.strptime(dt_part, "%Y-%m-%dT%H:%M")
            event_date = event_datetime.date()
            event_time_str = event_datetime.strftime("%H:%M")

        # Only include today and upcoming events
        if event_date < today_local:
            continue

        # Deduplicate across merged calendars (keep separate dates for recurring instances)
        date_iso = event_date.isoformat()
        sig_key = ("sig", date_iso, event_time_str, summary.strip().lower())
        ical_uid = item.get("iCalUID")
        uid_key = ("uid", ical_uid, date_iso) if ical_uid else None

        if sig_key in seen_keys or (uid_key is not None and uid_key in seen_keys):
            continue
        seen_keys.add(sig_key)
        if uid_key is not None:
            seen_keys.add(uid_key)

        # Collect event days for current month grid (today & future in current month)
        if event_date.year == today_local.year and event_date.month == today_local.month:
            month_days_set.add(event_date.day)

        parsed_entries.append((
            event_date,
            0 if is_all_day else 1,
            event_time_str,
            summary.lower(),
            {
                "date": date_iso,
                "date_label": event_date.strftime("%a %d %b"),
                "time": event_time_str,
                "is_all_day": is_all_day,
                "is_today": (event_date == today_local),
                "title": summary,
            },
        ))

    parsed_entries.sort(key=lambda e: (e[0], e[1], e[2], e[3]))
    events_list = [entry[4] for entry in parsed_entries[:max_events]]

    return {
        "year": today_local.year,
        "month": today_local.month,
        "today_day": today_local.day,
        "month_days_with_events": sorted(list(month_days_set)),
        "events": events_list,
    }


async def get_google_calendar_and_tasks(profile_id: str) -> Dict[str, Any]:
    """
    Fetches events from all selected Google Calendars and tasks from Google Tasks for a profile.
    Merges and deduplicates upcoming calendar events (today onward, up to 15), computes active
    days for the current month grid, and lists active tasks across selected task lists.
    """
    now_local = datetime.now().astimezone()
    today_local = now_local.date()
    fallback_state = {
        "calendar": {
            "year": today_local.year,
            "month": today_local.month,
            "today_day": today_local.day,
            "month_days_with_events": [],
            "events": []
        },
        "todos": []
    }

    mgr = ProfileTokenManager(profile_id, "Google")
    tokens = mgr.load_tokens()
    if not tokens or "access_token" not in tokens:
        logger.debug(f"Google credentials are not configured or missing for profile {profile_id}.")
        return fallback_state

    access_token = tokens["access_token"]
    prof_settings = load_profile_settings(profile_id)

    async def make_request(url: str, params: Dict[str, Any], token: str) -> httpx.Response:
        headers = {"Authorization": f"Bearer {token}"}
        async with httpx.AsyncClient() as client:
            return await client.get(url, headers=headers, params=params, timeout=7.0)

    try:
        # --- 1. Fetch Calendar Events across selected calendars ---
        if "active_calendars" in prof_settings and isinstance(prof_settings["active_calendars"], list):
            active_calendars = prof_settings["active_calendars"]
        else:
            active_calendars = ["primary"]

        calendar_payload = fallback_state["calendar"]

        if active_calendars:
            # Start of today in local timezone through end of next month
            time_min = now_local.replace(hour=0, minute=0, second=0, microsecond=0).isoformat()

            if today_local.month == 12:
                next_month_year = today_local.year + 1
                next_month_val = 1
            else:
                next_month_year = today_local.year
                next_month_val = today_local.month + 1

            end_date = (datetime(next_month_year, next_month_val, 1) + timedelta(days=32)).replace(day=1) - timedelta(days=1)
            time_max = datetime(end_date.year, end_date.month, end_date.day, 23, 59, 59, tzinfo=now_local.tzinfo).isoformat()

            cal_params = {
                "timeMin": time_min,
                "timeMax": time_max,
                "singleEvents": "true",
                "orderBy": "startTime",
                "maxResults": 100
            }

            async def fetch_calendar_raw(cal_id: str, tok: str) -> tuple[str, Optional[httpx.Response]]:
                encoded_id = quote(str(cal_id), safe="")
                cal_url = f"https://www.googleapis.com/calendar/v3/calendars/{encoded_id}/events"
                try:
                    return cal_id, await make_request(cal_url, cal_params, tok)
                except Exception as exc:
                    logger.warning(f"Failed fetching calendar '{cal_id}' for profile {profile_id}: {exc}")
                    return cal_id, None

            raw_results = await asyncio.gather(*[fetch_calendar_raw(cid, access_token) for cid in active_calendars])

            # If any calendar returned 401, refresh token once and retry only those calendars
            expired_cids = [cid for cid, resp in raw_results if resp is not None and resp.status_code == 401]
            if expired_cids:
                logger.info(f"Google access token expired for profile {profile_id}. Attempting token refresh...")
                new_access_token = await refresh_google_token(profile_id)
                if not new_access_token:
                    logger.error(f"Failed to refresh Google token for profile {profile_id}.")
                    return fallback_state
                access_token = new_access_token
                retry_results = dict(await asyncio.gather(*[fetch_calendar_raw(cid, access_token) for cid in expired_cids]))
                raw_results = [
                    (cid, retry_results[cid] if cid in retry_results else resp)
                    for cid, resp in raw_results
                ]

            all_cal_items: List[Dict[str, Any]] = []
            for cid, resp in raw_results:
                if resp is None:
                    continue
                if resp.status_code == 200:
                    all_cal_items.extend(resp.json().get("items", []))
                else:
                    logger.warning(f"Google Calendar API for '{cid}' returned status {resp.status_code} (profile {profile_id})")

            calendar_payload = parse_calendar_items(all_cal_items, today_local, max_events=15)

        # --- 2. Fetch Tasks (from multiple selected lists) ---
        tasks_list = []

        # Determine which lists are active
        active_lists = prof_settings.get("active_task_lists", ["@default"])
        if not active_lists:
            active_lists = ["@default"]

        # Fetch all available lists to build an ID-to-Title mapping
        available_lists = await get_google_task_lists(profile_id)
        list_title_map = {l["id"]: l["title"] for l in available_lists}
        list_title_map["@default"] = "Tasks"

        async def fetch_tasks_from_list(list_id: str, list_title: str) -> List[Dict[str, Any]]:
            url = f"https://tasks.googleapis.com/tasks/v1/lists/{list_id}/tasks"
            params = {
                "showCompleted": "false",
                "maxResults": 10
            }
            try:
                res = await make_request(url, params, access_token)
                if res.status_code == 200:
                    data = res.json()
                    t_items = data.get("items", [])
                    results = []
                    for t in t_items:
                        title = t.get("title")
                        if title:
                            # Apply the list title prefix if configured for better visibility
                            display_title = f"[{list_title}] {title}" if list_title != "Tasks" else title
                            results.append({
                                "id": t.get("id"),
                                "title": display_title,
                                "completed": False
                            })
                    return results
                else:
                    logger.warning(f"Failed to fetch tasks from list {list_id} for profile {profile_id}: {res.status_code}")
                    return []
            except Exception as e:
                logger.error(f"Error fetching tasks from list {list_id} for profile {profile_id}: {e}")
                return []

        # Run all task fetches in parallel
        tasks_queries = [
            fetch_tasks_from_list(lid, list_title_map.get(lid, "Tasks"))
            for lid in active_lists
        ]
        
        results_lists = await asyncio.gather(*tasks_queries)
        for rlist in results_lists:
            tasks_list.extend(rlist)

        return {
            "calendar": calendar_payload,
            "todos": tasks_list[:8] # limit to 8 total aggregated tasks
        }

    except httpx.RequestError as e:
        logger.error(f"Network error while calling Google APIs for profile {profile_id}: {e}")
        return fallback_state
    except Exception as e:
        logger.error(f"Unexpected error in Google APIs aggregator for profile {profile_id}: {e}")
        return fallback_state
