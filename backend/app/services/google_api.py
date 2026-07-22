import logging
import asyncio
import httpx
from datetime import datetime, timedelta, date, timezone
from typing import Dict, Any, List

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


async def get_google_calendar_and_tasks(profile_id: str) -> Dict[str, Any]:
    """
    Fetches events from Google Calendar and tasks from Google Tasks for a profile.
    Aggregates calendar events for today & tomorrow, calculates month-level active days,
    and lists active tasks across all selected Google Task lists, utilizing auto-refresh.
    """
    fallback_state = {
        "calendar": {
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

    async def make_request(url: str, params: Dict[str, Any], token: str) -> httpx.Response:
        headers = {"Authorization": f"Bearer {token}"}
        async with httpx.AsyncClient() as client:
            return await client.get(url, headers=headers, params=params, timeout=7.0)

    try:
        # --- 1. Fetch Calendar Events ---
        now = datetime.now(timezone.utc)
        today_local = datetime.now().date()
        tomorrow_local = today_local + timedelta(days=1)
        
        # Calculate time range: from start of today to the end of next month
        time_min = datetime(today_local.year, today_local.month, today_local.day, 0, 0, 0, tzinfo=timezone.utc).isoformat()
        
        # End of next month
        if today_local.month == 12:
            next_month_year = today_local.year + 1
            next_month_val = 1
        else:
            next_month_year = today_local.year
            next_month_val = today_local.month + 1
        
        # Get last day of next month
        end_date = (datetime(next_month_year, next_month_val, 1) + timedelta(days=32)).replace(day=1) - timedelta(days=1)
        time_max = datetime(end_date.year, end_date.month, end_date.day, 23, 59, 59, tzinfo=timezone.utc).isoformat()

        cal_url = "https://www.googleapis.com/calendar/v3/calendars/primary/events"
        cal_params = {
            "timeMin": time_min,
            "timeMax": time_max,
            "singleEvents": "true",
            "orderBy": "startTime",
            "maxResults": 100
        }

        cal_response = await make_request(cal_url, cal_params, access_token)

        # Handle expiration
        if cal_response.status_code == 401:
            logger.info(f"Google access token expired for profile {profile_id}. Attempting token refresh...")
            new_access_token = await refresh_google_token(profile_id)
            if new_access_token:
                access_token = new_access_token
                cal_response = await make_request(cal_url, cal_params, access_token)
            else:
                logger.error(f"Failed to refresh Google token for profile {profile_id}.")
                return fallback_state

        month_days_set = set()
        events_list = []

        if cal_response.status_code == 200:
            cal_data = cal_response.json()
            items = cal_data.get("items", [])

            for item in items:
                summary = item.get("summary", "No Title")
                start = item.get("start", {})
                
                start_dt_str = start.get("dateTime") or start.get("date")
                if not start_dt_str:
                    continue

                is_all_day = "dateTime" not in start

                if is_all_day:
                    event_date = datetime.strptime(start_dt_str[:10], "%Y-%m-%d").date()
                    event_time_str = "All Day"
                else:
                    dt_part = start_dt_str[:16] # YYYY-MM-DDTHH:MM
                    event_datetime = datetime.strptime(dt_part, "%Y-%m-%dT%H:%M")
                    event_date = event_datetime.date()
                    event_time_str = event_datetime.strftime("%H:%M")

                # Collect event days for current month grid
                if event_date.year == today_local.year and event_date.month == today_local.month:
                    month_days_set.add(event_date.day)

                # Extract events for today and tomorrow
                if event_date == today_local:
                    events_list.append({
                        "time": event_time_str,
                        "title": summary,
                        "is_today": True
                    })
                elif event_date == tomorrow_local:
                    events_list.append({
                        "time": f"Tomorrow {event_time_str}" if not is_all_day else "Tomorrow (All Day)",
                        "title": summary,
                        "is_today": False
                    })
        else:
            logger.warning(f"Google Calendar API returned status code: {cal_response.status_code} for profile {profile_id}")

        # --- 2. Fetch Tasks (from multiple selected lists) ---
        tasks_list = []
        
        # Determine which lists are active
        prof_settings = load_profile_settings(profile_id)
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
            "calendar": {
                "month_days_with_events": sorted(list(month_days_set)),
                "events": events_list[:8]
            },
            "todos": tasks_list[:8] # limit to 8 total aggregated tasks
        }

    except httpx.RequestError as e:
        logger.error(f"Network error while calling Google APIs for profile {profile_id}: {e}")
        return fallback_state
    except Exception as e:
        logger.error(f"Unexpected error in Google APIs aggregator for profile {profile_id}: {e}")
        return fallback_state
