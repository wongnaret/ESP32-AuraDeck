"""
Unit tests for GCP Billing service (BigQuery export discovery, query parsing, and fallback).
"""

import unittest
import os
import sys
from datetime import datetime, timedelta

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BACKEND_DIR = os.path.join(REPO_ROOT, "backend")
if BACKEND_DIR not in sys.path:
    sys.path.insert(0, BACKEND_DIR)
if REPO_ROOT not in sys.path:
    sys.path.insert(0, REPO_ROOT)

from app.services.gcp_billing import (
    _parse_bigquery_billing_rows,
    _compile_project_billing_from_records,
    _generate_fallback_project
)


class TestGcpBillingService(unittest.TestCase):
    def test_parse_bigquery_billing_rows(self):
        sample_bq_response = {
            "schema": {
                "fields": [
                    {"name": "usage_date", "type": "DATE"},
                    {"name": "service_name", "type": "STRING"},
                    {"name": "currency", "type": "STRING"},
                    {"name": "gross_cost", "type": "FLOAT"},
                    {"name": "credits", "type": "FLOAT"},
                    {"name": "net_cost", "type": "FLOAT"}
                ]
            },
            "rows": [
                {
                    "f": [
                        {"v": "2026-11-08"},
                        {"v": "Generative Language API"},
                        {"v": "THB"},
                        {"v": "38.0"},
                        {"v": "-10.0"},
                        {"v": "28.0"}
                    ]
                },
                {
                    "f": [
                        {"v": "2026-11-15"},
                        {"v": "Compute Engine"},
                        {"v": "THB"},
                        {"v": "15.0"},
                        {"v": "0.0"},
                        {"v": "15.0"}
                    ]
                }
            ]
        }
        records = _parse_bigquery_billing_rows(sample_bq_response)
        self.assertEqual(len(records), 2)
        self.assertEqual(records[0]["usage_date"], "2026-11-08")
        self.assertEqual(records[0]["service_name"], "Generative Language API")
        self.assertEqual(records[0]["gross_cost"], 38.0)
        self.assertEqual(records[0]["credits"], -10.0)
        self.assertEqual(records[0]["net_cost"], 28.0)
        self.assertEqual(records[0]["currency"], "THB")

    def test_compile_project_billing_from_records(self):
        now = datetime.now()
        d1 = now.strftime("%Y-%m-%d")
        d2 = (now - timedelta(days=1)).strftime("%Y-%m-%d")

        records = [
            {
                "usage_date": d1,
                "service_name": "Generative Language API",
                "currency": "THB",
                "gross_cost": 50.0,
                "credits": -20.0,
                "net_cost": 30.0
            },
            {
                "usage_date": d2,
                "service_name": "Compute Engine",
                "currency": "THB",
                "gross_cost": 30.0,
                "credits": -5.0,
                "net_cost": 25.0
            }
        ]
        result = _compile_project_billing_from_records("trailhub-main", "TrailHub", records, default_currency="THB")
        self.assertEqual(result["project_id"], "trailhub-main")
        self.assertEqual(result["project_name"], "TrailHub")
        self.assertEqual(result["currency"], "THB")
        self.assertEqual(result["cost_mtd"], 55.0)
        self.assertEqual(result["gross_cost_mtd"], 80.0)
        self.assertEqual(result["savings_mtd"], 25.0)
        self.assertTrue(result["is_live"])
        self.assertEqual(len(result["daily_costs"]), 10)
        self.assertEqual(len(result["service_breakdown"]), 2)
        self.assertEqual(result["service_breakdown"][0]["service"], "Generative Language API")
        self.assertEqual(result["service_breakdown"][0]["cost"], 30.0)

    def test_fallback_project_structure(self):
        fb = _generate_fallback_project("test-proj", "Test Project", currency="USD")
        self.assertEqual(fb["project_id"], "test-proj")
        self.assertEqual(fb["currency"], "USD")
        self.assertIn("cost_mtd", fb)
        self.assertIn("forecast_end_of_month", fb)
        self.assertEqual(len(fb["daily_costs"]), 10)
        self.assertEqual(len(fb["service_breakdown"]), 4)


if __name__ == "__main__":
    unittest.main()
