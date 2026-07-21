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

### Page 2: Stock Watchlist (Multi-Asset Ticker)
* **Purpose:** Track selected equities, commodities, and digital assets.
* **Behavior:** Managed dynamically from the server side (add/remove assets without changing ESP32 client code).
* **Asset Support:**
  * **Thai Stocks:** Live SET indexes (e.g. `CPALL.BK`).
  * **Thai Gold Prices:** Live local gold bar spot prices (`GOLD_TH`).
  * **Cryptocurrencies:** Major global trading pairs (e.g. `BTC/THB`).
* **Layout Elements:**
  * High-contrast table with columns: `Symbol`, `Price`, and `% Change`.
  * Distinct trajectory characters (up/down arrows) representing price actions.

---

### Page 3: To-Do List (Google Tasks)
* **Purpose:** Display personal action items.
* **Source:** Google Tasks API (fetched via Raspberry Pi).
* **Layout Elements:**
  * Clean vertical list of open task titles with checkboxes `[ ]`.
  * Support for pagination or scrolling if tasks exceed screen space.

---

### Page 4: Calendar (Two-Part Split View)
* **Purpose:** High-level monthly overview paired with immediate chronological agendas.
* **Source:** Google Calendar API.
* **Layout Elements (Split Screen):**
  * **Top Section (Monthly Grid):** A visual compact month calendar displaying days of the current month. Days containing scheduled events are marked with bold high-contrast square outline pixels.
  * **Bottom Section (Agenda List):** A list showing chronological event agendas for **Today** and **Tomorrow** (e.g., `14:00 - Meeting`).

---

### Page 5: Spotify Now Playing
* **Purpose:** Media playback control & visualization.
* **Layout Elements:**
  * **Track Title & Artist Name:** Prominent, auto-scrolling if too long.
  * **Album Art Placeholder/Image:** Rendered if memory/resolution permits, or clean monochrome icon.
  * **Progress Bar:** Track elapsed time vs. total duration (`MM:SS / MM:SS`).
  * **Playback State:** Playing/Paused status.

---

### Page 6: DevOps, Analytics & GCP Billing
* **Purpose:** System health, web traffic metrics, and Google Cloud operational cost insights.
* **Layout Elements:**
  * **GCP Status:** System operational state indicator (`OK` / `DEGRADED` / `ISSUE`).
  * **Google Analytics 4 (GA4):** Real-time Active Users.
  * **Google Search Console (GSC):** Daily GSC search clicks & impressions count.
  * **GCP Multi-Project Billing Summary:** Displays month-to-date (MTD) accumulated spending metrics aggregated across multiple selected target projects and billing accounts.

---

## 4. Server-to-ESP32 API Data Interface
The Raspberry Pi exposes a single GET endpoint (e.g., `GET /api/v1/dashboard`) returning a structured JSON payload:

*Note:* This might be change it's just a design

```json
{
  "spotify": {
    "is_playing": true,
    "title": "เพลงรักในสายลม",
    "artist": "วงดนตรีสากล",
    "progress_ms": 128000,
    "duration_ms": 240000
  },
  "calendar": {
    "month_days_with_events": [1, 5, 12, 15, 20, 21, 22, 28],
    "events": [
      { "time": "14:00", "title": "ประชุมทีมสถาปัตยกรรม", "is_today": true },
      { "time": "Tomorrow 10:00", "title": "สแตนด์อัปรายวัน", "is_today": false }
    ]
  },
  "todos": [
    { "id": "1", "title": "ตรวจทาน Pull Request #42", "completed": false },
    { "id": "2", "title": "ติดตั้งโปรแกรมปรับปรุงระบบฐานข้อมูล", "completed": false }
  ],
  "stocks": [
    { "symbol": "CPALL", "price": 57.25, "change_pct": 1.33, "type": "TH_STOCK" },
    { "symbol": "BTC/THB", "price": 2350000.00, "change_pct": 2.15, "type": "CRYPTO" },
    { "symbol": "GOLD_TH", "price": 41200.00, "change_pct": -0.24, "type": "GOLD" }
  ],
  "antigravity": {
    "limit_5h": { "used": 12.5, "total": 50.0, "percentage": 25.0 },
    "limit_weekly": { "used": 140.0, "total": 500.0, "percentage": 28.0 },
    "next_reset": "02h 15m"
  },
  "analytics": {
    "gcp_status": "OK",
    "ga4_active_users": 34,
    "gsc_clicks": 1420,
    "gsc_impressions": 28500,
    "gcp_billing": [
      { "project_name": "AuraDeck Dev", "cost_mtd": 12.50, "currency": "USD" },
      { "project_name": "Client Prod", "cost_mtd": 148.20, "currency": "USD" }
    ]
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

---

## 7. Special Graphics & Thai Character Handling (แก้ปัญหาสระลอย)

### The Stacking Vowel Challenge
Standard LVGL v8 lacks a complex text-shaping engine (like HarfBuzz). When rendering Thai UTF-8 scripts, combining characters—including upper/lower vowels (e.g., สระอุ, สระอู, สระอี) and tone marks (e.g., ไม้เอก, ไม้โท)—collide vertically, causing "สระลอย" (floating/overlapping glyphs).

### Reshaping and Font PUA Mapping Solution
To fix this, the ESP32-S3 client utilizes a custom **C++ ThaiReshaper** library. 
1.  **UTF-8 to Unicode Conversion:** The reshaping engine intercepts Thai string payloads and decodes UTF-8 byte streams into 16-bit Unicode characters.
2.  **Glyph Rules Examination:** It inspects adjacent character types. If an upper vowel is followed by a tone mark, or if characters clash with tall-consonants (like ป, ฝ, ฟ), the engine swaps the standard Unicode code points with pre-compiled **Private Use Area (PUA) codes (`0xF700` to `0xF71F`)**.
3.  **Special Thai Font Asset:** These PUA code points map to specially shifted and vertically adjusted glyph versions baked into our legacy Thai font files, enabling pixel-perfect, overlapping-free multi-layer vowel stacking directly on the 1-bit screen.


```

```