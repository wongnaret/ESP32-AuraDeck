# Blueprint: ESP32-S3 RLCD Smart Dashboard (Project Specification)

## 1. Project Overview & Architecture
This project is an ambient, low-power, reflective desk dashboard based on the **Waveshare ESP32-S3-RLCD-4.2** board. It uses a **Raspberry Pi** as the central backend server to process heavy logic, manage OAuth tokens, and aggregate third-party API data, serving a lightweight JSON payload to the ESP32.

### System Architecture

```

[ Third-Party Services ]
├── Antigravity API (5h & Weekly Credit Limits)
├── Stock API (Dynamic Watchlist configured on Server)
├── Google Tasks API (To-Do Items)
├── Google Calendar API (Upcoming Events)
├── Spotify Web API (Now Playing & Track Info)
└── Google APIs (GCP Status, GA4 Active Users, GSC Clicks/Impressions)
│
▼
┌───────────────────────────────────────────────────────────────┐
│                    Raspberry Pi (Backend)                     │
│  - Cron jobs & background fetching                            │
│  - Secure OAuth2 token management & data caching             │
│  - Serves lightweight JSON via REST Endpoint / MQTT           │
└───────────────────────────────┬───────────────────────────────┘
│ JSON Over Local Wi-Fi
▼
┌───────────────────────────────────────────────────────────────┐
│               ESP32-S3 RLCD Terminal (Frontend)               │
│  - LVGL Engine (Resolution: 300x400)                          │
│  - Reads SHTC3 Sensor (Temp/Humidity) & NTP/RTC Clock Sync   │
│  - Physical 'KEY' button cycles through Screens (0 → 6)       │
└───────────────────────────────────────────────────────────────┘

```

---

## 2. Hardware & Environment Specifications
* **Target Board:** Waveshare ESP32-S3-RLCD-4.2 (ESP32-S3-WROOM-1-N16R8)
* **Display:** 4.2-inch Reflective LCD (RLCD), 300x400 Resolution (Portrait Mode)
* **Onboard Hardware Utilized:**
  * `SHTC3`: Ambient Temperature & Humidity Sensor (I2C)
  * `PCF85063`: Hardware RTC with battery backup
  * `KEY Button`: Physical GPIO input for screen cycling
* **Graphics Framework:** LVGL v8.x+
* **Backend Stack:** Python (FastAPI / Flask) running on Raspberry Pi

---

## 3. UI Screen Specifications (Page 0 – Page 6)

### Persistent Header (Displayed across ALL pages)
* **Layout:** Top bar (~40px height)
* **Elements:**
  * Local Time (`HH:MM`) synced via NTP + RTC
  * Ambient Temp & Humidity (`SHTC3`): e.g., `26.5°C | 60%`
  * Wi-Fi Connection Status Indicator
  * Battery level indicator, Charging status (if any)

---

### Page 0: Home (Minimalist Summary Dashboard)
* **Purpose:** Single-glance overview combining the most critical metrics from all pages.
* **Layout Elements:**
  * **Clock/Date:** Prominent primary focus.
  * **Antigravity Quick Status:** Compact progress bar or percentage readout.
  * **Active Spotify Track:** One-line marquee text (`Artist - Track`).
  * **Next Event / Top Task:** Next upcoming calendar item or highest-priority task.
  * **Analytics Snapshot:** Quick stats (e.g., GA4 Active Users, Top Stock symbol).

---

### Page 1: Antigravity Credit Limit
* **Purpose:** Monitor AI usage credits and quota limits.
* **Layout Elements:**
  * **Progress Bar 1:** 5-Hour Credit Limit (Used vs. Total / Percentage).
  * **Progress Bar 2:** Weekly Credit Limit (Used vs. Total / Percentage).
  * **Reset Countdown:** Text showing time remaining until quota refresh.

---

### Page 2: Stock Watchlist
* **Purpose:** Track selected stocks and financial instruments.
* **Behavior:** Managed dynamically from the server side (add/remove stocks without updating ESP32 code).
* **Layout Elements:**
  * Vertical list or structured table of stock symbols.
  * Attributes per stock: `Symbol`, `Current Price`, `Change Amount`, `% Change` (Indicated with up/down arrows or clean text symbols).

---

### Page 3: To-Do List (Google Tasks)
* **Purpose:** Display personal action items.
* **Source:** Google Tasks API (fetched via Raspberry Pi).
* **Layout Elements:**
  * Clean vertical list of open task titles with checkboxes `[ ]`.
  * Support for pagination or scrolling if tasks exceed screen space.

---

### Page 4: Calendar
* **Purpose:** Overview of scheduled events and meetings.
* **Source:** Google Calendar API.
* **Display Mode:** Agenda / List view for upcoming days (preferred over month grid due to 300x400 size constraints).
* **Layout Elements:**
  * Grouped by Date (e.g., `Today`, `Tomorrow`).
  * Time range (`09:00 - 10:00`) and Event Title.

---

### Page 5: Spotify Now Playing
* **Purpose:** Media playback control & visualization.
* **Layout Elements:**
  * **Track Title & Artist Name:** Prominent, auto-scrolling if too long.
  * **Album Art Placeholder/Image:** Rendered if memory/resolution permits, or clean monochrome icon.
  * **Progress Bar:** Track elapsed time vs. total duration (`MM:SS / MM:SS`).
  * **Playback State:** Playing/Paused status.

---

### Page 6: DevOps & Web Analytics
* **Purpose:** System health and website performance metrics.
* **Layout Elements:**
  * **GCP Status:** System operational state indicator (`OK` / `DEGRADED` / `ISSUE`).
  * **Google Analytics 4 (GA4):** Real-time Active Users on site.
  * **Google Search Console (GSC):** Daily Clicks & Impressions summary.

---

## 4. Server-to-ESP32 API Data Interface
The Raspberry Pi exposes a single GET endpoint (e.g., `GET /api/v1/dashboard`) returning a structured JSON payload:

*Note:* This might be change it's just a design

```json
{
  "timestamp": 1784589784,
  "antigravity": {
    "limit_5h": { "used": 12.5, "total": 50.0, "percentage": 25.0 },
    "limit_weekly": { "used": 140.0, "total": 500.0, "percentage": 28.0 },
    "next_reset": "02h 15m"
  },
  "stocks": [
    { "symbol": "NVDA", "price": 125.40, "change_pct": 2.15 },
    { "symbol": "GOOGL", "price": 178.20, "change_pct": -0.85 }
  ],
  "todos": [
    { "id": "1", "title": "Review Pull Request #42", "completed": false },
    { "id": "2", "title": "Deploy Backend updates", "completed": false }
  ],
  "calendar": [
    { "time": "14:00", "title": "Architecture Sync", "is_today": true }
  ],
  "spotify": {
    "is_playing": true,
    "title": "Starboy",
    "artist": "The Weeknd",
    "progress_ms": 65000,
    "duration_ms": 230000
  },
  "analytics": {
    "gcp_status": "OK",
    "ga4_active_users": 34,
    "gsc_clicks": 1420,
    "gsc_impressions": 28500
  }
}

```

---

## 5. Engineering Standards & Development Rules

### Rule 1: Living Documentation First

* **Mandatory Documentation Update:** Every code change, feature addition, or API refactoring **MUST** be accompanied by an immediate update to the repository's documentation (e.g., `README.md`, `blueprint.md`, or `/docs`).
* **Required Documentation Files:**
* `README.md`: Quickstart, hardware assembly, wiring, environment variables, and installation instructions for both Raspberry Pi and ESP32.
* `blueprint.md`: Architecture specification, screen indexes, and JSON data schemas (this document).
* `API.md`: Detailed documentation for all local and third-party API endpoints, payloads, and OAuth refresh flows.



### Rule 2: Modular & Reusable Architecture

* **No Monolithic Scripts:** Code on both backend (Python) and frontend (ESP32 C++/LVGL) must be cleanly decoupled into modular, single-responsibility files.
* **Backend Modular Structure:**
* Separated API services (`services/spotify.py`, `services/google.py`, `services/stocks.py`).
* Dedicated data models/schemas (`models/dashboard.py`).
* Centralized configuration management (`config.py` using environment variables `.env`).


* **Frontend (ESP32) Modular Structure:**
* Individual UI page modules (`ui/page_home.cpp`, `ui/page_antigravity.cpp`, etc.).
* Shared reusable widgets (Header component, custom progress bars, text carousels).
* Isolated hardware abstraction drivers (`drivers/shtc3.cpp`, `drivers/rtc_pcf85063.cpp`).



### Rule 3: Robust Error Handling & Graceful Degradation

* **Display Fallbacks:** If a specific third-party API fails (e.g., Spotify is offline or Stock API hits rate limit), the backend must return `null` or a fallback state for that object. The ESP32 UI **MUST NOT** crash or freeze; it should show a non-intrusive icon or "N/A" status in that specific widget zone.
* **Network Resilience:** The ESP32 screen must continue operating offline, rendering the Persistent Header (via RTC and SHTC3) and cached data, while displaying an offline indicator icon. Reconnection attempts should run asynchronously in the background.

### Rule 4: Security & Secrets Management

* **Zero Hardcoded Credentials:** Never place API keys, Wi-Fi credentials, or OAuth tokens directly in source code.
* **Storage:**
* Raspberry Pi: Use `.env` files (loaded via `python-dotenv`) and strict `.gitignore` rules.
* ESP32: Store Wi-Fi and Server URLs in `NVS` (Non-Volatile Storage) or a `config.h` excluded from version control.



### Rule 5: Memory & Hardware Awareness (ESP32-S3)

* **LVGL Memory Management:** Dynamically clean up hidden screens or reuse persistent UI objects where possible to prevent heap fragmentation.
* **Polling Efficiency:** The ESP32 should fetch the `/api/v1/dashboard` endpoint at reasonable intervals (e.g., every 15–30 seconds) rather than aggressively spamming the backend, preserving battery and Wi-Fi resources.

---

## 6. Development Milestones & Roadmap

### Phase 1: Backend Infrastructure (Raspberry Pi)

* [ ] Set up Python environment with FastAPI/Flask.
* [ ] Implement Google OAuth2 authentication (Tasks, Calendar, GA4, GSC).
* [ ] Implement Spotify Web API client.
* [ ] Implement Antigravity API integration & Stock ticker fetching.
* [ ] Create endpoint `/api/v1/dashboard` with JSON caching (30s refresh interval).

### Phase 2: ESP32 Hardware & Drivers Initialization

* [ ] Set up Arduino IDE / PlatformIO with ESP32-S3 BSP.
* [ ] Configure RLCD driver (SPI/Parallel display controller).
* [ ] Initialize `SHTC3` sensor over I2C and read Temp/Humidity.
* [ ] Set up `PCF85063` RTC and NTP sync sequence.
* [ ] Configure GPIO interrupt for physical `KEY` button with debounce logic.

### Phase 3: LVGL UI Implementation

* [ ] Initialize LVGL v8 library for 300x400 resolution.
* [ ] Build static Persistent Header component.
* [ ] Implement State Machine for Page 0 through Page 6.
* [ ] Bind JSON response fields to respective LVGL widgets.
* [ ] Optimize render loop for smooth screen transitions.

```

```