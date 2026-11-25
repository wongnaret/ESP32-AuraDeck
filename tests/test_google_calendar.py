"""
Unit tests for Google Calendar upcoming agendas & monthly grid marker parsing.
"""

import os
import sys
import unittest
import asyncio
from datetime import date, datetime
from unittest.mock import patch, MagicMock

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BACKEND_DIR = os.path.join(REPO_ROOT, "backend")
if BACKEND_DIR not in sys.path:
    sys.path.insert(0, BACKEND_DIR)
if REPO_ROOT not in sys.path:
    sys.path.insert(0, REPO_ROOT)

from app.services.google_api import parse_calendar_items, get_google_calendar_and_tasks


class TestGoogleCalendarParsing(unittest.TestCase):
    def test_today_and_future_only_with_month_dots(self):
        today = date(2026, 11, 24)
        items = [
            # Past event in same month -> excluded from both dots and events
            {"summary": "Past Event", "start": {"date": "2026-11-10"}, "end": {"date": "2026-11-11"}},
            # Multi-day event spanning across today -> clamped to today
            {"summary": "Ongoing Trip", "start": {"date": "2026-11-22"}, "end": {"date": "2026-11-26"}},
            # Timed event today
            {"summary": "Team Sync", "start": {"dateTime": "2026-11-24T09:30:00+07:00"}},
            # Future event in same month
            {"summary": "Demo Day", "start": {"dateTime": "2026-11-27T15:00:00+07:00"}},
            # Next month event -> in events list, NOT in current month dots
            {"summary": "Dec Kickoff", "start": {"date": "2026-12-02"}, "end": {"date": "2026-12-03"}},
        ]

        payload = parse_calendar_items(items, today, max_events=15)

        self.assertEqual(payload["year"], 2026)
        self.assertEqual(payload["month"], 11)
        self.assertEqual(payload["today_day"], 24)
        self.assertEqual(payload["month_days_with_events"], [24, 27])
        self.assertEqual(len(payload["events"]), 4)

        self.assertEqual(payload["events"][0]["title"], "Ongoing Trip")
        self.assertTrue(payload["events"][0]["is_all_day"])
        self.assertTrue(payload["events"][0]["is_today"])
        self.assertEqual(payload["events"][0]["date_label"], "Tue 24 Nov")

        self.assertEqual(payload["events"][1]["title"], "Team Sync")
        self.assertFalse(payload["events"][1]["is_all_day"])
        self.assertEqual(payload["events"][1]["time"], "09:30")

        self.assertEqual(payload["events"][3]["date"], "2026-12-02")
        self.assertEqual(payload["events"][3]["date_label"], "Wed 02 Dec")

    def test_max_events_limit_15_while_preserving_all_month_dots(self):
        today = date(2026, 11, 1)
        items = [
            {"summary": f"Event {d}", "start": {"date": f"2026-11-{d:02d}"}, "end": {"date": f"2026-11-{d+1:02d}"}}
            for d in range(1, 21)
        ]

        payload = parse_calendar_items(items, today, max_events=15)
        self.assertEqual(len(payload["events"]), 15)
        self.assertEqual(len(payload["month_days_with_events"]), 20)

    def test_merge_dedup_and_chronological_sort_all_day_first(self):
        today = date(2026, 11, 24)
        # Events arriving out of order across 2 calendars, with duplicates + recurring instances
        items = [
            {"iCalUID": "ev-late", "summary": "Late Meeting", "start": {"dateTime": "2026-11-24T16:00:00+07:00"}},
            {"iCalUID": "ev-shared", "summary": "Company Townhall", "start": {"dateTime": "2026-11-24T09:00:00+07:00"}},
            # Duplicate via iCalUID on same date
            {"iCalUID": "ev-shared", "summary": "Company Townhall (Work Copy)", "start": {"dateTime": "2026-11-24T09:00:00+07:00"}},
            # Duplicate via (date, time, title) without iCalUID
            {"summary": "Loy Krathong", "start": {"date": "2026-11-24"}, "end": {"date": "2026-11-25"}},
            {"summary": "loy krathong", "start": {"date": "2026-11-24"}, "end": {"date": "2026-11-25"}},
            # Recurring instance with same iCalUID on a DIFFERENT date must NOT be dropped
            {"iCalUID": "ev-recur", "summary": "Daily Standup", "start": {"dateTime": "2026-11-24T10:00:00+07:00"}},
            {"iCalUID": "ev-recur", "summary": "Daily Standup", "start": {"dateTime": "2026-11-25T10:00:00+07:00"}},
        ]

        payload = parse_calendar_items(items, today, max_events=15)
        titles_and_times = [(e["date"], e["time"], e["title"]) for e in payload["events"]]

        # All-day comes before 09:00 on 2026-11-24; duplicates dropped; both recurring dates kept
        self.assertEqual(titles_and_times, [
            ("2026-11-24", "All Day", "Loy Krathong"),
            ("2026-11-24", "09:00", "Company Townhall"),
            ("2026-11-24", "10:00", "Daily Standup"),
            ("2026-11-24", "16:00", "Late Meeting"),
            ("2026-11-25", "10:00", "Daily Standup"),
        ])

    @patch("app.services.google_api.get_google_task_lists", return_value=[])
    @patch("app.services.google_api.load_profile_settings")
    @patch("app.services.google_api.ProfileTokenManager")
    def test_empty_active_calendars_and_url_encoding(self, mock_tok_cls, mock_settings, _mock_task_lists):
        mock_tok = MagicMock()
        mock_tok.load_tokens.return_value = {"access_token": "tok-123"}
        mock_tok_cls.return_value = mock_tok

        # 1. Explicit empty list -> returns empty calendar without calling Calendar API
        mock_settings.return_value = {"active_calendars": [], "active_task_lists": ["@default"]}

        called_urls = []

        class FakeResp:
            def __init__(self, status_code, data):
                self.status_code = status_code
                self._data = data

            def json(self):
                return self._data

        class FakeClient:
            async def __aenter__(self):
                return self

            async def __aexit__(self, *args):
                return False

            async def get(self, url, headers=None, params=None, timeout=None):
                called_urls.append(url)
                if "tasks.googleapis.com" in url:
                    return FakeResp(200, {"items": []})
                today_iso = datetime.now().date().isoformat()
                return FakeResp(200, {"items": [
                    {"summary": "Holiday", "start": {"date": today_iso}}
                ]})

        with patch("app.services.google_api.httpx.AsyncClient", return_value=FakeClient()):
            res_empty = asyncio.run(get_google_calendar_and_tasks("p1"))
            self.assertEqual(res_empty["calendar"]["events"], [])
            self.assertEqual(res_empty["calendar"]["month_days_with_events"], [])
            self.assertFalse(any("calendar/v3/calendars" in u for u in called_urls))

            # 2. Calendar ID with `#` gets properly percent-encoded (`%23`)
            called_urls.clear()
            mock_settings.return_value = {
                "active_calendars": ["primary", "th.th#holiday@group.v.calendar.google.com"],
                "active_task_lists": ["@default"]
            }
            res_multi = asyncio.run(get_google_calendar_and_tasks("p1"))
            cal_urls = [u for u in called_urls if "calendar/v3/calendars" in u]
            self.assertEqual(len(cal_urls), 2)
            self.assertIn("th.th%23holiday%40group.v.calendar.google.com", cal_urls[1])
            # Identical event on both calendars deduplicated to 1
            self.assertEqual(len(res_multi["calendar"]["events"]), 1)


if __name__ == "__main__":
    unittest.main()
