"""
Unit and integration tests for cyberdeck_statusline.py CLI bridge.
"""

import unittest
import os
import sys
import json
import time
import tempfile
import subprocess
from http.server import HTTPServer, BaseHTTPRequestHandler
import threading

# Add root directory to sys.path
REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if REPO_ROOT not in sys.path:
    sys.path.insert(0, REPO_ROOT)

from cyberdeck_statusline import (
    format_duration,
    parse_percentage,
    extract_quota_bucket,
    parse_antigravity_input,
    format_statusline,
    build_relay_payload,
    execute_transports,
    load_config
)


class TestStatuslineUnit(unittest.TestCase):
    """Tests core parsing and formatting functions of statusline bridge."""

    def test_format_duration(self):
        self.assertEqual(format_duration(0), "Full")
        self.assertEqual(format_duration(-10), "Full")
        self.assertEqual(format_duration(45), "45s")
        self.assertEqual(format_duration(60), "1m")
        self.assertEqual(format_duration(1800), "30m")
        self.assertEqual(format_duration(11160), "03h 06m")
        self.assertEqual(format_duration(90000), "1 day, 1 hour")
        self.assertEqual(format_duration(187200), "2 days, 4 hours")
        self.assertEqual(format_duration(86400 * 3), "3 days")

    def test_parse_percentage(self):
        # Fraction inputs (0.0 - 1.0)
        self.assertEqual(parse_percentage(0.69), 69.0)
        self.assertEqual(parse_percentage(0.62), 62.0)
        self.assertEqual(parse_percentage(1.0), 100.0)
        self.assertEqual(parse_percentage(0.0), 0.0)
        self.assertEqual(parse_percentage(0.02), 2.0)

        # Percentage inputs (1.0 - 100.0)
        self.assertEqual(parse_percentage(69.0), 69.0)
        self.assertEqual(parse_percentage(100.0), 100.0)
        self.assertEqual(parse_percentage(2.0), 2.0)

        # Clamping and edge cases
        self.assertEqual(parse_percentage(150.0), 100.0)
        self.assertEqual(parse_percentage(-10.0), 0.0)
        self.assertEqual(parse_percentage("invalid"), 0.0)
        self.assertEqual(parse_percentage(None), 0.0)

    def test_extract_quota_bucket(self):
        bucket_data = {
            "remaining_fraction": 0.69,
            "reset_in_seconds": 11160,
            "reset_time": "2026-11-13T08:46:00Z"
        }
        extracted = extract_quota_bucket(bucket_data)
        self.assertEqual(extracted["remaining_percent"], 69.0)
        self.assertEqual(extracted["remaining_fraction"], 0.69)
        self.assertEqual(extracted["reset_in_seconds"], 11160)
        self.assertEqual(extracted["next_reset"], "03h 06m")

        # Human-readable reset_time passthrough
        bucket_hr = {
            "remaining_percent": 62.0,
            "reset_time": "1 day, 1 hour"
        }
        extracted_hr = extract_quota_bucket(bucket_hr)
        self.assertEqual(extracted_hr["remaining_percent"], 62.0)
        self.assertEqual(extracted_hr["next_reset"], "1 day, 1 hour")

    def test_parse_antigravity_input_nominal(self):
        fixture_path = os.path.join(REPO_ROOT, "tests", "fixtures", "nominal_quota.json")
        with open(fixture_path, "r", encoding="utf-8") as f:
            raw_json = f.read()

        parsed = parse_antigravity_input(raw_json)
        self.assertEqual(parsed["status"], "ok")
        self.assertEqual(parsed["plan"], "Google AI Pro")
        self.assertEqual(parsed["credits"], 560)
        self.assertEqual(parsed["gemini_5h"]["remaining_percent"], 69.0)
        self.assertEqual(parsed["gemini_weekly"]["remaining_percent"], 62.0)
        self.assertEqual(parsed["3p_5h"]["remaining_percent"], 100.0)
        self.assertEqual(parsed["3p_weekly"]["remaining_percent"], 100.0)

        # Dynamic model check
        dyn = parsed["dynamic_models"]
        self.assertEqual(len(dyn), 1)
        self.assertEqual(dyn[0]["name"], "Gemini 4 Argon")
        self.assertEqual(dyn[0]["remaining_percent"], 2.0)
        self.assertEqual(dyn[0]["next_reset"], "2 days, 4 hours")

    def test_format_statusline_plain(self):
        config = {
            "statusline": {
                "use_ansi": False,
                "show_credits": True,
                "show_countdowns": False,
                "max_dynamic_models": 2
            }
        }
        data = {
            "status": "ok",
            "credits": 560,
            "gemini_5h": {"remaining_percent": 69.0, "next_reset": "03h 06m"},
            "gemini_weekly": {"remaining_percent": 62.0, "next_reset": "1 day, 1 hour"},
            "3p_5h": {"remaining_percent": 100.0, "next_reset": "Full"},
            "3p_weekly": {"remaining_percent": 100.0, "next_reset": "Full"},
            "dynamic_models": [
                {"name": "Gemini 4 Argon", "remaining_percent": 2.0, "next_reset": "2 days, 4 hours"}
            ]
        }
        line = format_statusline(data, config)
        expected = "Gemini 5h: 69% | Wk: 62% | 3P: 100% | Argon: 2% | Credits: 560"
        self.assertEqual(line, expected)

    def test_format_statusline_ansi(self):
        config = {
            "statusline": {
                "use_ansi": True,
                "show_credits": True,
                "show_countdowns": False,
                "max_dynamic_models": 2
            }
        }
        data = {
            "status": "ok",
            "credits": 560,
            "gemini_5h": {"remaining_percent": 69.0, "next_reset": "03h 06m"},
            "gemini_weekly": {"remaining_percent": 62.0, "next_reset": "1 day, 1 hour"},
            "3p_5h": {"remaining_percent": 100.0, "next_reset": "Full"},
            "3p_weekly": {"remaining_percent": 100.0, "next_reset": "Full"},
            "dynamic_models": [
                {"name": "Gemini 4 Argon", "remaining_percent": 2.0, "next_reset": "2 days, 4 hours"}
            ]
        }
        line = format_statusline(data, config)
        # Should contain ANSI color escape codes
        self.assertIn("\033[32m", line)  # Green for >=50%
        self.assertIn("\033[31m", line)  # Red for 2%
        self.assertIn("\033[0m", line)   # Reset

    def test_build_relay_payload(self):
        fixture_path = os.path.join(REPO_ROOT, "tests", "fixtures", "nominal_quota.json")
        with open(fixture_path, "r", encoding="utf-8") as f:
            raw_json = f.read()

        parsed = parse_antigravity_input(raw_json)
        payload = build_relay_payload(parsed)

        self.assertEqual(payload["status"], "ok")
        self.assertEqual(payload["plan"], "Google AI Pro")
        self.assertEqual(payload["available_credits"], 560)
        self.assertEqual(payload["ai_credits"], 560)
        self.assertEqual(payload["gemini_models"]["five_hour_limit_percent"], 69.0)
        self.assertEqual(payload["gemini_models"]["weekly_limit_percent"], 62.0)
        self.assertEqual(payload["gemini_models"]["next_reset_5h"], "03h 06m")
        self.assertEqual(payload["gemini_models"]["next_reset_weekly"], "1 day, 1 hour")
        self.assertEqual(payload["claude_gpt_models"]["five_hour_limit_percent"], 100.0)

        # Dynamic model in both list and dict
        self.assertIn("gemini_4_argon", payload["additional_models"])
        self.assertEqual(payload["additional_models"]["gemini_4_argon"]["weekly_limit_percent"], 2.0)

        # Legacy fields
        self.assertEqual(payload["credit_hours_remaining"], 560.0)
        self.assertEqual(payload["percent_quota_used"], 31.0)
        self.assertEqual(payload["limit_5h"]["used"], 31.0)
        self.assertEqual(payload["limit_weekly"]["used"], 38.0)


class TestStatuslineCLI(unittest.TestCase):
    """Integration tests running cyberdeck_statusline.py as a subprocess."""

    def setUp(self):
        self.script_path = os.path.join(REPO_ROOT, "cyberdeck_statusline.py")
        self.python_bin = sys.executable

    def test_cli_pipe_execution_and_speed(self):
        fixture_path = os.path.join(REPO_ROOT, "tests", "fixtures", "nominal_quota.json")
        with open(fixture_path, "r", encoding="utf-8") as f:
            stdin_text = f.read()

        # In-process benchmark for strict parsing and formatting speed (<50ms budget)
        t_in = time.perf_counter()
        cfg = load_config()
        parsed = parse_antigravity_input(stdin_text)
        line = format_statusline(parsed, cfg)
        in_process_ms = (time.perf_counter() - t_in) * 1000.0
        self.assertLess(in_process_ms, 50.0)

        # External subprocess execution benchmark (accounting for Windows Python interpreter startup)
        t0 = time.time()
        res = subprocess.run(
            [self.python_bin, self.script_path, "--no-color"],
            input=stdin_text,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        elapsed_ms = (time.time() - t0) * 1000

        self.assertEqual(res.returncode, 0)
        self.assertEqual(res.stderr, "")  # Strict silence on stderr
        lines = res.stdout.strip().splitlines()
        self.assertEqual(len(lines), 1)  # Strictly single line
        self.assertIn("Gemini 5h: 69%", lines[0])
        self.assertIn("Argon: 2%", lines[0])
        self.assertIn("Credits: 560", lines[0])
        self.assertLess(elapsed_ms, 1500.0)  # Generous threshold for Windows process spawn

    def test_strict_silence_on_invalid_input(self):
        # Pass completely invalid JSON
        res = subprocess.run(
            [self.python_bin, self.script_path],
            input="NOT VALID JSON {[[ 42",
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        self.assertEqual(res.returncode, 0)
        self.assertEqual(res.stderr, "")
        self.assertIn("Antigravity Quota: Unavailable", res.stdout)

    def test_strict_silence_on_empty_input(self):
        res = subprocess.run(
            [self.python_bin, self.script_path],
            input="",
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        self.assertEqual(res.returncode, 0)
        self.assertEqual(res.stderr, "")
        self.assertIn("Antigravity Quota: Unavailable", res.stdout)

    def test_debug_dump_flag(self):
        with tempfile.NamedTemporaryFile(delete=False, suffix=".json") as tmp:
            dump_file = tmp.name

        try:
            fixture_path = os.path.join(REPO_ROOT, "tests", "fixtures", "nominal_quota.json")
            with open(fixture_path, "r", encoding="utf-8") as f:
                stdin_text = f.read()

            res = subprocess.run(
                [self.python_bin, self.script_path, "--debug-dump", dump_file],
                input=stdin_text,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            self.assertEqual(res.returncode, 0)
            self.assertTrue(os.path.exists(dump_file))
            with open(dump_file, "r", encoding="utf-8") as f:
                dumped_content = f.read()
            self.assertEqual(dumped_content, stdin_text)
        finally:
            if os.path.exists(dump_file):
                os.remove(dump_file)


class MockHTTPHandler(BaseHTTPRequestHandler):
    received_payload = None

    def do_POST(self):
        content_length = int(self.headers.get("Content-Length", 0))
        post_data = self.rfile.read(content_length)
        MockHTTPHandler.received_payload = json.loads(post_data.decode("utf-8"))
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.end_headers()
        self.wfile.write(b'{"status": "ok"}')

    def log_message(self, format, *args):
        # Silence HTTP server logs
        pass


class TestStatuslineTransports(unittest.TestCase):
    """Tests atomic file writing and REST POST transports."""

    def test_atomic_file_transport(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            target_path = os.path.join(tmpdir, "tokens", "antigravity_data.json")
            config = {
                "transports": {
                    "file": {"enabled": True, "path": target_path},
                    "rest": {"enabled": False},
                    "mqtt": {"enabled": False}
                },
                "debug": {}
            }
            payload = {"status": "ok", "plan": "Test Plan", "credits": 42}
            execute_transports(payload, config)

            self.assertTrue(os.path.exists(target_path))
            with open(target_path, "r", encoding="utf-8") as f:
                saved = json.load(f)
            self.assertEqual(saved["plan"], "Test Plan")
            self.assertEqual(saved["credits"], 42)

    def test_rest_post_transport(self):
        # Spin up local mock HTTP server
        server = HTTPServer(("127.0.0.1", 0), MockHTTPHandler)
        port = server.server_port
        thread = threading.Thread(target=server.handle_request)
        thread.daemon = True
        thread.start()

        config = {
            "transports": {
                "file": {"enabled": False},
                "rest": {"enabled": True, "url": f"http://127.0.0.1:{port}/api/v1/antigravity", "timeout_seconds": 0.5},
                "mqtt": {"enabled": False}
            },
            "debug": {}
        }
        payload = {"status": "ok", "plan": "REST Plan", "available_credits": 777}
        execute_transports(payload, config)
        thread.join(timeout=1.0)
        server.server_close()

        self.assertIsNotNone(MockHTTPHandler.received_payload)
        self.assertEqual(MockHTTPHandler.received_payload["plan"], "REST Plan")
        self.assertEqual(MockHTTPHandler.received_payload["available_credits"], 777)


if __name__ == "__main__":
    unittest.main()
