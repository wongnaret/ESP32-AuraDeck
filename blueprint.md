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
* **Target Board:** Waveshare ESP32-S3-RLCD-4.2 (ESP32-S3-WROOM-1-N16R8)
* **Display:** 4.2-inch Reflective LCD (RLCD), 400x300 Resolution (Landscape Mode)
* **Onboard Hardware Utilized:**
  * `ST7305`: High-performance display controller with precomputed O(1) Landscape LUT transformation
  * `SHTC3`: Ambient Temperature & Humidity Sensor (I2C)
  * `PCF85063`: Hardware RTC with battery backup for offline time-keeping
  * `KEY Button`: Physical GPIO18 input for thread-safe screen cycling
* **Graphics Framework:** LVGL v8.x+ (1-bit monochrome, theme mono, custom PSRAM allocator)
* **Backend Stack:** Docker Containers (FastAPI & Mosquitto MQTT Broker) running on Raspberry Pi

---

## 3. UI Screen Specifications (Page 0 – Page 6)

### Persistent Header (Displayed across ALL pages)
* **Layout:** Top status bar (0 to 25px vertical slice, partitioned from view screen)
* **Elements:**
  * Page Title Name (Left aligned, dynamically reflects the current screen context)
  * Wi-Fi Connection Icon (Left aligned: `📶` when online, `⚠️` when offline)
  * SHTC3 Live Temperature (Right aligned: e.g. `26.5°C`)
  * Local Time (`HH:MM`) synced via NTP + RTC (Right aligned)
  * Bottom divider line rule (1px high-contrast stroke at y=25px)

---

### Page 0: Home (Minimalist Summary Dashboard)
* **Purpose:** Single-glance overview combining the most critical metrics from all pages.
* **Layout Elements:**
  * **Digital Clock:** Large central bold digital time display (~48px font size).
  * **Calendar Date:** Prominent center-aligned date string (e.g., `Wednesday, July 22`).
  * **SHTC3 Live Readout:** Prominent center-bottom sensor card showing ambient temp & humidity.
  * **System Status:** Footer label indicating gateway connectivity state.

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

---

### Page 4: Calendar (Chronological Agendas)
* **Purpose:** Chronicle immediate Google Calendar events.
* **Source:** Google Calendar API.
* **Layout Elements:**
  * Clean multi-line schedule list showcasing the next 3 chronological agendas.
  * Displays: `[Start Time] Event Title (Today/Tomorrow)`.
  * Integrated C++ ThaiReshaper and auto-ellipsis truncation protecting layout margins.

---

### Page 5: Spotify Now Playing
* **Purpose:** Real-time media progress and track status.
* **Layout Elements:**
  * **Track Title:** Primary bold 24px label with ellipsis protection.
  * **Artist Name:** Sub-title artist metadata (~16px).
  * **Linear Progress Bar:** Full playback visualizer showing elapsed vs total duration.
  * **Elapsed Timestamps:** Left and right text readouts (e.g. `1:23 / 3:45`).
  * **Graceful Degradation:** Displays a beautiful "Spotify Offline / Idle" layout when inactive.

---

### Page 6: DevOps, Analytics & GCP Billing
* **Purpose:** Web traffic metrics, and Google Cloud operational cost insights.
* **Layout Elements:**
  * **Real-time Traffic (GA4):** Massive, eye-catching active visitor count (`00` up to `99` in ~48px font).
  * **GCP Cloud Billing:** Dedicated Month-to-Date (MTD) accumulated spend aggregation in US Dollars.
  * **Dividing Rail:** Neat high-contrast vertical dividing line parting the dashboard columns.

---

## 4. Server-to-ESP32 MQTT Payload Schema
The Raspberry Pi publishes structured, single-responsibility telemetry objects on specific MQTT topics. Below are the implemented JSON schemas:

### Topic: `auradeck/spotify`
```json
{
  "is_playing": true,
  "track": "เพลงรักในสายลม",
  "artist": "วงดนตรีสากล",
  "progress": 128,
  "duration": 240
}
```

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
```json
{
  "todos": [
    "ตรวจทาน Pull Request #42",
    "ติดตั้งโปรแกรมปรับปรุงระบบฐานข้อมูล"
  ]
}
```

### Topic: `auradeck/stocks`
```json
{
  "stocks": [
    { "symbol": "SET50", "price": 942.50, "change_percent": 1.33 },
    { "symbol": "CPALL.BK", "price": 57.25, "change_percent": -0.45 },
    { "symbol": "GOLD", "price": 41200.00, "change_percent": 0.24 },
    { "symbol": "BTC-THB", "price": 2350000.00, "change_percent": 2.15 }
  ]
}
```

### Topic: `auradeck/antigravity`
```json
{
  "credit_hours_remaining": 4.5,
  "percent_quota_used": 72.0
}
```

### Topic: `auradeck/analytics`
```json
{
  "active_users": 15,
  "mtd_billing": 160.70
}
```

---

## 5. Special Graphics & Thai Character Handling (แก้ปัญหาสระลอย)

### The Stacking Vowel Challenge
Standard LVGL v8 lacks a complex text-shaping engine (like HarfBuzz). When rendering Thai UTF-8 scripts, combining characters—including upper/lower vowels (e.g., สระอุ, สระอู, สระอี) and tone marks (e.g., ไม้เอก, ไม้โท)—collide vertically, causing "สระลอย" (floating/overlapping glyphs).

### Reshaping and Font PUA Mapping Solution
To fix this, the ESP32-S3 client utilizes a custom **C++ ThaiReshaper** library:
1.  **UTF-8 to Unicode Conversion:** The reshaping engine intercepts Thai string payloads and decodes UTF-8 byte streams into 16-bit Unicode characters.
2.  **Glyph Rules Examination:** It inspects adjacent character types. If an upper vowel is followed by a tone mark, or if characters clash with tall-consonants (like ป, ฝ, ฟ), the engine swaps the standard Unicode code points with pre-compiled **Private Use Area (PUA) codes (`0xF700` to `0xF71F`)**.
3.  **Special Thai Font Asset:** These PUA code points map to specially shifted and vertically adjusted glyph versions baked into our legacy Thai font files, enabling pixel-perfect, overlapping-free multi-layer vowel stacking directly on the 1-bit screen.