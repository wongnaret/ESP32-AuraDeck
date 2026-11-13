"""
Unit tests for Antigravity backend service (normalization, atomic storage, and backward compatibility).
"""

import unittest
import os
import sys
import json
import asyncio
import tempfile

# Add backend and repository root to sys.path
REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BACKEND_DIR = os.path.join(REPO_ROOT, "backend")
if BACKEND_DIR not in sys.path:
    sys.path.insert(0, BACKEND_DIR)
if REPO_ROOT not in sys.path:
    sys.path.insert(0, REPO_ROOT)

from app.services.antigravity import (
    normalize_antigravity_payload,
    load_antigravity_state,
    save_antigravity_state,
    ingest_antigravity_payload,
    get_antigravity_credits,
    format_countdown,
    parse_fraction_or_percent,
    DEFAULT_ANTIGRAVITY_STATE
)


class TestAntigravityService(unittest.TestCase):

    def test_format_countdown(self):
        self.assertEqual(format_countdown(0), "Full")
        self.assertEqual(format_countdown(-1), "Full")
        self.assertEqual(format_countdown(30), "30s")
        self.assertEqual(format_countdown(120), "2m")
        self.assertEqual(format_countdown(11160), "03h 06m")
        self.assertEqual(format_countdown(90000), "1 day, 1 hour")
        self.assertEqual(format_countdown(187200), "2 days, 4 hours")
        self.assertEqual(format_countdown(86400 * 2), "2 days")

    def test_parse_fraction_or_percent(self):
        self.assertEqual(parse_fraction_or_percent(0.69), 69.0)
        self.assertEqual(parse_fraction_or_percent(1.0), 100.0)
        self.assertEqual(parse_fraction_or_percent(0.0), 0.0)
        self.assertEqual(parse_fraction_or_percent(62.0), 62.0)
        self.assertEqual(parse_fraction_or_percent("0.55"), 55.0)
        self.assertEqual(parse_fraction_or_percent(-5), 0.0)
        self.assertEqual(parse_fraction_or_percent(150), 100.0)

    def test_normalize_nominal_payload(self):
        fixture_path = os.path.join(REPO_ROOT, "tests", "fixtures", "nominal_quota.json")
        with open(fixture_path, "r", encoding="utf-8") as f:
            raw = json.load(f)

        norm = normalize_antigravity_payload(raw)

        # Basic metadata
        self.assertEqual(norm["status"], "ok")
        self.assertEqual(norm["plan"], "Google AI Pro")
        self.assertEqual(norm["available_credits"], 560)
        self.assertEqual(norm["ai_credits"], 560)

        # Gemini models
        gemini = norm["gemini_models"]
        self.assertEqual(gemini["five_hour_limit_percent"], 69.0)
        self.assertEqual(gemini["weekly_limit_percent"], 62.0)
        self.assertEqual(gemini["next_reset_5h"], "03h 06m")
        self.assertEqual(gemini["next_reset_weekly"], "1 day, 1 hour")

        # 3P models
        three_p = norm["claude_gpt_models"]
        self.assertEqual(three_p["five_hour_limit_percent"], 100.0)
        self.assertEqual(three_p["weekly_limit_percent"], 100.0)

        # Dynamic model (Argon)
        self.assertEqual(len(norm["dynamic_models"]), 1)
        self.assertEqual(norm["dynamic_models"][0]["name"], "Gemini 4 Argon")
        self.assertEqual(norm["dynamic_models"][0]["remaining_percent"], 2.0)
        self.assertEqual(norm["dynamic_models"][0]["next_reset"], "2 days, 4 hours")
        self.assertIn("gemini_4_argon", norm["additional_models"])

        # Legacy backward compatibility
        self.assertEqual(norm["credit_hours_remaining"], 560.0)
        self.assertEqual(norm["percent_quota_used"], 31.0)
        self.assertEqual(norm["limit_5h"]["used"], 31.0)
        self.assertEqual(norm["limit_weekly"]["used"], 38.0)
        self.assertEqual(norm["next_reset"], "03h 06m")

    def test_normalize_boundary_payload(self):
        raw = {
            "plan": "Google AI Ultra",
            "credits": 0,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.0, "reset_in_seconds": 120},
                "gemini-weekly": {"remaining_fraction": 1.0, "reset_in_seconds": 0},
                "3p-5h": {"remaining_fraction": 0.0, "reset_in_seconds": 3600},
                "3p-weekly": {"remaining_fraction": 0.01, "reset_in_seconds": 604800}
            }
        }
        norm = normalize_antigravity_payload(raw)
        self.assertEqual(norm["available_credits"], 0)
        self.assertEqual(norm["gemini_models"]["five_hour_limit_percent"], 0.0)
        self.assertEqual(norm["gemini_models"]["weekly_limit_percent"], 100.0)
        self.assertEqual(norm["claude_gpt_models"]["five_hour_limit_percent"], 0.0)
        self.assertEqual(norm["claude_gpt_models"]["weekly_limit_percent"], 1.0)
        self.assertEqual(norm["percent_quota_used"], 100.0)

    def test_normalize_partial_and_missing_payload(self):
        raw = {
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.55}
            }
        }
        norm = normalize_antigravity_payload(raw)
        self.assertEqual(norm["plan"], "Google AI Pro")
        self.assertEqual(norm["gemini_models"]["five_hour_limit_percent"], 55.0)
        # Missing models fall back gracefully to sensible defaults
        self.assertEqual(norm["gemini_models"]["weekly_limit_percent"], 99.0)
        self.assertEqual(norm["claude_gpt_models"]["five_hour_limit_percent"], 100.0)

    def test_normalize_unauthenticated_payload(self):
        raw = {
            "status": "unauthenticated",
            "error_message": "Antigravity CLI unauthenticated."
        }
        norm = normalize_antigravity_payload(raw)
        self.assertEqual(norm["status"], "unauthenticated")
        self.assertEqual(norm["error_message"], "Antigravity CLI unauthenticated.")

    def test_atomic_save_and_load(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            test_file = os.path.join(tmpdir, "antigravity_data.json")
            import app.services.antigravity as agy_svc
            orig_state_file = agy_svc.STATE_FILE
            try:
                agy_svc.STATE_FILE = test_file
                test_state = {
                    "status": "ok",
                    "plan": "Unit Test Plan",
                    "available_credits": 999,
                    "gemini_models": {
                        "five_hour_limit_percent": 88.0,
                        "weekly_limit_percent": 77.0,
                        "next_reset_5h": "01h 00m",
                        "next_reset_weekly": "2 days"
                    }
                }
                save_antigravity_state(test_state)
                self.assertTrue(os.path.exists(test_file))

                loaded = load_antigravity_state()
                self.assertEqual(loaded["plan"], "Unit Test Plan")
                self.assertEqual(loaded["available_credits"], 999)
                self.assertEqual(loaded["gemini_models"]["five_hour_limit_percent"], 88.0)
            finally:
                agy_svc.STATE_FILE = orig_state_file

    def test_ingest_antigravity_payload(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            test_file = os.path.join(tmpdir, "antigravity_data.json")
            import app.services.antigravity as agy_svc
            orig_state_file = agy_svc.STATE_FILE
            try:
                agy_svc.STATE_FILE = test_file
                raw_input = {
                    "plan": "Ingest Test",
                    "credits": 450,
                    "quota": {
                        "gemini-5h": {"remaining_fraction": 0.80, "reset_in_seconds": 3600},
                        "gemini-weekly": {"remaining_fraction": 0.70, "reset_in_seconds": 72000},
                        "3p-5h": {"remaining_fraction": 0.90},
                        "3p-weekly": {"remaining_fraction": 0.95},
                        "gemini-custom": {"remaining_fraction": 0.15, "reset_in_seconds": 1800}
                    }
                }
                ingested = ingest_antigravity_payload(raw_input)
                self.assertEqual(ingested["plan"], "Ingest Test")
                self.assertEqual(ingested["available_credits"], 450)
                self.assertEqual(ingested["gemini_models"]["five_hour_limit_percent"], 80.0)
                self.assertIn("gemini_custom", ingested["additional_models"])

                # Confirm persisted on disk
                with open(test_file, "r", encoding="utf-8") as f:
                    disk_data = json.load(f)
                self.assertEqual(disk_data["plan"], "Ingest Test")
                self.assertEqual(disk_data["available_credits"], 450)
            finally:
                agy_svc.STATE_FILE = orig_state_file

    def test_get_antigravity_credits_async(self):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        try:
            credits_data = loop.run_until_complete(get_antigravity_credits())
            self.assertIn("plan", credits_data)
            self.assertIn("gemini_models", credits_data)
            self.assertIn("claude_gpt_models", credits_data)
            self.assertIn("credit_hours_remaining", credits_data)
        finally:
            loop.close()


if __name__ == "__main__":
    unittest.main()
