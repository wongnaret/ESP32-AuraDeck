"""
API integration tests for /api/v1/antigravity GET and POST endpoints using FastAPI TestClient.
"""

import unittest
from unittest.mock import MagicMock, patch
import os
import sys
import json

# Add backend and repository root to sys.path
REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BACKEND_DIR = os.path.join(REPO_ROOT, "backend")
if BACKEND_DIR not in sys.path:
    sys.path.insert(0, BACKEND_DIR)
if REPO_ROOT not in sys.path:
    sys.path.insert(0, REPO_ROOT)

from fastapi.testclient import TestClient
from app.main import app


class TestAntigravityAPI(unittest.TestCase):

    def setUp(self):
        self.client = TestClient(app)

    @patch("app.main.mqtt_service.publish")
    def test_get_antigravity_credits_api(self, mock_mqtt):
        response = self.client.get("/api/v1/antigravity")
        self.assertEqual(response.status_code, 200)
        data = response.json()

        self.assertIn("plan", data)
        self.assertIn("available_credits", data)
        self.assertIn("gemini_models", data)
        self.assertIn("claude_gpt_models", data)
        self.assertIn("credit_hours_remaining", data)
        self.assertIn("limit_5h", data)

        # Check MQTT publish was triggered
        self.assertTrue(mock_mqtt.called)
        calls = [c[0] for c in mock_mqtt.call_args_list]
        topics = [c[0] for c in calls]
        self.assertIn("auradeck/antigravity", topics)

    @patch("app.main.mqtt_service.publish")
    def test_post_antigravity_credits_api(self, mock_mqtt):
        fixture_path = os.path.join(REPO_ROOT, "tests", "fixtures", "nominal_quota.json")
        with open(fixture_path, "r", encoding="utf-8") as f:
            raw_payload = json.load(f)

        response = self.client.post("/api/v1/antigravity", json=raw_payload)
        self.assertEqual(response.status_code, 200)
        data = response.json()

        self.assertEqual(data["status"], "ok")
        self.assertEqual(data["plan"], "Google AI Pro")
        self.assertEqual(data["available_credits"], 560)
        self.assertEqual(data["gemini_models"]["five_hour_limit_percent"], 69.0)
        self.assertEqual(data["gemini_models"]["weekly_limit_percent"], 62.0)
        self.assertEqual(data["claude_gpt_models"]["five_hour_limit_percent"], 100.0)

        # Dynamic model check
        self.assertIn("gemini_4_argon", data["additional_models"])
        self.assertEqual(data["additional_models"]["gemini_4_argon"]["weekly_limit_percent"], 2.0)

        # Check MQTT publish broadcast
        self.assertTrue(mock_mqtt.called)
        topics = [c[0][0] for c in mock_mqtt.call_args_list]
        self.assertIn("auradeck/antigravity", topics)

    @patch("app.main.mqtt_service.publish")
    def test_post_antigravity_boundary_api(self, mock_mqtt):
        boundary_payload = {
            "plan": "Google AI Ultra",
            "credits": 0,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.0},
                "gemini-weekly": {"remaining_fraction": 0.50}
            }
        }
        response = self.client.post("/api/v1/antigravity", json=boundary_payload)
        self.assertEqual(response.status_code, 200)
        data = response.json()
        self.assertEqual(data["available_credits"], 0)
        self.assertEqual(data["gemini_models"]["five_hour_limit_percent"], 0.0)
        self.assertEqual(data["gemini_models"]["weekly_limit_percent"], 50.0)


if __name__ == "__main__":
    unittest.main()
