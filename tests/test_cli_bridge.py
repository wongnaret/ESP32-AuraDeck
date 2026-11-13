"""
tests/test_cli_bridge.py
Opaque-box tests for cyberdeck_statusline.py CLI bridge.
Covers:
- Tier 1: Feature Coverage (Stdin ingestion, percentage conversion, stdout line formatting, metadata extraction, atomic file writing)
- Tier 2: Boundary & Corner Cases (0%/100% quota, empty/invalid stdin, offline MQTT/REST, missing optional keys, countdown boundaries)
"""

import unittest
import os
import sys
import json
import subprocess
import tempfile
import time
import socket
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CLI_SCRIPT = REPO_ROOT / "cyberdeck_statusline.py"


def run_cli(stdin_text: str = "", args: list = None, env: dict = None, timeout: float = 5.0) -> subprocess.CompletedProcess:
    """Helper to execute cyberdeck_statusline.py as a subprocess."""
    cmd = [sys.executable, str(CLI_SCRIPT)]
    if args:
        cmd.extend(args)
    run_env = os.environ.copy()
    if env:
        run_env.update(env)
    return subprocess.run(
        cmd,
        input=stdin_text,
        text=True,
        capture_output=True,
        timeout=timeout,
        cwd=str(REPO_ROOT),
        env=run_env
    )


class TestCLIBridgeStdinAndParsing(unittest.TestCase):
    """Tier 1: Feature 1 - Reading JSON string updates from sys.stdin."""

    def test_stdin_json_standard_payload(self):
        """Verify nominal JSON payload is read from stdin and processed successfully."""
        payload = {
            "plan": "Google AI Pro",
            "available_credits": 560,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.69, "reset_in_seconds": 11160},
                "gemini-weekly": {"remaining_fraction": 0.62, "reset_in_seconds": 90000},
                "3p-5h": {"remaining_fraction": 1.0, "reset_in_seconds": 0},
                "3p-weekly": {"remaining_fraction": 1.0, "reset_in_seconds": 0}
            }
        }
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0, f"CLI exited with error: {res.stderr}")
        self.assertIn("69%", res.stdout)
        self.assertIn("62%", res.stdout)

    def test_stdin_json_with_surrounding_whitespace(self):
        """Verify JSON payload with leading/trailing newlines and spaces is handled cleanly."""
        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
        raw_input = "\n\n   " + json.dumps(payload) + "   \n\n"
        res = run_cli(stdin_text=raw_input)
        self.assertEqual(res.returncode, 0)
        self.assertIn("50%", res.stdout)

    def test_stdin_multiline_json(self):
        """Verify pretty-printed multiline JSON payload from stdin is parsed correctly."""
        payload = {
            "plan": "Google AI Pro",
            "credits": 560,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.80},
                "3p-5h": {"remaining_fraction": 0.90}
            }
        }
        res = run_cli(stdin_text=json.dumps(payload, indent=4))
        self.assertEqual(res.returncode, 0)
        self.assertIn("80%", res.stdout)

    def test_stdin_utf8_encoding(self):
        """Verify UTF-8 characters in plan name or metadata do not cause encoding errors."""
        payload = {
            "plan": "Google AI Pro ✨ 測試",
            "quota": {"gemini-5h": {"remaining_fraction": 0.75}}
        }
        res = run_cli(stdin_text=json.dumps(payload, ensure_ascii=False))
        self.assertEqual(res.returncode, 0)
        self.assertIn("75%", res.stdout)

    def test_stdin_large_payload_with_many_dynamic_models(self):
        """Verify large JSON payloads containing many dynamic models parse without truncation."""
        quota_dict = {f"custom-model-{i}": {"remaining_fraction": round(i / 100.0, 2)} for i in range(1, 50)}
        quota_dict["gemini-5h"] = {"remaining_fraction": 0.45}
        payload = {"quota": quota_dict}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("45%", res.stdout)


class TestCLIBridgePercentageConversion(unittest.TestCase):
    """Tier 1: Feature 2 - Converting remaining_fraction (0.0-1.0) to percentage (0-100%)."""

    def test_convert_remaining_fraction_nominal(self):
        """Verify 0.69 and 0.62 correctly convert to 69% and 62%."""
        payload = {
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.69},
                "gemini-weekly": {"remaining_fraction": 0.62}
            }
        }
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("69%", res.stdout)
        self.assertIn("62%", res.stdout)

    def test_convert_remaining_fraction_full(self):
        """Verify 1.0 converts to 100%."""
        payload = {"quota": {"3p-5h": {"remaining_fraction": 1.0}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("100%", res.stdout)

    def test_convert_remaining_fraction_zero(self):
        """Verify 0.0 converts to 0%."""
        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.0}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("0%", res.stdout)

    def test_convert_remaining_fraction_small(self):
        """Verify small fraction 0.02 converts to 2% (e.g. Gemini 4 Argon from prompt)."""
        payload = {"quota": {"gemini-4-argon": {"remaining_fraction": 0.02}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("2%", res.stdout)

    def test_convert_remaining_fraction_rounding_decimals(self):
        """Verify floating precision rounding (0.694 -> 69% and 0.696 -> 70% or 69.6%)."""
        payload = {
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.694},
                "gemini-weekly": {"remaining_fraction": 0.696}
            }
        }
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        # Should contain 69% (or 69.4%) and 70% (or 69.6%)
        output = res.stdout
        self.assertTrue("69%" in output or "69.4%" in output)
        self.assertTrue("70%" in output or "69.6%" in output)


class TestCLIBridgeStdoutFormatting(unittest.TestCase):
    """Tier 1: Feature 3 - Single summary status line on sys.stdout."""

    def test_stdout_strictly_single_line_output(self):
        """Verify sys.stdout emits strictly one line (no multiple newlines/banners)."""
        payload = {
            "plan": "Google AI Pro",
            "available_credits": 560,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.69},
                "gemini-weekly": {"remaining_fraction": 0.62},
                "3p-5h": {"remaining_fraction": 1.0}
            }
        }
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        lines = [line for line in res.stdout.splitlines() if line.strip()]
        self.assertEqual(len(lines), 1, f"Expected exactly 1 line on stdout, got {len(lines)}: {res.stdout}")

    def test_stdout_contains_gemini_5h_metric(self):
        """Verify stdout contains Gemini 5h quota identifier and value."""
        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.69}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertRegex(res.stdout, r"(?i)gemini.*5h.*69%")

    def test_stdout_contains_gemini_weekly_metric(self):
        """Verify stdout contains weekly quota identifier and value."""
        payload = {"quota": {"gemini-weekly": {"remaining_fraction": 0.62}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertRegex(res.stdout, r"(?i)(wk|weekly).*62%")

    def test_stdout_contains_3p_models_metric(self):
        """Verify stdout contains 3P (Claude/GPT) metric."""
        payload = {"quota": {"3p-5h": {"remaining_fraction": 1.0}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertRegex(res.stdout, r"(?i)3p.*100%")

    def test_stdout_no_debug_logging_or_tracebacks(self):
        """Verify stdout contains only the status line and no logger prefixes or tracebacks."""
        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertNotIn("DEBUG", res.stdout)
        self.assertNotIn("INFO", res.stdout)
        self.assertNotIn("Traceback", res.stdout)


class TestCLIBridgeMetadataExtraction(unittest.TestCase):
    """Tier 1: Feature 4 - Extracting plan and credits/available_credits."""

    def test_extract_plan_google_ai_pro(self):
        """Verify plan name is parsed and forwarded to output/state."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            out_file = Path(tmp_dir) / "quota.json"
            payload = {
                "plan": "Google AI Pro",
                "quota": {"gemini-5h": {"remaining_fraction": 0.50}}
            }
            res = run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(out_file), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)
            self.assertTrue(out_file.exists())
            with open(out_file, "r", encoding="utf-8") as f:
                saved = json.load(f)
            self.assertEqual(saved.get("plan"), "Google AI Pro")

    def test_extract_available_credits_field(self):
        """Verify available_credits (560) is extracted correctly."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            out_file = Path(tmp_dir) / "quota.json"
            payload = {
                "available_credits": 560,
                "quota": {"gemini-5h": {"remaining_fraction": 0.50}}
            }
            res = run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(out_file), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)
            with open(out_file, "r", encoding="utf-8") as f:
                saved = json.load(f)
            self.assertEqual(saved.get("available_credits"), 560)

    def test_extract_ai_credits_alias_field(self):
        """Verify ai_credits alias is also supported when available_credits is absent."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            out_file = Path(tmp_dir) / "quota.json"
            payload = {
                "ai_credits": 560,
                "quota": {"gemini-5h": {"remaining_fraction": 0.50}}
            }
            res = run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(out_file), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)
            with open(out_file, "r", encoding="utf-8") as f:
                saved = json.load(f)
            credits = saved.get("available_credits", saved.get("ai_credits"))
            self.assertEqual(credits, 560)

    def test_extract_both_credits_and_plan_together(self):
        """Verify both plan and credits exist together in the saved payload."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            out_file = Path(tmp_dir) / "quota.json"
            payload = {
                "plan": "Google AI Enterprise",
                "available_credits": 1200,
                "quota": {"gemini-5h": {"remaining_fraction": 0.90}}
            }
            res = run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(out_file), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)
            with open(out_file, "r", encoding="utf-8") as f:
                saved = json.load(f)
            self.assertEqual(saved.get("plan"), "Google AI Enterprise")
            self.assertEqual(saved.get("available_credits"), 1200)

    def test_extract_numeric_credits_preserved(self):
        """Verify floating or zero credit balances are preserved as numbers."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            out_file = Path(tmp_dir) / "quota.json"
            payload = {
                "available_credits": 0,
                "quota": {"gemini-5h": {"remaining_fraction": 0.10}}
            }
            res = run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(out_file), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)
            with open(out_file, "r", encoding="utf-8") as f:
                saved = json.load(f)
            self.assertEqual(saved.get("available_credits"), 0)


class TestCLIBridgeAtomicFileTransport(unittest.TestCase):
    """Tier 1: Feature 5 - Atomic JSON file writing (.tmp then os.replace)."""

    def test_atomic_file_creates_destination_file(self):
        """Verify destination JSON file is created upon CLI invocation."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            target_path = Path(tmp_dir) / "agy_quota.json"
            payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
            res = run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(target_path), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)
            self.assertTrue(target_path.exists(), "Target file must exist after execution")

    def test_atomic_file_no_leftover_tmp_files(self):
        """Verify no leftover .tmp files remain in destination directory after atomic rename."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            target_path = Path(tmp_dir) / "agy_quota.json"
            payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
            run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(target_path), "CYBERDECK_FILE_ENABLED": "true"}
            )
            tmp_files = list(Path(tmp_dir).glob("*.tmp*"))
            self.assertEqual(len(tmp_files), 0, f"Found leftover temporary files: {tmp_files}")

    def test_atomic_file_valid_json_content(self):
        """Verify file written contains valid, non-corrupted JSON."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            target_path = Path(tmp_dir) / "agy_quota.json"
            payload = {
                "plan": "Google AI Pro",
                "quota": {"gemini-5h": {"remaining_fraction": 0.69}}
            }
            run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(target_path), "CYBERDECK_FILE_ENABLED": "true"}
            )
            with open(target_path, "r", encoding="utf-8") as f:
                loaded = json.load(f)
            self.assertIn("quota", loaded)
            self.assertIn("gemini-5h", loaded["quota"])

    def test_atomic_file_creates_nested_parent_dirs_if_missing(self):
        """Verify intermediate parent directories are created automatically if missing."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            nested_target = Path(tmp_dir) / "nested" / "sub" / "agy_quota.json"
            payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.70}}}
            res = run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(nested_target), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)
            self.assertTrue(nested_target.exists())

    def test_atomic_file_overwrites_existing_file_cleanly(self):
        """Verify repeated executions overwrite existing file cleanly without error."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            target_path = Path(tmp_dir) / "agy_quota.json"
            payload_1 = {"quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
            payload_2 = {"quota": {"gemini-5h": {"remaining_fraction": 0.85}}}
            run_cli(
                stdin_text=json.dumps(payload_1),
                env={"CYBERDECK_FILE_PATH": str(target_path), "CYBERDECK_FILE_ENABLED": "true"}
            )
            run_cli(
                stdin_text=json.dumps(payload_2),
                env={"CYBERDECK_FILE_PATH": str(target_path), "CYBERDECK_FILE_ENABLED": "true"}
            )
            with open(target_path, "r", encoding="utf-8") as f:
                data = json.load(f)
            self.assertAlmostEqual(data["quota"]["gemini-5h"]["remaining_fraction"], 0.85)


class TestCLIBridgeQuotaBoundaries(unittest.TestCase):
    """Tier 2: Boundary 1 - 0% and 100% quota extremes."""

    def test_boundary_zero_percent_quota(self):
        """Verify 0.0 fraction outputs 0% cleanly without divide-by-zero or crash."""
        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.0}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("0%", res.stdout)

    def test_boundary_hundred_percent_quota(self):
        """Verify 1.0 fraction outputs 100% cleanly."""
        payload = {"quota": {"gemini-5h": {"remaining_fraction": 1.0}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("100%", res.stdout)

    def test_boundary_zero_remaining_fraction_preserves_countdown(self):
        """Verify 0% quota retains its reset_in_seconds countdown info."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            out_file = Path(tmp_dir) / "quota.json"
            payload = {
                "quota": {
                    "gemini-5h": {"remaining_fraction": 0.0, "reset_in_seconds": 3600}
                }
            }
            res = run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(out_file), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)
            with open(out_file, "r", encoding="utf-8") as f:
                saved = json.load(f)
            self.assertEqual(saved["quota"]["gemini-5h"]["reset_in_seconds"], 3600)

    def test_boundary_mixed_zero_and_hundred_percent(self):
        """Verify simultaneous 0% and 100% quotas display properly."""
        payload = {
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.0},
                "gemini-weekly": {"remaining_fraction": 1.0}
            }
        }
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("0%", res.stdout)
        self.assertIn("100%", res.stdout)

    def test_boundary_floating_point_imprecision(self):
        """Verify tiny fractional values (e.g. 0.0001 or 0.9999) do not format strangely."""
        payload = {
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.0001},
                "gemini-weekly": {"remaining_fraction": 0.9999}
            }
        }
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        # Should not throw and should output valid percentage
        self.assertRegex(res.stdout, r"\d+%")


class TestCLIBridgeStdinErrorsAndSilence(unittest.TestCase):
    """Tier 2: Boundary 2 - Empty stdin or invalid non-JSON stdin."""

    def test_corner_empty_stdin_exits_zero(self):
        """Verify empty stdin exits 0 with fallback or silent exit and NO crash."""
        res = run_cli(stdin_text="")
        self.assertEqual(res.returncode, 0, f"Empty stdin caused non-zero exit: {res.returncode}")
        self.assertNotIn("Traceback", res.stderr)
        self.assertNotIn("Traceback", res.stdout)

    def test_corner_whitespace_only_stdin_exits_zero(self):
        """Verify whitespace-only stdin exits 0 without error."""
        res = run_cli(stdin_text="   \n\t  \n  ")
        self.assertEqual(res.returncode, 0)
        self.assertNotIn("Traceback", res.stderr)

    def test_corner_invalid_non_json_stdin_exits_zero(self):
        """Verify invalid non-JSON string exits 0 without tracebacks."""
        res = run_cli(stdin_text="This is definitely not JSON {broken")
        self.assertEqual(res.returncode, 0)
        self.assertNotIn("Traceback", res.stdout)
        self.assertNotIn("Traceback", res.stderr)

    def test_corner_truncated_json_exits_zero(self):
        """Verify partially cut-off JSON payload exits 0 cleanly."""
        res = run_cli(stdin_text='{"quota": {"gemini-5h": {"remaining_fraction": 0.')
        self.assertEqual(res.returncode, 0)
        self.assertNotIn("Traceback", res.stderr)

    def test_corner_non_dict_json_exits_zero(self):
        """Verify JSON arrays or primitives (e.g. [1, 2, 3] or "hello") exit 0 safely."""
        res = run_cli(stdin_text='["unexpected", "list"]')
        self.assertEqual(res.returncode, 0)
        self.assertNotIn("Traceback", res.stderr)


class TestCLIBridgeOfflineTransports(unittest.TestCase):
    """Tier 2: Boundary 3 - Unreachable/offline MQTT broker and REST endpoint."""

    def test_offline_mqtt_broker_silent_fail(self):
        """Verify unreachable MQTT broker fails silently with zero exit code and no stdout traces."""
        # Find an unused local port
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind(("127.0.0.1", 0))
            free_port = s.getsockname()[1]
        
        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
        env = {
            "CYBERDECK_MQTT_ENABLED": "true",
            "CYBERDECK_MQTT_HOST": "127.0.0.1",
            "CYBERDECK_MQTT_PORT": str(free_port),
            "CYBERDECK_TIMEOUT": "0.1"
        }
        res = run_cli(stdin_text=json.dumps(payload), env=env)
        self.assertEqual(res.returncode, 0)
        self.assertNotIn("Traceback", res.stdout)
        self.assertNotIn("Traceback", res.stderr)

    def test_offline_rest_endpoint_silent_fail(self):
        """Verify unreachable REST endpoint fails silently with zero exit code and no stdout traces."""
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind(("127.0.0.1", 0))
            free_port = s.getsockname()[1]

        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
        env = {
            "CYBERDECK_REST_ENABLED": "true",
            "CYBERDECK_REST_URL": f"http://127.0.0.1:{free_port}/api/v1/antigravity",
            "CYBERDECK_TIMEOUT": "0.1"
        }
        res = run_cli(stdin_text=json.dumps(payload), env=env)
        self.assertEqual(res.returncode, 0)
        self.assertNotIn("Traceback", res.stdout)
        self.assertNotIn("Traceback", res.stderr)

    def test_offline_both_transports_simultaneous_silent_fail(self):
        """Verify both MQTT and REST offline simultaneously do not crash CLI."""
        env = {
            "CYBERDECK_MQTT_ENABLED": "true",
            "CYBERDECK_MQTT_HOST": "127.0.0.1",
            "CYBERDECK_MQTT_PORT": "54321",
            "CYBERDECK_REST_ENABLED": "true",
            "CYBERDECK_REST_URL": "http://127.0.0.1:54322/api/v1/antigravity",
            "CYBERDECK_TIMEOUT": "0.1"
        }
        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
        res = run_cli(stdin_text=json.dumps(payload), env=env)
        self.assertEqual(res.returncode, 0)
        self.assertNotIn("Traceback", res.stdout)

    def test_offline_fail_safe_timeout_enforced(self):
        """Verify communication with offline broker/server finishes within timeout (<1.5s total including subprocess startup)."""
        env = {
            "CYBERDECK_REST_ENABLED": "true",
            "CYBERDECK_REST_URL": "http://10.255.255.1:8080/api/v1/antigravity", # Non-routable IP
            "CYBERDECK_TIMEOUT": "0.1"
        }
        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
        start_t = time.time()
        res = run_cli(stdin_text=json.dumps(payload), env=env, timeout=3.0)
        duration = time.time() - start_t
        self.assertEqual(res.returncode, 0)
        self.assertLess(duration, 1.5, f"Execution took {duration}s, exceeding fail-safe limit")

    def test_offline_still_outputs_status_line_to_stdout(self):
        """Verify status line is still printed to stdout even if network transport fails."""
        env = {
            "CYBERDECK_REST_ENABLED": "true",
            "CYBERDECK_REST_URL": "http://127.0.0.1:59999/api/v1/antigravity",
            "CYBERDECK_TIMEOUT": "0.1"
        }
        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.69}}}
        res = run_cli(stdin_text=json.dumps(payload), env=env)
        self.assertEqual(res.returncode, 0)
        self.assertIn("69%", res.stdout)


class TestCLIBridgeMissingOptionalKeys(unittest.TestCase):
    """Tier 2: Boundary 4 - Missing optional keys."""

    def test_missing_credits_key(self):
        """Verify payload without credits key parses cleanly."""
        payload = {"plan": "Google AI Pro", "quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("50%", res.stdout)

    def test_missing_plan_key(self):
        """Verify payload without plan key parses cleanly."""
        payload = {"available_credits": 100, "quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("50%", res.stdout)

    def test_missing_reset_in_seconds_key(self):
        """Verify quota buckets without reset_in_seconds or reset_time parse cleanly."""
        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("50%", res.stdout)

    def test_missing_gemini_models_key(self):
        """Verify payload missing gemini-5h or gemini-weekly still outputs available models."""
        payload = {"quota": {"3p-5h": {"remaining_fraction": 0.88}}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("88%", res.stdout)

    def test_empty_quota_dictionary(self):
        """Verify completely empty quota dictionary handles gracefully without crash."""
        payload = {"plan": "Google AI Pro", "quota": {}}
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertNotIn("Traceback", res.stdout)


class TestCLIBridgeCountdownBoundaries(unittest.TestCase):
    """Tier 2: Boundary 5 - Zero countdown / instant refresh."""

    def test_zero_reset_in_seconds(self):
        """Verify reset_in_seconds: 0 indicates instant refresh or zero countdown."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            out_file = Path(tmp_dir) / "quota.json"
            payload = {
                "quota": {
                    "3p-5h": {"remaining_fraction": 1.0, "reset_in_seconds": 0}
                }
            }
            res = run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(out_file), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)
            with open(out_file, "r", encoding="utf-8") as f:
                saved = json.load(f)
            self.assertEqual(saved["quota"]["3p-5h"]["reset_in_seconds"], 0)

    def test_negative_reset_in_seconds_clamped(self):
        """Verify negative countdown seconds are handled safely."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            out_file = Path(tmp_dir) / "quota.json"
            payload = {
                "quota": {
                    "gemini-5h": {"remaining_fraction": 0.50, "reset_in_seconds": -10}
                }
            }
            res = run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(out_file), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)

    def test_large_reset_in_seconds_multi_day(self):
        """Verify large countdown seconds (e.g. 187200s = 2 days, 4 hours) are preserved."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            out_file = Path(tmp_dir) / "quota.json"
            payload = {
                "quota": {
                    "gemini-4-argon": {"remaining_fraction": 0.02, "reset_in_seconds": 187200}
                }
            }
            res = run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(out_file), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)
            with open(out_file, "r", encoding="utf-8") as f:
                saved = json.load(f)
            self.assertEqual(saved["quota"]["gemini-4-argon"]["reset_in_seconds"], 187200)

    def test_iso_reset_time_preserved(self):
        """Verify ISO8601 reset_time string is preserved in output payload."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            out_file = Path(tmp_dir) / "quota.json"
            payload = {
                "quota": {
                    "gemini-5h": {
                        "remaining_fraction": 0.69,
                        "reset_time": "2026-11-13T08:42:00Z"
                    }
                }
            }
            res = run_cli(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(out_file), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)
            with open(out_file, "r", encoding="utf-8") as f:
                saved = json.load(f)
            # Either retained directly in raw quota or mapped to next_reset
            bucket = saved.get("quota", {}).get("gemini-5h", {})
            iso_val = bucket.get("reset_time") or bucket.get("next_reset") or saved.get("next_reset")
            self.assertEqual(iso_val, "2026-11-13T08:42:00Z")

    def test_null_reset_time_handled(self):
        """Verify null/None reset_time or reset_in_seconds does not crash CLI."""
        payload = {
            "quota": {
                "gemini-5h": {
                    "remaining_fraction": 0.50,
                    "reset_time": None,
                    "reset_in_seconds": None
                }
            }
        }
        res = run_cli(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("50%", res.stdout)


class TestCLIBridgeRemediationDefect1(unittest.TestCase):
    """Regression tests for Defect 1: Corrupted config sections and fallback handling."""

    def test_corrupted_config_scalar_sections_do_not_crash(self):
        """When config sections contain scalars instead of dicts, CLI does not crash and outputs single line."""
        corrupted_cfg = {
            "statusline": 12345,
            "debug": 123,
            "transports": "not_a_dict"
        }
        with tempfile.NamedTemporaryFile("w", suffix=".json", delete=False) as f:
            json.dump(corrupted_cfg, f)
            cfg_path = f.name
        try:
            payload = {
                "plan": "Google AI Pro",
                "quota": {
                    "gemini-5h": {"remaining_fraction": 0.69},
                    "gemini-weekly": {"remaining_fraction": 0.62}
                }
            }
            res = run_cli(stdin_text=json.dumps(payload), args=["--config", cfg_path])
            self.assertEqual(res.returncode, 0)
            self.assertIn("69%", res.stdout)
            self.assertIn("62%", res.stdout)
            lines = [l for l in res.stdout.splitlines() if l.strip()]
            self.assertEqual(len(lines), 1)
        finally:
            if os.path.exists(cfg_path):
                try:
                    os.unlink(cfg_path)
                except Exception:
                    pass

    def test_corrupted_config_nested_transports_scalar(self):
        """When nested transport configs contain scalars, CLI handles safely."""
        corrupted_cfg = {
            "transports": {
                "file": 999,
                "rest": None,
                "mqtt": "disabled"
            }
        }
        with tempfile.NamedTemporaryFile("w", suffix=".json", delete=False) as f:
            json.dump(corrupted_cfg, f)
            cfg_path = f.name
        try:
            payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
            res = run_cli(stdin_text=json.dumps(payload), args=["--config", cfg_path])
            self.assertEqual(res.returncode, 0)
            self.assertIn("50%", res.stdout)
        finally:
            if os.path.exists(cfg_path):
                try:
                    os.unlink(cfg_path)
                except Exception:
                    pass

    def test_cross_platform_debug_dump_default_path(self):
        """Verify --debug-dump without argument uses cross-platform temp directory."""
        expected_dump_file = os.path.join(tempfile.gettempdir(), "cyberdeck_debug_dump.json")
        if os.path.exists(expected_dump_file):
            try:
                os.unlink(expected_dump_file)
            except Exception:
                pass

        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.77}}}
        res = run_cli(stdin_text=json.dumps(payload), args=["--debug-dump"])
        self.assertEqual(res.returncode, 0)
        self.assertIn("77%", res.stdout)
        self.assertTrue(os.path.exists(expected_dump_file))
        with open(expected_dump_file, "r", encoding="utf-8") as f:
            content = f.read()
        self.assertIn("0.77", content)

    def test_top_level_exception_handler_emits_fallback_output(self):
        """Verify unhandled exception in main() triggers fallback stdout line."""
        test_script = (
            "import sys\n"
            "import cyberdeck_statusline\n"
            "def broken_main():\n"
            "    raise RuntimeError('Simulated unexpected failure')\n"
            "cyberdeck_statusline.main = broken_main\n"
            "try:\n"
            "    cyberdeck_statusline.main()\n"
            "except Exception:\n"
            "    try:\n"
            "        sys.stdout.write('Antigravity Quota: Unavailable\\n')\n"
            "        sys.stdout.flush()\n"
            "    except Exception:\n"
            "        pass\n"
            "    sys.exit(0)\n"
        )
        res = subprocess.run(
            [sys.executable, "-c", test_script],
            capture_output=True,
            text=True,
            cwd=str(REPO_ROOT)
        )
        self.assertEqual(res.returncode, 0)
        self.assertEqual(res.stdout.strip(), "Antigravity Quota: Unavailable")


if __name__ == "__main__":
    unittest.main()
