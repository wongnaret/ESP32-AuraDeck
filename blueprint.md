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
│  - FastAPI server & Mosquitto MQTT Broker                     │
│  - Custom background scrapers (Google, Spotify, Stocks, etc.) │
│  - Configures secure Local Wi-Fi Access Point (10.42.0.1)     │
└───────────────────────────────┬───────────────────────────────┘
│ JSON Over Local Wi-Fi / MQTT (Port 1883)
▼
┌───────────────────────────────────────────────────────────────┐
│               ESP32-S3 RLCD Terminal (Frontend)               │
│  - LVGL Engine (Resolution: 400x300 - Landscape Mode)         │
│  - Reads SHTC3 Sensor (Temp/Humidity) & NTP/RTC Clock Sync   │
│  - Physical 'KEY' button (GPIO18) cycles Screens (0 → 6)      │
└───────────────────────────────────────────────────────────────┘

```

---

## 2. Hardware & Environment Specifications
* **Target Board:** Waveshare ESP32-S3-RLCD-4.2 (ESP32-S3-WROOM-1-N16R8 with 16MB Flash and 8MB Octal PSRAM)
* **Display:** 4.2-inch Reflective LCD (RLCD), 400x300 Resolution (Landscape Mode)
* **Onboard Pins & Wire Configurations:**
  * **ST7305 RLCD Screen (SPI Interface):**
    * `PIN_LCD_SCL`   = GPIO11 (SPI Clock)
    * `PIN_LCD_SDA`   = GPIO12 (SPI MOSI)
    * `PIN_LCD_CS`    = GPIO40 (Chip Select)
    * `PIN_LCD_RS`    = GPIO5  (Register/Data Select)
    * `PIN_LCD_RESET` = GPIO41 (Hardware Reset)
  * **Onboard Sensors (Shared I2C Interface):**
    * `PIN_I2C_SDA`   = GPIO13
    * `PIN_I2C_SCL`   = GPIO14
  * **Touch Panel Bus Controller:**
    * `PIN_TP_INT`    = GPIO7
    * `PIN_TP_RESET`  = GPIO42 (Must be held HIGH to prevent a floating chip state from locking the shared I2C bus low!)
  * **Side Navigation Button:**
    * `PIN_BUTTON`    = GPIO18 (Hardware interrupt with 150ms debounce suppression)
  * **Battery ADC Power Sense:**
    * `PIN_BAT_ADC`   = GPIO4 (Reads battery voltage via onboard voltage divider, configurable ratio `BAT_ADC_DIVIDER` in `config.h`)
* **Physical Hardware Modules Utilized:**
  * `ST7305`: Display controller with custom precomputed O(1) pixel LUT transpositions for landscape rendering.
  * `SHTC3`: Ambient Temperature & Humidity Sensor polled via raw non-clock-stretching TwoWire.
  * `PCF85063`: Hardware RTC for bulletproof local time-keeping during network disconnections.
* **Bus Stability & Resilience Configurations:**
  * **I2C Bus Optimization:** Running at standard 100kHz clock speed for maximum hardware signal integrity.
  * **Active Bus Self-Healing:** Automatic bus recovery routine that programmatically toggles SCL 16 times if a slave locks SDA, issues an I2C STOP, and restarts the TwoWire driver dynamically, ensuring 100% telemetry uptime.
* **On-Screen Startup Diagnostics:** Early boot visual splash page showing live startup milestone checklists and a progress bar directly on-screen before initiating backend MQTT/NTP sessions.
* **Graphics Framework:** LVGL v8.3.x (1-bit monochrome, custom PSRAM frame-buffer allocation)
* **Backend Stack:** Docker Containers (FastAPI & Eclipse Mosquitto MQTT Broker) running on Raspberry Pi.

---

## 3. UI Screen Specifications (Page 0 – Page 6)

### Persistent Header (Displayed across ALL pages)
* **Layout:** Top status bar (0 to 25px vertical slice, partitioned from view screen)
* **Elements:**
  * Page Title Name (Left aligned, dynamically reflects the current screen context)
  * Wi-Fi Connection Icon (Left aligned: `📶` when online, `⚠️` when offline)
  * MQTT Broker Dot Indicator (Left aligned: green dot when connected, red dot when disconnected)
  * Battery Status & Percentage (Right aligned: `🔌 USB` / `⚡ XX%` / `🔋 XX%` with tier icons)
  * SHTC3 Live Temperature (Right aligned: e.g. `26.5°C`)
  * Local Time (`HH:MM`) synced via NTP + RTC (Right aligned)
  * Bottom divider line rule (1px high-contrast stroke at y=25px)

---

### Page 0: Home (Minimalist Summary Dashboard & Rain Forecast)
* **Purpose:** Single-glance overview combining digital clock, dual-language dates, indoor environmental telemetry, outdoor weather, and 6-hour rain forecast.
* **Layout Structure (3-Tier Layout):**
  * **Tier 1 (Top):**
    * **Digital Clock:** Large bold digital time (`12:00`, `lv_font_montserrat_32`) aligned top-left.
    * **English Date:** `dayofweek, day month year` (`Thursday, 20 August 2026`, `lv_font_montserrat_16`).
    * **Thai Date:** `วัน...ที่ ... ... พ.ศ. ....` (`วันพฤหัสบดีที่ 20 สิงหาคม 2569`, `lv_font_prompt_16` with `ThaiReshaper`).
  * **Tier 2 (Middle):**
    * **Indoor SHTC3 Card:** Displays live onboard ambient temperature and humidity (`26.5 °C | 55 %`).
    * **Outdoor Weather Card:** Displays live outdoor temperature and sky condition (`28.5 °C • มีเมฆมาก`).
  * **Tier 3 (Bottom):**
    * **6-Hour Hourly Rain Forecast Grid (370x144):** 6 horizontal columns displaying time (`18:00`), weather condition (`Rain`/`Cloud`), rain probability percentage (`80%`), and expected temperature (`28°`).

---

### Page 1: Antigravity Credit Limit
* **Purpose:** Monitor AI usage credits and quota limits.
* **Layout Elements:**
  * **Remaining Hours:** Prominent digital countdown (e.g. `4.5 Hrs Left` with ~32px font size).
  * **Linear Progress Bar:** Custom LVGL monochrome bar showing used vs. total weekly quota.
  * **Quota Label:** Center-aligned percentage text (e.g., `Quota Used: 72%`).
  * **Footer Info:** API quota reset period notification.

---

### Page 2: Stock Watchlist (Multi-Asset Ticker)
* **Purpose:** Track selected equities, commodities, and digital assets.
* **Asset Support:**
  * **SET50 Index:** Thai stock market index (points and % change).
  * **CPALL.BK:** Local Stock quote with US Dollar / local currency conversion.
  * **GOLD:** Local Thai gold bar spot price (expressed in local Thai Baht `฿`).
  * **BTC-THB:** Real-time crypto conversion rate.
* **Layout Elements:**
  * Clean, non-overlapping multi-row list displaying: `Symbol`, `Price`, and `% Change`.
  * Distinct trajectory direction glyphs (`▲ +` / `▼ -`) representing market momentum.

---

### Page 3: To-Do List (Google Tasks)
* **Purpose:** Display personal action items.
* **Source:** Google Tasks API (fetched via Raspberry Pi).
* **Layout Elements:**
  * Up to 4 vertical checklist items with standard `[ ]` markers.
  * Full support for Thai Unicode characters via the custom C++ ThaiReshaper.
  * Built-in `LV_LABEL_LONG_DOT` auto-truncation for longer task titles to prevent boundary overlaps.
* **Multi-List Support:** When multiple Google Task lists are configured in the profile, tasks from each list are prefixed with `[List Name]` in the title (e.g., `[ ] [Work] ตรวจทาน PR #42`) for contextual visibility.

---

### Page 4: Calendar (Chronological Agendas)
* **Purpose:** Chronicle immediate Google Calendar events.
* **Source:** Google Calendar API.
* **Layout Elements:**
  * Clean multi-line schedule list showcasing the next 3 chronological agendas.
  * Displays: `[Start Time] Event Title (Today/Tomorrow)`.
  * Integrated C++ ThaiReshaper and auto-ellipsis truncation protecting layout margins.

---

### Page 5: Spotify Now Playing & Live Synced Lyrics
* **Purpose:** Real-time media progress, album artwork, track status, and time-synced lyrics display.
* **Layout Elements:**
  * **Track Title:** Primary bold 24px label (`Prompt 24`) with smooth circular scrolling marquee.
  * **Artist Name:** Sub-title artist metadata (`Prompt 16`) with smooth circular scrolling marquee.
  * **Album Art Container:** 80x80 container rendering 1-bit Floyd-Steinberg dithered album cover.
  * **Linear Progress Bar:** Full playback visualizer showing elapsed vs total duration.
  * **Elapsed Timestamps:** Left and right text readouts (e.g. `1:23` / `3:45`).
  * **Time-Synced Lyrics Container (364x82):** Positioned directly beneath the progress bar, displaying 2 dynamic lines:
    * **Current Lyric Line:** Bold, prominent 16px prompt font (`lv_font_prompt_16`) with ThaiReshaper support.
    * **Next Upcoming Lyric Line:** Secondary 16px prompt font (`lv_font_prompt_16`) previewing the next verse.
    * **Fallback Metadata:** Shows album name (`Album: ...`) and artist/composer info if track has no lyrics.
  * **Graceful Degradation:** Displays a clean "Spotify Offline / Idle" layout when inactive.

---

### Page 6: Google Analytics 4 (GA4 Dedicated Real-Time & 28D Analytics)
* **Purpose:** Granular web traffic telemetry, real-time visitors, engagement duration, and regional breakdown.
* **Layout Structure (Dual-Zone):**
  * **Left Zone (230px):**
    * **Real-time Counter (30m):** Eye-catching active visitor counter (`34` in bold `48px` Montserrat font) with subtitle `Real-time users now`.
    * **2x2 Metrics Grid:**
      * `28D Active Users` (e.g. `14.2K`)
      * `New Users` (e.g. `1.8K`)
      * `Avg Engagement Time` (e.g. `2m 15s`)
      * `Event Count` (e.g. `92.4K`)
  * **Right Zone (138px):**
    * **Top Cities Panel:** List of top 5 active visitor cities with counts (e.g. `1. Bangkok (18)`, `2. Chiang Mai (6)`, etc.) supporting ThaiReshaper.

---

### Page 7: GCP Multi-Project Cloud Billing & Daily Forecast
* **Purpose:** Multi-tenant Google Cloud spend tracker, month-end forecast, top service cost breakdown, and daily spend bar chart.
* **Navigation:** Supports multiple configured GCP projects with auto-cycling every 15s or manual switching.
* **Layout Structure (Dual-Zone):**
  * **Top Status Header:** Project Title and pagination index (`GCP [1/2]: AuraDeck Prod`) with click-to-switch indicator.
  * **Left Zone (180px):**
    * **Month-to-Date Spend:** Prominent MTD amount with native project currency (`฿14,250 MTD` or `$12.50 MTD`).
    * **Month-End Forecast:** Estimated total spend by end of month (`Est. End: ฿18,500`).
    * **Top 4 Services Breakdown:** Percentage and cost per service (e.g. `1. Compute Engine: 45%`, `2. BigQuery & AI: 25%`, etc.).
  * **Right Zone (188px):**
    * **10-Day Daily Spend Bar Chart:** High-contrast 10-bar chart with daily date labels (`11` to `20`) and dynamic Max scaling marker.

---

## 4. Server-to-ESP32 MQTT Payload Schema
The Raspberry Pi publishes structured, single-responsibility telemetry objects on specific MQTT topics. Below are the implemented JSON schemas:

> **Topic Normalization:** The ESP32 `network_manager.cpp` normalizes both generic topics (`auradeck/{service}`) and device-specific topics (`auradeck/device/{mac}/{service}`) into the same generic form before dispatching to UI pages. This means UI page modules always receive payloads via the generic topic format regardless of pairing state.

> **Data Cache Layer:** `ui_manager.cpp` maintains a per-service `DynamicJsonDocument` cache (`m_dataCache[]`). Every incoming MQTT payload is deep-copied into the cache before being forwarded to page `update_*()` functions. When the user navigates to a page via `showPage()`, the cached data is immediately replayed via `replayCachedData()` — ensuring pages always display the most recent backend data, even if it arrived while the user was viewing a different page.

### Topic: `auradeck/ga4`
```json
{
  "active_users_30m": 34,
  "active_28d_users": "14.2K",
  "new_users": "1.8K",
  "avg_engagement_time": "2m 15s",
  "event_count": "92.4K",
  "top_cities": [
    { "city": "Bangkok", "active_users": 18 },
    { "city": "Chiang Mai", "active_users": 6 },
    { "city": "Nonthaburi", "active_users": 4 },
    { "city": "Phuket", "active_users": 3 },
    { "city": "Chon Buri", "active_users": 2 }
  ]
}
```
> **ESP32 Handling:** `page_ga4.cpp` populates the 48px counter, 2x2 grid, and the right-hand active cities list with ThaiReshaper support.

### Topic: `auradeck/gcp`
```json
{
  "total_projects": 2,
  "projects": [
    {
      "project_id": "auradeck-prod",
      "project_name": "AuraDeck Prod",
      "currency": "THB",
      "cost_mtd": 14250.0,
      "forecast_end_of_month": 18500.0,
      "service_breakdown": [
        { "service": "Compute Engine", "cost": 6412.5, "pct": 45 },
        { "service": "BigQuery & AI", "cost": 3562.5, "pct": 25 },
        { "service": "Cloud Run / GKE", "cost": 2850.0, "pct": 20 },
        { "service": "Cloud Storage", "cost": 1425.0, "pct": 10 }
      ],
      "daily_costs": [
        { "date": "11/08", "cost": 420.0 },
        { "date": "12/08", "cost": 450.0 },
        { "date": "13/08", "cost": 480.0 },
        { "date": "14/08", "cost": 510.0 },
        { "date": "15/08", "cost": 490.0 },
        { "date": "16/08", "cost": 460.0 },
        { "date": "17/08", "cost": 530.0 },
        { "date": "18/08", "cost": 550.0 },
        { "date": "19/08", "cost": 520.0 },
        { "date": "20/08", "cost": 540.0 }
      ]
    }
  ]
}
```
> **ESP32 Handling:** `page_gcp.cpp` renders the active project data, renders the 10-bar daily spend chart, and supports project switching via `cycle_gcp_project()`.

### Topic: `auradeck/spotify`
```json
{
  "is_playing": true,
  "title": "เพลงรักในสายลม",
  "artist": "วงดนตรีสากล",
  "album": "บทเพลงแห่งความคิดถึง",
  "progress_ms": 128000,
  "duration_ms": 240000,
  "has_lyrics": true,
  "current_lyric": "อยากบอกให้เธอรู้ ว่าฉันรักเธอเท่าไหร่...",
  "next_lyric": "แม้เวลาจะหมุนเวียนเปลี่ยนไปนานแค่ไหน..."
}
```
> **ESP32 Handling:** `page_spotify.cpp` reads `title`, `artist`, `current_lyric`, and `next_lyric`, passes them through `ThaiReshaper::reshape()`, and renders the active + upcoming lyrics lines with local 1-second progress bar animation ticks.

### Topic: `auradeck/calendar`
```json
{
  "month_days_with_events": [1, 5, 12, 15, 20, 21, 22, 28],
  "events": [
    { "time": "14:00", "title": "ประชุมทีมสถาปัตยกรรม", "is_today": true },
    { "time": "10:00", "title": "สแตนด์อัปรายวัน", "is_today": false }
  ]
}
```

### Topic: `auradeck/todos`
Published as a **root JSON array** of task objects. Each item's `title` field may carry a `[List Name]` prefix when the profile has multiple Google Task lists configured.
```json
[
  { "id": "task_abc123", "title": "[Work] ตรวจทาน Pull Request #42", "completed": false },
  { "id": "task_def456", "title": "[Shopping] ซื้อของเข้าบ้าน", "completed": false }
]
```
> **ESP32 Handling:** `page_todos.cpp` supports both root-array format (from device-specific topics) and wrapped `{"todos": [...]}` format (from generic topics) for full backward compatibility. The `title` field is extracted directly, preserving the multi-list prefix as display context.

### Topic: `auradeck/stocks`
Published as a **root JSON array** of asset objects. The `type` field drives display formatting on the ESP32 (`GOLD` = Thai Baht, `TH_STOCK` = local equity, `CRYPTO`/`GLOBAL` = USD prefix).
```json
[
  { "symbol": "GOLD/TH", "raw_symbol": "GOLD/TH", "name": "Thai Gold Bar 96.5%", "price": 41200.0, "change_pct": 0.24, "type": "GOLD" },
  { "symbol": "CPALL",   "raw_symbol": "CPALL.BK", "name": "CP ALL Public Company Limited", "price": 57.25, "change_pct": 1.33, "type": "TH_STOCK" },
  { "symbol": "BTC/USD", "raw_symbol": "BTC-USD",  "name": "Bitcoin USD", "price": 64500.0, "change_pct": 2.15, "type": "CRYPTO" }
]
```
> **ESP32 Handling:** `page_stocks.cpp` detects root-array vs wrapped-object format. Asset formatting (currency symbol, decimal places) is driven by the `type` field. The `change_pct` field (backend native) is preferred over the legacy `change_percent` field.

### Topic: `auradeck/antigravity`
```json
{
  "plan": "Google AI Pro",
  "available_credits": 823,
  "ai_credits": 823,
  "gemini_models": {
    "weekly_limit_percent": 99.0,
    "five_hour_limit_percent": 97.0,
    "next_reset_5h": "03h 47m",
    "next_reset_weekly": "6 days, 22 hours"
  },
  "claude_gpt_models": {
    "weekly_limit_percent": 100.0,
    "five_hour_limit_percent": 100.0
  },
  "limit_5h": {
    "used": 3.0,
    "total": 100.0,
    "percentage": 3.0
  },
  "limit_weekly": {
    "used": 1.0,
    "total": 100.0,
    "percentage": 1.0
  },
  "next_reset": "03h 47m",
  "credit_hours_remaining": 823.0,
  "percent_quota_used": 3.0
}
```

### Topic: `auradeck/analytics`
```json
{
  "gcp_status": "OK",
  "ga4_active_users": 34,
  "gsc_clicks": 1420,
  "gsc_impressions": 28500,
  "gcp_billing": [
    { "project_name": "AuraDeck Dev", "cost_mtd": 12.50, "currency": "USD" },
    { "project_name": "Client Prod",  "cost_mtd": 148.20, "currency": "USD" }
  ]
}
```
> **ESP32 Handling:** `page_analytics.cpp` reads `ga4_active_users` (not `active_users`) and sums all `cost_mtd` values from the `gcp_billing` array to display a single total MTD figure.

### Topic: `auradeck/weather`
```json
{
  "date_en": "Thursday, 20 August 2026",
  "date_th": "วันพฤหัสบดีที่ 20 สิงหาคม 2569",
  "current_temp": 28.5,
  "current_condition": "มีเมฆมาก",
  "current_icon": "CLOUD",
  "hourly": [
    { "time": "18:00", "condition": "Drizzle", "icon": "DRIZZLE", "rain_prob": 80, "temp": 28.0 },
    { "time": "19:00", "condition": "Rain", "icon": "RAIN", "rain_prob": 85, "temp": 27.5 },
    { "time": "20:00", "condition": "Drizzle", "icon": "DRIZZLE", "rain_prob": 70, "temp": 27.0 },
    { "time": "21:00", "condition": "Cloudy", "icon": "CLOUD", "rain_prob": 30, "temp": 26.8 },
    { "time": "22:00", "condition": "Cloudy", "icon": "CLOUD", "rain_prob": 20, "temp": 26.5 },
    { "time": "23:00", "condition": "Clear", "icon": "SUN", "rain_prob": 10, "temp": 26.2 }
  ]
}
```
> **ESP32 Handling:** `page_home.cpp` updates the dual-language dates, outdoor temperature/condition, and the 6-slot horizontal hourly rain probability forecast strip.

---

## 5. Special Graphics & Thai Character Handling (แก้ปัญหาสระลอย)

### The Stacking Vowel Challenge
Standard LVGL v8 lacks a complex text-shaping engine (like HarfBuzz). When rendering Thai UTF-8 scripts, combining characters—including upper/lower vowels (e.g., สระอุ, สระอู, สระอี) and tone marks (e.g., ไม้เอก, ไม้โท)—collide vertically, causing "สระลอย" (floating/overlapping glyphs).

### Reshaping and Font PUA Mapping Solution
To fix this, the ESP32-S3 client utilizes a custom **C++ ThaiReshaper** library:
1.  **UTF-8 to Unicode Conversion:** The reshaping engine intercepts Thai string payloads and decodes UTF-8 byte streams into 16-bit Unicode characters.
2.  **Glyph Rules Examination:** It inspects adjacent character types. If an upper vowel is followed by a tone mark, or if characters clash with tall-consonants (like ป, ฝ, ฟ), the engine swaps the standard Unicode code points with pre-compiled **Private Use Area (PUA) codes (`0xF700` to `0xF71F`)**.
3.  **Special Thai Font Asset:** These PUA code points map to specially shifted and vertically adjusted glyph versions baked into our legacy Thai font files, enabling pixel-perfect, overlapping-free multi-layer vowel stacking directly on the 1-bit screen.

---

## 6. AuraDeck 2.0 Secure Google Sign-In & Workspace Isolation

AuraDeck 2.0 introduces secure, multi-tenant workspace isolation gated by unified **Google OAuth Sign-In** using global server credentials, coupled with strict directory sandboxes and TV-style screen pairing.

### Unified Google Identity Gateway & Auto-Provisioning
*   **Zero Profile Leakage:** The landing page (`/login`) is a completely secure gate containing no public list or database leakage of existing users. 
*   **Identity Mapping:** Authenticating with Google triggers OAuth with `openid` and `email` scopes. On successful code exchange, the backend queries Google's Userinfo API to extract the user's email.
*   **Sanitized Sandbox Directory:** The verified email is slugified (e.g. `user@gmail.com` -> `user_gmail_com`) and maps to an isolated directory at `backend/tokens/profiles/{profile_id}/`. Initial workspace configurations are auto-provisioned upon first-time login.

### Deep Session Authorization Checks
*   **Cookie Session Tracking:** On successful authentication, the backend sets an `active_profile_id` cookie.
*   **Endpoint Isolation Guards:** Every endpoint matching `/api/profiles/{profile_id}/*` parses this cookie and validates that `active_profile_id == profile_id`. Unauthorized cross-workspace actions trigger a strict `403 Forbidden` response.
*   **Pairing Gating:** Users can only pair physically displayed screens (obtained via `GET /api/pairing/request`) to their active workspace session during the verification phase (`POST /api/pairing/verify`).

### TV-Style Wireless Screen Pairing Flow
When an unconfigured ESP32 screen boots up, it communicates with the Raspberry Pi to register its physical MAC address and obtain a temporary 6-digit numeric pairing PIN code.

1.  **PIN Allocation:** ESP32 requests a PIN via `GET /api/pairing/request?mac=...`.
2.  **Web Verification:** The user opens the secure AuraDeck Web Portal, enters the PIN, and pairs the physical screen with their logged profile (`POST /api/pairing/verify`).
3.  **Session Binding:** Once paired, the background schedulers retrieve API states per-profile and publish to device-specific MQTT channels: `auradeck/device/{mac}/{service}` (e.g. `auradeck/device/84:F3:EB:C9:4A:E1/spotify`).

### Container-to-Host D-Bus AP Communication
The FastAPI backend container mounts the host Raspberry Pi's system bus (`/var/run/dbus/system_bus_socket`) inside the Docker container environment and utilizes the native `network-manager` tool (`nmcli`) to safely toggle, query, and monitor the host's physical AuraDeck hotspot without security escalation.

---

## 7. Configurable Background Polling Intervals & Live Sync

AuraDeck allows users to customize background polling frequencies on a per-profile basis directly through the Web Control Center UI, balancing data freshness against API quotas and device power consumption.

### Polling Interval Schema (`settings.json` / `/api/v1/profile/intervals`)
```json
{
  "tasks_calendar_mins": 15,
  "stocks_mins": 5,
  "antigravity_mins": 1,
  "analytics_mins": 15,
  "time_sync_secs": 10
}
```

### Endpoints
* `GET /api/v1/profile/intervals`: Retrieves the configured polling intervals for the active profile (falling back to system defaults if unconfigured).
* `POST /api/v1/profile/intervals`: Updates interval settings, resets the scheduler countdown timers for that profile, and immediately triggers an **Instant Fetch & MQTT Push** to all paired ESP32 screens.

### Service Update Policies
* **Spotify:** Polled at a fixed 5-second interval for real-time now-playing tracking.
* **Google Tasks & Calendar:** Configurable from 1 min to 60 mins (Default: 15 mins).
* **Stocks & Crypto:** Configurable from 1 min to 60 mins (Default: 5 mins).
* **Antigravity AI:** Configurable from 1 min to 60 mins (Default: 1 min).
* **Cloud Analytics:** Configurable from 1 min to 60 mins (Default: 15 mins).
* **Hardware RTC Time Sync:** Broadcasts Thailand GMT+7 time every 10 seconds.

---

## 8. Web Control Center 3-Zone Architecture

The Web Control Center (`backend/app/templates/auth.html`) is structured into 3 distinct functional zones arranged in a responsive 2-column dashboard:

### Zone 1: 🔌 Connected Cloud Accounts & Analytics
* **Google Workspace OAuth:** Connects Google Calendar agendas and Google Tasks checklists.
* **Spotify Premium OAuth:** Connects Spotify playback metadata and 1-bit monochrome cover art pipeline.
* **Google Analytics 4 (GA4):** Dedicated card for GA4 Property ID configuration with real-time test connection endpoint (`POST /api/v1/ga4/test`) and independent save handler.
* **GCP Multi-Project Manager:** Drag & drop JSON upload for Service Account keys and project management for multi-tenant billing cost tracking.

### Zone 2: 🎨 Widget Content & Display Customization
* **Weather Location (แผนที่พยากรณ์อากาศ):** Interactive Leaflet Map with CartoDB Voyager tiles, custom glowing Aura pin, Nominatim POI/ROI autocomplete search, and HTML5 GPS Geolocation locator with independent save and instant screen synchronization.
* **Stock & Asset Watchlist:** Yahoo Finance autocomplete search and watchlist management table for global/Thai stocks, crypto, and gold.
* **Spotify Monochrome Cover Preview:** Real-time Floyd-Steinberg 1-bit dithered album cover preview.
* **Selected Google Task Lists:** Multi-list selection filter for tasks aggregation.

### Zone 3: 📺 Hardware, Network & Polling Intervals
* **Background Sync Intervals:** 6 independent frequency dropdown selectors with live instant-sync trigger.
* **Pair AuraDeck Screen:** 6-digit TV-style screen pairing PIN manager.
* **AuraDeck Local Access Point:** Real-time host hotspot AP status and remote reboot control.