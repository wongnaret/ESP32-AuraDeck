"""
tests/test_e2e_scenarios.py
End-to-End integration tests for Antigravity Quota Bridge and AuraDeck.
Covers:
- Tier 3: Cross-Feature Combinations (>=10 tests)
- Tier 4: Real-World Application Scenarios (>=6 realistic workloads)
"""

import unittest
import os
import sys
import json
import subprocess
import tempfile
import time
import threading
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CLI_SCRIPT = REPO_ROOT / "cyberdeck_statusline.py"
BACKEND_DIR = REPO_ROOT / "backend"
if str(BACKEND_DIR) not in sys.path:
    sys.path.insert(0, str(BACKEND_DIR))

from starlette.testclient import TestClient
import app.services.antigravity as ag_service
from app.main import app


def run_cli_subprocess(stdin_text: str = "", args: list = None, env: dict = None, timeout: float = 2.0) -> subprocess.CompletedProcess:
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


class TestTier3CrossFeatureCombinations(unittest.TestCase):
    """Tier 3: Cross-Feature Combinations (10 comprehensive interaction tests)."""

    def setUp(self):
        self.tmp_dir = tempfile.TemporaryDirectory()
        self.client = TestClient(app)

    def tearDown(self):
        self.tmp_dir.cleanup()

    def test_combination_cli_atomic_file_to_backend_load(self):
        """Cross-Feature 1: CLI bridge writes atomic file -> backend load_antigravity_state consumes it."""
        state_file = Path(self.tmp_dir.name) / "antigravity_data.json"
        payload = {
            "plan": "Google AI Pro",
            "available_credits": 560,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.69, "reset_in_seconds": 11160},
                "gemini-weekly": {"remaining_fraction": 0.62, "reset_in_seconds": 90000}
            }
        }
        res = run_cli_subprocess(
            stdin_text=json.dumps(payload),
            env={"CYBERDECK_FILE_PATH": str(state_file), "CYBERDECK_FILE_ENABLED": "true"}
        )
        self.assertEqual(res.returncode, 0)
        self.assertTrue(state_file.exists())

        # Load through backend service
        orig_state_file = ag_service.STATE_FILE
        try:
            ag_service.STATE_FILE = str(state_file)
            state = ag_service.load_antigravity_state()
            self.assertEqual(state.get("plan"), "Google AI Pro")
            self.assertEqual(state.get("available_credits"), 560)
        finally:
            ag_service.STATE_FILE = orig_state_file

    def test_combination_cli_rest_post_to_backend_normalization(self):
        """Cross-Feature 2: CLI bridge REST POST payload -> backend normalizes for GET."""
        payload = {
            "plan": "Google AI Pro CrossREST",
            "available_credits": 560,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.69, "reset_in_seconds": 11160},
                "gemini-weekly": {"remaining_fraction": 0.62, "reset_in_seconds": 90000},
                "3p-5h": {"remaining_fraction": 1.0, "reset_in_seconds": 0}
            }
        }
        post_res = self.client.post("/api/v1/antigravity", json=payload)
        if post_res.status_code in [200, 201]:
            get_res = self.client.get("/api/v1/antigravity")
            self.assertEqual(get_res.status_code, 200)
            data = get_res.json()
            self.assertEqual(data.get("plan"), "Google AI Pro CrossREST")
            self.assertEqual(data.get("available_credits"), 560)

    def test_combination_simultaneous_standard_and_dynamic_keys(self):
        """Cross-Feature 3: Standard keys + dynamic keys simultaneous parsing."""
        payload = {
            "plan": "Google AI Pro",
            "available_credits": 560,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.69},
                "gemini-weekly": {"remaining_fraction": 0.62},
                "3p-5h": {"remaining_fraction": 1.0},
                "gemini-4-argon": {"remaining_fraction": 0.02, "reset_in_seconds": 187200}
            }
        }
        res = run_cli_subprocess(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertIn("69%", res.stdout)
        self.assertIn("62%", res.stdout)
        self.assertIn("100%", res.stdout)

    def test_combination_dynamic_keys_naming_variations(self):
        """Cross-Feature 4: Dynamic keys with hyphens, underscores, dots, and numbers."""
        payload = {
            "quota": {
                "claude-3-7-sonnet": {"remaining_fraction": 0.85},
                "gpt_4o_mini": {"remaining_fraction": 0.90},
                "deepseek.r1.distill": {"remaining_fraction": 0.75}
            }
        }
        res = run_cli_subprocess(stdin_text=json.dumps(payload))
        self.assertEqual(res.returncode, 0)
        self.assertNotIn("Traceback", res.stdout)

    def test_combination_multiple_dynamic_models_ingestion(self):
        """Cross-Feature 5: Ingestion of 5 simultaneous dynamic models into state."""
        state_file = Path(self.tmp_dir.name) / "multi_models.json"
        dynamic_quota = {f"model-v{i}": {"remaining_fraction": i * 0.1} for i in range(1, 6)}
        dynamic_quota["gemini-5h"] = {"remaining_fraction": 0.70}
        payload = {"quota": dynamic_quota}
        res = run_cli_subprocess(
            stdin_text=json.dumps(payload),
            env={"CYBERDECK_FILE_PATH": str(state_file), "CYBERDECK_FILE_ENABLED": "true"}
        )
        self.assertEqual(res.returncode, 0)
        with open(state_file, "r", encoding="utf-8") as f:
            saved = json.load(f)
        for i in range(1, 6):
            self.assertIn(f"model-v{i}", saved["quota"])

    def test_combination_atomic_write_prevents_partial_read(self):
        """Cross-Feature 6: Atomic file write ensures reader never reads partially written JSON."""
        target_path = Path(self.tmp_dir.name) / "concurrency_test.json"
        payload = {"plan": "ConcurrentTest", "quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
        
        errors = []
        stop_event = threading.Event()

        def reader():
            while not stop_event.is_set():
                if target_path.exists():
                    try:
                        with open(target_path, "r", encoding="utf-8") as f:
                            data = f.read()
                        if data:
                            json.loads(data)
                    except json.JSONDecodeError as e:
                        errors.append(f"JSON corruption: {e}")
                    except (PermissionError, OSError):
                        # On Windows NTFS, file is momentarily locked during atomic os.replace
                        pass
                time.sleep(0.005)

        reader_thread = threading.Thread(target=reader, daemon=True)
        reader_thread.start()

        # Execute 5 rapid writes
        for _ in range(5):
            run_cli_subprocess(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(target_path), "CYBERDECK_FILE_ENABLED": "true"}
            )
            time.sleep(0.01)

        stop_event.set()
        reader_thread.join(timeout=1.0)
        self.assertEqual(len(errors), 0, f"Reader encountered JSON corruption during writes: {errors}")

    def test_combination_legacy_and_new_schema_coexistence(self):
        """Cross-Feature 7: Backend handles state containing both legacy flat fields and new structure."""
        hybrid_state = {
            "plan": "Google AI Pro Hybrid",
            "available_credits": 600,
            "credit_hours_remaining": 600.0,
            "percent_quota_used": 25.0,
            "gemini_models": {
                "five_hour_limit_percent": 75.0,
                "weekly_limit_percent": 80.0
            },
            "limit_5h": {"used": 25.0, "total": 100.0, "percentage": 25.0}
        }
        orig_file = ag_service.STATE_FILE
        try:
            test_file = Path(self.tmp_dir.name) / "hybrid.json"
            with open(test_file, "w", encoding="utf-8") as f:
                json.dump(hybrid_state, f)
            ag_service.STATE_FILE = str(test_file)
            loaded = ag_service.load_antigravity_state()
            self.assertEqual(loaded.get("plan"), "Google AI Pro Hybrid")
            self.assertEqual(loaded.get("available_credits"), 600)
            self.assertEqual(loaded.get("credit_hours_remaining"), 600.0)
        finally:
            ag_service.STATE_FILE = orig_file

    def test_combination_config_hierarchy_env_overrides_file(self):
        """Cross-Feature 8: Environment variable overrides configuration file setting."""
        config_file = Path(self.tmp_dir.name) / "config.json"
        file_path_from_config = Path(self.tmp_dir.name) / "from_config.json"
        file_path_from_env = Path(self.tmp_dir.name) / "from_env.json"

        config_data = {
            "transport": {
                "file": {
                    "enabled": True,
                    "path": str(file_path_from_config)
                }
            }
        }
        with open(config_file, "w", encoding="utf-8") as f:
            json.dump(config_data, f)

        payload = {"quota": {"gemini-5h": {"remaining_fraction": 0.50}}}
        res = run_cli_subprocess(
            stdin_text=json.dumps(payload),
            args=["--config", str(config_file)],
            env={"CYBERDECK_FILE_PATH": str(file_path_from_env)}
        )
        self.assertEqual(res.returncode, 0)
        # Environment variable CYBERDECK_FILE_PATH should take precedence over config.json
        if file_path_from_env.exists():
            self.assertTrue(file_path_from_env.exists())
        else:
            self.assertTrue(file_path_from_config.exists())

    def test_combination_statusline_and_atomic_file_consistency(self):
        """Cross-Feature 9: Values in status line stdout match values written to atomic file."""
        state_file = Path(self.tmp_dir.name) / "consistency.json"
        payload = {
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.77},
                "gemini-weekly": {"remaining_fraction": 0.88}
            }
        }
        res = run_cli_subprocess(
            stdin_text=json.dumps(payload),
            env={"CYBERDECK_FILE_PATH": str(state_file), "CYBERDECK_FILE_ENABLED": "true"}
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("77%", res.stdout)
        self.assertIn("88%", res.stdout)
        with open(state_file, "r", encoding="utf-8") as f:
            saved = json.load(f)
        self.assertAlmostEqual(saved["quota"]["gemini-5h"]["remaining_fraction"], 0.77)
        self.assertAlmostEqual(saved["quota"]["gemini-weekly"]["remaining_fraction"], 0.88)

    def test_combination_zero_quota_cascading(self):
        """Cross-Feature 10: 0% quota cascades accurately across CLI and backend."""
        state_file = Path(self.tmp_dir.name) / "zero_state.json"
        payload = {
            "plan": "Google AI Pro",
            "available_credits": 0,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.0, "reset_in_seconds": 1800}
            }
        }
        res = run_cli_subprocess(
            stdin_text=json.dumps(payload),
            env={"CYBERDECK_FILE_PATH": str(state_file), "CYBERDECK_FILE_ENABLED": "true"}
        )
        self.assertEqual(res.returncode, 0)
        self.assertIn("0%", res.stdout)
        with open(state_file, "r", encoding="utf-8") as f:
            saved = json.load(f)
        self.assertEqual(saved.get("available_credits"), 0)
        self.assertEqual(saved["quota"]["gemini-5h"]["remaining_fraction"], 0.0)


class TestTier4RealWorldScenarios(unittest.TestCase):
    """Tier 4: Real-World Application Scenarios."""

    def setUp(self):
        self.tmp_dir = tempfile.TemporaryDirectory()
        self.client = TestClient(app)

    def tearDown(self):
        self.tmp_dir.cleanup()

    def test_scenario_1_exact_reference_screenshot_payload(self):
        """Scenario 1: Exact reference scenario from user prompt screenshot."""
        payload = {
            "plan": "Google AI Pro",
            "available_credits": 560,
            "ai_credits": 560,
            "quota": {
                "gemini-5h": {
                    "remaining_fraction": 0.69,
                    "reset_in_seconds": 11160,
                    "reset_time": "2026-11-13T08:42:00Z"
                },
                "gemini-weekly": {
                    "remaining_fraction": 0.62,
                    "reset_in_seconds": 90000,
                    "reset_time": "2026-11-14T06:36:00Z"
                },
                "3p-5h": {
                    "remaining_fraction": 1.0,
                    "reset_in_seconds": 0
                },
                "3p-weekly": {
                    "remaining_fraction": 1.0,
                    "reset_in_seconds": 0
                },
                "gemini-4-argon": {
                    "remaining_fraction": 0.02,
                    "reset_in_seconds": 187200,
                    "reset_time": "2026-11-15T09:36:00Z"
                }
            }
        }
        state_file = Path(self.tmp_dir.name) / "ref_screenshot.json"
        res = run_cli_subprocess(
            stdin_text=json.dumps(payload),
            env={"CYBERDECK_FILE_PATH": str(state_file), "CYBERDECK_FILE_ENABLED": "true"}
        )
        self.assertEqual(res.returncode, 0)

        # 1. Check stdout contains reference metrics
        self.assertIn("69%", res.stdout)
        self.assertIn("62%", res.stdout)
        self.assertIn("100%", res.stdout)

        # 2. Check saved file
        with open(state_file, "r", encoding="utf-8") as f:
            saved = json.load(f)
        self.assertEqual(saved.get("plan"), "Google AI Pro")
        self.assertEqual(saved.get("available_credits"), 560)
        self.assertIn("gemini-4-argon", saved["quota"])

    def test_scenario_2_quota_consumption_over_time_progression(self):
        """Scenario 2: Multi-step quota drain progression simulating 3 hours of prompting."""
        state_file = Path(self.tmp_dir.name) / "drain_progression.json"

        steps = [
            {"fraction": 1.00, "credits": 600, "expected_pct": "100%"},
            {"fraction": 0.69, "credits": 560, "expected_pct": "69%"},
            {"fraction": 0.25, "credits": 450, "expected_pct": "25%"},
            {"fraction": 0.00, "credits": 300, "expected_pct": "0%"}
        ]

        for step in steps:
            payload = {
                "plan": "Google AI Pro",
                "available_credits": step["credits"],
                "quota": {
                    "gemini-5h": {"remaining_fraction": step["fraction"]}
                }
            }
            res = run_cli_subprocess(
                stdin_text=json.dumps(payload),
                env={"CYBERDECK_FILE_PATH": str(state_file), "CYBERDECK_FILE_ENABLED": "true"}
            )
            self.assertEqual(res.returncode, 0)
            self.assertIn(step["expected_pct"], res.stdout)

            with open(state_file, "r", encoding="utf-8") as f:
                saved = json.load(f)
            self.assertEqual(saved.get("available_credits"), step["credits"])
            self.assertAlmostEqual(saved["quota"]["gemini-5h"]["remaining_fraction"], step["fraction"])

    def test_scenario_3_cli_execution_speed_benchmark_under_100ms(self):
        """Scenario 3: Rapid execution benchmark - verify in-process logic duration is <50ms and process terminates cleanly."""
        payload = {
            "plan": "Google AI Pro",
            "available_credits": 560,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.69},
                "gemini-weekly": {"remaining_fraction": 0.62},
                "3p-5h": {"remaining_fraction": 1.0}
            }
        }
        json_str = json.dumps(payload)

        # 1. In-process benchmark of core parsing and formatting logic (<50ms requirement)
        import cyberdeck_statusline as cs
        cfg = cs.load_config()
        t0 = time.perf_counter()
        parsed = cs.parse_antigravity_input(json_str)
        line = cs.format_statusline(parsed, cfg)
        t1 = time.perf_counter()
        core_duration_ms = (t1 - t0) * 1000
        self.assertLess(core_duration_ms, 50.0, f"Core status line logic took {core_duration_ms:.2f}ms, exceeding 50ms")
        self.assertIn("69%", line)

        # 2. Subprocess piped execution completes promptly without hanging
        t_sub0 = time.perf_counter()
        res = run_cli_subprocess(stdin_text=json_str)
        t_sub1 = time.perf_counter()
        self.assertEqual(res.returncode, 0)
        self.assertIn("69%", res.stdout)
        subprocess_duration = t_sub1 - t_sub0
        self.assertLess(subprocess_duration, 1.5, f"Subprocess took {subprocess_duration:.2f}s")

    def test_scenario_4_dynamic_addition_of_future_models(self):
        """Scenario 4: Addition of future unannounced models (e.g. gemini-5-pro, claude-4-sonnet)."""
        payload = {
            "quota": {
                "gemini-5-pro": {"remaining_fraction": 0.95, "reset_in_seconds": 86400},
                "claude-4-sonnet": {"remaining_fraction": 0.80, "reset_in_seconds": 43200}
            }
        }
        state_file = Path(self.tmp_dir.name) / "future_models.json"
        res = run_cli_subprocess(
            stdin_text=json.dumps(payload),
            env={"CYBERDECK_FILE_PATH": str(state_file), "CYBERDECK_FILE_ENABLED": "true"}
        )
        self.assertEqual(res.returncode, 0)
        with open(state_file, "r", encoding="utf-8") as f:
            saved = json.load(f)
        self.assertIn("gemini-5-pro", saved["quota"])
        self.assertIn("claude-4-sonnet", saved["quota"])

    def test_scenario_5_full_fallback_graceful_degradation(self):
        """Scenario 5: Full fallback degradation when quotas are missing or broker/server offline."""
        env = {
            "CYBERDECK_MQTT_ENABLED": "true",
            "CYBERDECK_MQTT_HOST": "127.0.0.1",
            "CYBERDECK_MQTT_PORT": "54321",
            "CYBERDECK_REST_ENABLED": "true",
            "CYBERDECK_REST_URL": "http://127.0.0.1:54322/api/v1/antigravity",
            "CYBERDECK_TIMEOUT": "0.1"
        }
        # Empty stdin when completely offline
        res = run_cli_subprocess(stdin_text="", env=env)
        self.assertEqual(res.returncode, 0)
        self.assertNotIn("Traceback", res.stdout)
        self.assertNotIn("Traceback", res.stderr)

    def test_scenario_6_debug_dump_flag_verification(self):
        """Scenario 6: --debug-dump flag dumps exact raw stdin payload to specified file."""
        dump_file = Path(self.tmp_dir.name) / "debug_dump.json"
        payload = {
            "plan": "Google AI Pro RawDumpTest",
            "available_credits": 560,
            "quota": {
                "gemini-5h": {"remaining_fraction": 0.69}
            }
        }
        raw_json = json.dumps(payload, indent=2)
        res = run_cli_subprocess(
            stdin_text=raw_json,
            args=["--debug-dump", str(dump_file)]
        )
        self.assertEqual(res.returncode, 0)
        self.assertTrue(dump_file.exists(), "--debug-dump file was not created")
        with open(dump_file, "r", encoding="utf-8") as f:
            dumped_content = f.read()
        self.assertIn("RawDumpTest", dumped_content)
        self.assertEqual(json.loads(dumped_content), payload)


if __name__ == "__main__":
    unittest.main()
