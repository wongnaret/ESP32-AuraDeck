"""
Unit tests for Google Calendar upcoming agendas & monthly grid marker parsing.
"""

import os
import sys
import unittest
from datetime import date

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BACKEND_DIR = os.path.join(REPO_ROOT, "backend")
if BACKEND_DIR not in sys.path:
    sys.path.insert(0, BACKEND_DIR)
if REPO_ROOT not in sys.path:
    sys.path.insert(0, REPO_ROOT)

from app.services.google_api import parse_calendar_items


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


if __name__ == "__main__":
    unittest.main()
