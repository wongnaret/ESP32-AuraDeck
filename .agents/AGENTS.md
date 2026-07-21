# ESP32-S3 AuraDeck Workspace Rules

Please adhere to the following Engineering Standards and Development Rules specified in `blueprint.md` at all times:

## Rule 1: Living Documentation First
* Every code change, feature addition, or API refactoring **MUST** be accompanied by an immediate update to the repository's documentation (e.g., `README.md`, `blueprint.md`, or `/docs`).
* Required Documentation Files:
  * `README.md`: Quickstart, hardware assembly, wiring, environment variables, and installation instructions for both Raspberry Pi and ESP32.
  * `blueprint.md`: Architecture specification, screen indexes, and JSON data schemas.
  * `API.md`: Detailed documentation for all local and third-party API endpoints, payloads, and OAuth refresh flows.

## Rule 2: Modular & Reusable Architecture
* No Monolithic Scripts: Code on both backend (Python) and frontend (ESP32 C++/LVGL) must be cleanly decoupled into modular, single-responsibility files.
* **Backend Modular Structure:**
  * Separated API services (e.g., `services/spotify.py`, `services/google.py`, `services/stocks.py`).
  * Dedicated data models/schemas.
  * Centralized configuration management (`config.py` using environment variables `.env`).
* **Frontend (ESP32) Modular Structure:**
  * Individual UI page modules (`ui/page_home.cpp`, `ui/page_antigravity.cpp`, etc.).
  * Shared reusable widgets (Header component, custom progress bars, text carousels).
  * Isolated hardware abstraction drivers (`drivers/shtc3.cpp`, `drivers/rtc_pcf85063.cpp`).

## Rule 3: Robust Error Handling & Graceful Degradation
* **Display Fallbacks:** If a specific third-party API fails (e.g., Spotify is offline or Stock API hits rate limit), the backend must return `null` or a fallback state for that object. The ESP32 UI **MUST NOT** crash or freeze; it should show a non-intrusive icon or "N/A" status in that specific widget zone.
* **Network Resilience:** The ESP32 screen must continue operating offline, rendering the Persistent Header (via RTC and SHTC3) and cached data, while displaying an offline indicator icon. Reconnection attempts should run asynchronously in the background.

## Rule 4: Security & Secrets Management
* **Zero Hardcoded Credentials:** Never place API keys, Wi-Fi credentials, or OAuth tokens directly in source code.
* **Storage:**
  * Raspberry Pi: Use `.env` files (loaded via `python-dotenv`) and strict `.gitignore` rules (tokens and local `.env` must never be committed).
  * ESP32: Store Wi-Fi and Server URLs in `NVS` (Non-Volatile Storage) or a `config.h` excluded from version control.

## Rule 5: Memory & Hardware Awareness (ESP32-S3)
* **LVGL Memory Management:** Dynamically clean up hidden screens or reuse persistent UI objects where possible to prevent heap fragmentation.
* **Polling Efficiency:** The ESP32 should fetch the `/api/v1/dashboard` endpoint at reasonable intervals (e.g., every 15–30 seconds) rather than aggressively spamming the backend, preserving battery and Wi-Fi resources.
