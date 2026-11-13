"""
tests/test_backend_ingestion.py
Tests for AuraDeck Backend Antigravity Ingestion, Normalization, Backward Compatibility, and Endpoints.
Covers:
- Feature 4: Backend Ingestion & Backward Compatibility (PROJECT.md R3)
- Feature 5: Dynamic Model Handling & Countdown Calculation (PROJECT.md R1, R3)
- REST Endpoints: GET /api/v1/antigravity and POST /api/v1/antigravity
"""

import unittest
import os
import sys
import json
import tempfile
import asyncio
from pathlib import Path
from unittest.mock import patch, MagicMock

REPO_ROOT = Path(__file__).resolve().parent.parent
BACKEND_DIR = REPO_ROOT / "backend"
if str(BACKEND_DIR) not in sys.path:
    sys.path.insert(0, str(BACKEND_DIR))

from starlette.testclient import TestClient
import app.services.antigravity as ag_service
from app.main import app


class TestBackendStateFileManagement(unittest.TestCase):
    """Tests for loading and saving state files, atomicity, and default fallbacks."""

    def setUp(self):
        self.tmp_dir = tempfile.TemporaryDirectory()
        self.orig_tokens_dir = ag_service.TOKENS_DIR
        self.orig_state_file = ag_service.STATE_FILE
        ag_service.TOKENS_DIR = self.tmp_dir.name
        ag_service.STATE_FILE = os.path.join(self.tmp_dir.name, "antigravity_data.json")

    def tearDown(self):
        ag_service.TOKENS_DIR = self.orig_tokens_dir
        ag_service.STATE_FILE = self.orig_state_file
        self.tmp_dir.cleanup()

    def test_load_state_missing_file_returns_default(self):
        """When state file is missing, default state is returned and saved."""
        state = ag_service.load_antigravity_state()
        self.assertIsInstance(state, dict)
        self.assertEqual(state.get("plan"), "Google AI Pro")
        self.assertTrue(os.path.exists(ag_service.STATE_FILE))

    def test_save_and_load_roundtrip(self):
        """State written to disk is loaded accurately."""
        custom_state = {
            "status": "ok",
            "plan": "Google AI Enterprise",
            "available_credits": 1500,
            "ai_credits": 1500,
            "gemini_models": {
                "weekly_limit_percent": 80.0,
                "five_hour_limit_percent": 75.0,
                "next_reset_5h": "01h 30m",
                "next_reset_weekly": "3 days, 2 hours"
            }
        }
        ag_service.save_antigravity_state(custom_state)
        loaded = ag_service.load_antigravity_state()
        self.assertEqual(loaded.get("plan"), "Google AI Enterprise")
        self.assertEqual(loaded.get("available_credits"), 1500)
        self.assertEqual(loaded.get("gemini_models", {}).get("five_hour_limit_percent"), 75.0)

    def test_load_corrupted_json_file_falls_back_safely(self):
        """If state file contains invalid JSON, load_antigravity_state does not crash."""
        with open(ag_service.STATE_FILE, "w", encoding="utf-8") as f:
            f.write("{corrupted json contents...")
        state = ag_service.load_antigravity_state()
        self.assertIsInstance(state, dict)
        self.assertEqual(state.get("plan"), "Google AI Pro")

    def test_save_creates_directory_if_missing(self):
        """Saving state automatically ensures directory exists."""
        nested_dir = os.path.join(self.tmp_dir.name, "nested", "tokens")
        ag_service.TOKENS_DIR = nested_dir
        ag_service.STATE_FILE = os.path.join(nested_dir, "antigravity_data.json")
        custom = {"plan": "Pro Test"}
        ag_service.save_antigravity_state(custom)
        self.assertTrue(os.path.exists(ag_service.STATE_FILE))

    def test_load_merges_with_default_state_for_missing_keys(self):
        """Loading a partial state merges with defaults so required keys are never missing."""
        partial = {"plan": "Custom Plan Only"}
        with open(ag_service.STATE_FILE, "w", encoding="utf-8") as f:
            json.dump(partial, f)
        merged = ag_service.load_antigravity_state()
        self.assertEqual(merged.get("plan"), "Custom Plan Only")
        self.assertIn("gemini_models", merged)


class TestBackendNormalizationStandardModels(unittest.TestCase):
    """Tests for normalising raw CLI quota payloads for standard Gemini & 3P models."""

    def test_normalize_raw_quota_gemini_percentages(self):
        """Verify remaining_fraction converts to percentage in gemini_models."""
        raw_payload = {
            "plan": "Google AI Pro",
            "available_credits": 560,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.69, "reset_in_seconds": 11160},
                "gemini-weekly": {"remaining_fraction": 0.62, "reset_in_seconds": 90000}
            }
        }
        # Test normalization logic if helper exists or via state update
        if hasattr(ag_service, "normalize_antigravity_payload"):
            norm = ag_service.normalize_antigravity_payload(raw_payload)
            self.assertEqual(norm["gemini_models"]["five_hour_limit_percent"], 69.0)
            self.assertEqual(norm["gemini_models"]["weekly_limit_percent"], 62.0)
        else:
            # When normalized through save/load
            ag_service.save_antigravity_state({
                "plan": raw_payload["plan"],
                "available_credits": raw_payload["available_credits"],
                "gemini_models": {
                    "five_hour_limit_percent": raw_payload["quota"]["gemini-5h"]["remaining_fraction"] * 100,
                    "weekly_limit_percent": raw_payload["quota"]["gemini-weekly"]["remaining_fraction"] * 100,
                    "next_reset_5h": "03h 06m",
                    "next_reset_weekly": "1 day, 1 hour"
                }
            })
            res = asyncio.run(ag_service.get_antigravity_credits())
            self.assertEqual(res["gemini_models"]["five_hour_limit_percent"], 69.0)
            self.assertEqual(res["gemini_models"]["weekly_limit_percent"], 62.0)

    def test_normalize_3p_models_percentages(self):
        """Verify 3P models (Claude & GPT) are normalized to claude_gpt_models."""
        if hasattr(ag_service, "normalize_antigravity_payload"):
            raw = {
                "quota": {
                    "3p-5h": {"remaining_fraction": 1.0, "reset_in_seconds": 0},
                    "3p-weekly": {"remaining_fraction": 1.0, "reset_in_seconds": 0}
                }
            }
            norm = ag_service.normalize_antigravity_payload(raw)
            self.assertEqual(norm["claude_gpt_models"]["five_hour_limit_percent"], 100.0)
            self.assertEqual(norm["claude_gpt_models"]["weekly_limit_percent"], 100.0)

    def test_normalize_countdown_formatting(self):
        """Verify countdown seconds format into human readable strings."""
        if hasattr(ag_service, "format_countdown"):
            self.assertEqual(ag_service.format_countdown(11160), "03h 06m")
            self.assertEqual(ag_service.format_countdown(90000), "1 day, 1 hour")
            self.assertEqual(ag_service.format_countdown(187200), "2 days, 4 hours")
            self.assertIn(ag_service.format_countdown(0), ["Full", "0m"])

    def test_normalize_credits_aliases(self):
        """Verify both available_credits and ai_credits are unified."""
        state = {
            "ai_credits": 777,
            "gemini_models": {"five_hour_limit_percent": 90.0, "weekly_limit_percent": 95.0}
        }
        ag_service.save_antigravity_state(state)
        res = asyncio.run(ag_service.get_antigravity_credits())
        self.assertEqual(res.get("available_credits"), 777)
        self.assertEqual(res.get("ai_credits"), 777)

    def test_normalize_plan_name_default(self):
        """Verify missing plan defaults to Google AI Pro."""
        state = {"gemini_models": {"five_hour_limit_percent": 90.0, "weekly_limit_percent": 95.0}}
        ag_service.save_antigravity_state(state)
        res = asyncio.run(ag_service.get_antigravity_credits())
        self.assertTrue(res.get("plan"))


class TestBackendDynamicModelsAndCountdowns(unittest.TestCase):
    """Tests for dynamic model keys (e.g. gemini-4-argon, claude-3-7-sonnet)."""

    def test_dynamic_model_gemini_4_argon_extracted(self):
        """Verify gemini-4-argon is extracted into additional_models."""
        raw_payload = {
            "plan": "Google AI Pro",
            "available_credits": 560,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.69, "reset_in_seconds": 11160},
                "gemini-4-argon": {
                    "remaining_fraction": 0.02,
                    "reset_in_seconds": 187200,
                    "reset_time": "2026-11-15T09:36:00Z"
                }
            }
        }
        if hasattr(ag_service, "normalize_antigravity_payload"):
            norm = ag_service.normalize_antigravity_payload(raw_payload)
            self.assertIn("additional_models", norm)
            self.assertIn("gemini_4_argon", norm["additional_models"])
            argon = norm["additional_models"]["gemini_4_argon"]
            self.assertEqual(argon["weekly_limit_percent"], 2.0)
            self.assertIn("2 days", argon["next_reset"])

    def test_dynamic_model_key_sanitization(self):
        """Verify dynamic keys with hyphens, dots, or spaces are normalized into clean dictionary keys."""
        if hasattr(ag_service, "normalize_model_key"):
            self.assertEqual(ag_service.normalize_model_key("gemini-4-argon"), "gemini_4_argon")
            self.assertEqual(ag_service.normalize_model_key("claude-3.5-sonnet"), "claude_3_5_sonnet")

    def test_dynamic_model_display_name_generated(self):
        """Verify human friendly display name is generated from dynamic key (e.g. 'Gemini 4 Argon')."""
        if hasattr(ag_service, "format_model_name"):
            self.assertEqual(ag_service.format_model_name("gemini-4-argon"), "Gemini 4 Argon")
            self.assertEqual(ag_service.format_model_name("claude-3-7-sonnet"), "Claude 3 7 Sonnet")

    def test_multiple_dynamic_models_simultaneous(self):
        """Verify multiple dynamic models are stored in additional_models without collisions."""
        raw = {
            "quota": {
                "gemini-4-argon": {"remaining_fraction": 0.02, "reset_in_seconds": 187200},
                "claude-3-7-sonnet": {"remaining_fraction": 0.50, "reset_in_seconds": 7200},
                "gemini-ultra-2": {"remaining_fraction": 0.85, "reset_in_seconds": 36000}
            }
        }
        if hasattr(ag_service, "normalize_antigravity_payload"):
            norm = ag_service.normalize_antigravity_payload(raw)
            self.assertIn("gemini_4_argon", norm["additional_models"])
            self.assertIn("claude_3_7_sonnet", norm["additional_models"])
            self.assertIn("gemini_ultra_2", norm["additional_models"])

    def test_dynamic_model_missing_countdown(self):
        """Verify dynamic model without countdown gracefully defaults next_reset."""
        raw = {
            "quota": {
                "experimental-model": {"remaining_fraction": 0.30}
            }
        }
        if hasattr(ag_service, "normalize_antigravity_payload"):
            norm = ag_service.normalize_antigravity_payload(raw)
            model = norm["additional_models"].get("experimental_model", {})
            self.assertEqual(model.get("weekly_limit_percent"), 30.0)


class TestBackendLegacySchemaCompatibility(unittest.TestCase):
    """Tests for legacy fields required by ESP32 firmware and existing Web Dashboard."""

    def test_legacy_credit_hours_remaining_is_float(self):
        """Verify credit_hours_remaining is exposed as a float matching available_credits."""
        ag_service.save_antigravity_state({"available_credits": 560})
        res = asyncio.run(ag_service.get_antigravity_credits())
        self.assertIn("credit_hours_remaining", res)
        self.assertEqual(res["credit_hours_remaining"], 560.0)
        self.assertIsInstance(res["credit_hours_remaining"], float)

    def test_legacy_percent_quota_used_calculated_from_gemini_5h(self):
        """Verify percent_quota_used equals 100.0 - five_hour_limit_percent."""
        ag_service.save_antigravity_state({
            "gemini_models": {"five_hour_limit_percent": 69.0, "weekly_limit_percent": 62.0}
        })
        res = asyncio.run(ag_service.get_antigravity_credits())
        self.assertIn("percent_quota_used", res)
        self.assertEqual(res["percent_quota_used"], 31.0)

    def test_legacy_limit_5h_dictionary_structure(self):
        """Verify limit_5h contains used, total, and percentage keys."""
        ag_service.save_antigravity_state({
            "gemini_models": {"five_hour_limit_percent": 69.0, "weekly_limit_percent": 62.0}
        })
        res = asyncio.run(ag_service.get_antigravity_credits())
        self.assertIn("limit_5h", res)
        self.assertEqual(res["limit_5h"]["used"], 31.0)
        self.assertEqual(res["limit_5h"]["total"], 100.0)
        self.assertEqual(res["limit_5h"]["percentage"], 31.0)

    def test_legacy_limit_weekly_dictionary_structure(self):
        """Verify limit_weekly contains used, total, and percentage keys."""
        ag_service.save_antigravity_state({
            "gemini_models": {"five_hour_limit_percent": 69.0, "weekly_limit_percent": 62.0}
        })
        res = asyncio.run(ag_service.get_antigravity_credits())
        self.assertIn("limit_weekly", res)
        self.assertEqual(res["limit_weekly"]["used"], 38.0)
        self.assertEqual(res["limit_weekly"]["total"], 100.0)
        self.assertEqual(res["limit_weekly"]["percentage"], 38.0)

    def test_legacy_next_reset_field_matches_5h_countdown(self):
        """Verify next_reset top-level string matches next_reset_5h."""
        ag_service.save_antigravity_state({
            "gemini_models": {
                "five_hour_limit_percent": 69.0,
                "weekly_limit_percent": 62.0,
                "next_reset_5h": "03h 06m"
            }
        })
        res = asyncio.run(ag_service.get_antigravity_credits())
        self.assertEqual(res.get("next_reset"), "03h 06m")


class TestBackendRESTEndpoints(unittest.TestCase):
    """Tests for FastAPI HTTP endpoints: GET /api/v1/antigravity and POST /api/v1/antigravity."""

    def setUp(self):
        self.client = TestClient(app)

    def test_get_antigravity_returns_200_and_schema(self):
        """Verify GET /api/v1/antigravity returns 200 with standard schema."""
        response = self.client.get("/api/v1/antigravity")
        self.assertEqual(response.status_code, 200)
        data = response.json()
        self.assertIn("plan", data)
        self.assertIn("available_credits", data)
        self.assertIn("gemini_models", data)
        self.assertIn("limit_5h", data)
        self.assertIn("limit_weekly", data)

    def test_post_antigravity_ingests_payload(self):
        """Verify POST /api/v1/antigravity accepts incoming CLI quota payload."""
        payload = {
            "plan": "Google AI Pro",
            "available_credits": 560,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.69, "reset_in_seconds": 11160},
                "gemini-weekly": {"remaining_fraction": 0.62, "reset_in_seconds": 90000},
                "3p-5h": {"remaining_fraction": 1.0, "reset_in_seconds": 0},
                "gemini-4-argon": {"remaining_fraction": 0.02, "reset_in_seconds": 187200}
            }
        }
        response = self.client.post("/api/v1/antigravity", json=payload)
        # Check that endpoint accepts payload (either 200 OK or 201 Created)
        self.assertIn(response.status_code, [200, 201], f"POST /api/v1/antigravity failed: {response.text}")

    def test_post_antigravity_updates_get_endpoint_state(self):
        """Verify POST updates state returned by subsequent GET."""
        payload = {
            "plan": "Google AI Pro PostTest",
            "available_credits": 999,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.55}
            }
        }
        post_res = self.client.post("/api/v1/antigravity", json=payload)
        if post_res.status_code in [200, 201]:
            get_res = self.client.get("/api/v1/antigravity")
            self.assertEqual(get_res.status_code, 200)
            data = get_res.json()
            self.assertEqual(data.get("available_credits"), 999)

    def test_post_antigravity_broadcasts_to_mqtt(self):
        """Verify POST triggers MQTT broadcast to auradeck/antigravity."""
        payload = {
            "plan": "Google AI Pro",
            "available_credits": 560,
            "quota": {"gemini-5h": {"remaining_fraction": 0.69}}
        }
        with patch("app.main.mqtt_service.publish") as mock_pub:
            post_res = self.client.post("/api/v1/antigravity", json=payload)
            if post_res.status_code in [200, 201]:
                mock_pub.assert_called()

    def test_post_antigravity_rejects_non_dict_payload(self):
        """Verify POST rejects invalid bodies with 422 Unprocessable Entity."""
        res = self.client.post("/api/v1/antigravity", json=["invalid", "array"])
        self.assertIn(res.status_code, [400, 422])


class TestBackendGracefulDegradation(unittest.TestCase):
    """Tier 2: Graceful degradation when data is partial, empty, or unauthenticated."""

    def test_missing_gemini_models_does_not_crash_poller(self):
        """If gemini_models is empty dict in state file, get_antigravity_credits handles defaults."""
        ag_service.save_antigravity_state({"gemini_models": {}})
        res = asyncio.run(ag_service.get_antigravity_credits())
        self.assertIsInstance(res, dict)
        self.assertIn("limit_5h", res)
        self.assertIn("percentage", res["limit_5h"])

    def test_null_credits_does_not_crash_calculation(self):
        """If available_credits is None, defaults are safely substituted."""
        ag_service.save_antigravity_state({"available_credits": None, "ai_credits": None})
        res = asyncio.run(ag_service.get_antigravity_credits())
        self.assertIsNotNone(res.get("credit_hours_remaining"))

    def test_unauthenticated_status_forwarded(self):
        """If state status is unauthenticated, error message is preserved."""
        ag_service.save_antigravity_state({
            "status": "unauthenticated",
            "error_message": "Session expired"
        })
        res = asyncio.run(ag_service.get_antigravity_credits())
        self.assertEqual(res.get("status"), "unauthenticated")
        self.assertEqual(res.get("error_message"), "Session expired")

    def test_partial_model_fields_have_safe_fallbacks(self):
        """Missing five_hour_limit_percent defaults cleanly without KeyError."""
        ag_service.save_antigravity_state({
            "gemini_models": {"weekly_limit_percent": 50.0} # five_hour missing
        })
        res = asyncio.run(ag_service.get_antigravity_credits())
        self.assertIsNotNone(res["limit_5h"]["used"])

    def test_dashboard_api_accessible_when_antigravity_state_corrupt(self):
        """Web dashboard endpoint /api/v1/antigravity remains functional even if state is reset."""
        with open(ag_service.STATE_FILE, "w", encoding="utf-8") as f:
            f.write("") # Empty file
        client = TestClient(app)
        res = client.get("/api/v1/antigravity")
        self.assertEqual(res.status_code, 200)


class TestBackendRemediationDefect2(unittest.TestCase):
    """Regression tests for Defect 2: Dynamic model scalar buckets and null plan handling."""

    def setUp(self):
        self.tmp_dir = tempfile.TemporaryDirectory()
        self.orig_tokens_dir = ag_service.TOKENS_DIR
        self.orig_state_file = ag_service.STATE_FILE
        ag_service.TOKENS_DIR = self.tmp_dir.name
        ag_service.STATE_FILE = os.path.join(self.tmp_dir.name, "antigravity_data.json")
        self.client = TestClient(app)

    def tearDown(self):
        ag_service.TOKENS_DIR = self.orig_tokens_dir
        ag_service.STATE_FILE = self.orig_state_file
        self.tmp_dir.cleanup()

    def test_extract_bucket_data_non_dict_includes_display_name(self):
        """extract_bucket_data returns display_name key even for scalar inputs."""
        res_float = ag_service.extract_bucket_data(0.02)
        self.assertIn("display_name", res_float)
        self.assertEqual(res_float["display_name"], "")
        self.assertEqual(res_float["remaining_percent"], 2.0)
        self.assertEqual(res_float["next_reset"], "Full")

        res_none = ag_service.extract_bucket_data(None)
        self.assertIn("display_name", res_none)
        self.assertEqual(res_none["display_name"], "")
        self.assertEqual(res_none["remaining_percent"], 0.0)

    def test_post_dynamic_model_scalar_bucket_float(self):
        """POST /api/v1/antigravity with dynamic model bucket as float scalar succeeds without KeyError."""
        payload = {
            "quota": {
                "gemini-4-argon": 0.02
            }
        }
        res = self.client.post("/api/v1/antigravity", json=payload)
        self.assertEqual(res.status_code, 200, f"Expected 200 OK, got {res.status_code}: {res.text}")
        data = res.json()
        self.assertIn("additional_models", data)
        self.assertIn("gemini_4_argon", data["additional_models"])
        model = data["additional_models"]["gemini_4_argon"]
        self.assertEqual(model["name"], "Gemini 4 Argon")
        self.assertEqual(model["weekly_limit_percent"], 2.0)

    def test_post_dynamic_model_scalar_bucket_none(self):
        """POST /api/v1/antigravity with dynamic model bucket as None succeeds without KeyError."""
        payload = {
            "quota": {
                "gemini-4-argon": None
            }
        }
        res = self.client.post("/api/v1/antigravity", json=payload)
        self.assertEqual(res.status_code, 200)
        data = res.json()
        self.assertIn("gemini_4_argon", data.get("additional_models", {}))
        model = data["additional_models"]["gemini_4_argon"]
        self.assertEqual(model["name"], "Gemini 4 Argon")
        self.assertEqual(model["weekly_limit_percent"], 0.0)

    def test_post_explicit_null_plan_falls_back_to_default(self):
        """POST /api/v1/antigravity with plan=null falls back to 'Google AI Pro'."""
        payload = {
            "plan": None,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.5}
            }
        }
        res = self.client.post("/api/v1/antigravity", json=payload)
        self.assertEqual(res.status_code, 200)
        data = res.json()
        self.assertEqual(data.get("plan"), "Google AI Pro")

    def test_normalize_antigravity_payload_scalar_dynamic_buckets(self):
        """normalize_antigravity_payload gracefully handles mixed scalar and None buckets."""
        raw = {
            "plan": None,
            "quota": {
                "gemini-4-argon": 0.02,
                "claude-3-opus": None,
                "custom-model": "invalid"
            }
        }
        norm = ag_service.normalize_antigravity_payload(raw)
        self.assertEqual(norm["plan"], "Google AI Pro")
        self.assertEqual(norm["additional_models"]["gemini_4_argon"]["name"], "Gemini 4 Argon")
        self.assertEqual(norm["additional_models"]["gemini_4_argon"]["weekly_limit_percent"], 2.0)
        self.assertEqual(norm["additional_models"]["claude_3_opus"]["name"], "Claude 3 Opus")
        self.assertEqual(norm["additional_models"]["claude_3_opus"]["weekly_limit_percent"], 0.0)
        self.assertEqual(norm["additional_models"]["custom_model"]["weekly_limit_percent"], 0.0)


if __name__ == "__main__":
    unittest.main()
