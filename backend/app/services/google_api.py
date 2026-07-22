import logging
import httpx
from datetime import datetime, timedelta, date, timezone
from typing import Dict, Any, List

from app.services.google_auth import google_tokens, refresh_google_token

logger = logging.getLogger("google_api_service")

async def get_google_calendar_and_tasks() -> Dict[str, Any]:
    """
    Fetches events from Google Calendar and tasks from Google Tasks.
    Aggregates calendar events for today & tomorrow, calculates month-level active days,
    and lists active tasks, utilizing access token auto-refresh and returning robust fallbacks.
    """
    fallback_state = {
        "calendar": {
            "month_days_with_events": [],
            "events": []
        },
        "todos": []
    }

    tokens = google_tokens.load_tokens()
    if not tokens or "access_token" not in tokens:
        logger.debug("Google credentials are not configured or missing.")
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
        
        # Get last day of next month (approximate safely as day 28 of month + 2 to get to month + 3, or simply add 60 days)
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
            logger.info("Google access token expired. Attempting token refresh...")
            new_access_token = await refresh_google_token()
            if new_access_token:
                access_token = new_access_token
                cal_response = await make_request(cal_url, cal_params, access_token)
            else:
                logger.error("Failed to refresh Google token.")
                return fallback_state

        month_days_set = set()
        events_list = []

        if cal_response.status_code == 200:
            cal_data = cal_response.json()
            items = cal_data.get("items", [])

            for item in items:
                summary = item.get("summary", "No Title")
                start = item.get("start", {})
                
                # Parsing start date
                start_dt_str = start.get("dateTime") or start.get("date")
                if not start_dt_str:
                    continue

                is_all_day = "dateTime" not in start

                if is_all_day:
                    # format is YYYY-MM-DD
                    event_date = datetime.strptime(start_dt_str[:10], "%Y-%m-%d").date()
                    event_time_str = "All Day"
                else:
                    # format is ISO datetime with timezone
                    # e.g., 2026-07-22T14:30:00+07:00 or 2026-07-22T07:30:00Z
                    # Use substring to safely parse without full tz offsets if complicated
                    dt_part = start_dt_str[:16] # YYYY-MM-DDTHH:MM
                    event_datetime = datetime.strptime(dt_part, "%Y-%m-%dT%H:%M")
                    event_date = event_datetime.date()
                    event_time_str = event_datetime.strftime("%H:%M")

                # 1. Collect event days for current month grid
                if event_date.year == today_local.year and event_date.month == today_local.month:
                    month_days_set.add(event_date.day)

                # 2. Extract events for today and tomorrow
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
            logger.warning(f"Google Calendar API returned unexpected status code: {cal_response.status_code}")

        # --- 2. Fetch Tasks ---
        tasks_list = []
        tasks_url = "https://tasks.googleapis.com/tasks/v1/lists/@default/tasks"
        tasks_params = {
            "showCompleted": "false",
            "maxResults": 10
        }

        tasks_response = await make_request(tasks_url, tasks_params, access_token)
        if tasks_response.status_code == 200:
            tasks_data = tasks_response.json()
            t_items = tasks_data.get("items", [])
            for t in t_items:
                title = t.get("title")
                if title:
                    tasks_list.append({
                        "id": t.get("id"),
                        "title": title,
                        "completed": False
                    })
        else:
            logger.warning(f"Google Tasks API returned unexpected status code: {tasks_response.status_code}")

        return {
            "calendar": {
                "month_days_with_events": sorted(list(month_days_set)),
                "events": events_list[:8] # limit to 8 events for layout safety
            },
            "todos": tasks_list[:8] # limit to 8 tasks
        }

    except httpx.RequestError as e:
        logger.error(f"Network error while calling Google APIs: {e}")
        return fallback_state
    except Exception as e:
        logger.error(f"Unexpected error in Google APIs aggregator: {e}")
        return fallback_state
