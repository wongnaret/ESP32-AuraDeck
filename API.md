# AuraDeck API & Pub-Sub Documentation

This document describes all local REST endpoints, OAuth2 flows, and MQTT pub-sub payload schemas used to synchronize data between the Raspberry Pi backend and the ESP32-S3 frontend screen.

---

## 🔌 Local REST API Endpoints

### 1. OAuth2 Redirection Endpoints

#### `GET /google/login`
Redirects the administrator's web browser to the Google Consent screen to obtain permission for Calendar and Tasks access.

#### `GET /google/callback`
Processes the authorization code returned by Google, performs the authorization code exchange, and persists the refresh token securely in `backend/tokens/google_tokens.json`.

---

#### `GET /spotify/login`
Redirects the administrator's web browser to the Spotify Account login screen to authorize player metadata access.

#### `GET /spotify/callback`
Processes the authorization code returned by Spotify, performs the authorization code exchange, and persists the credentials in `backend/tokens/spotify_tokens.json`.

---

### 2. Config & Control Endpoints

#### `GET /api/status`
Checks if valid refresh tokens are currently cached for both Google and Spotify.
*   **Response (`200 OK`):**
    ```json
    {
      "google": true,
      "spotify": false
    }
    ```

#### `POST /api/publish`
Receives an arbitrary payload and publishes it directly to a local Mosquitto topic with `QoS=1` and `retain=true`.
*   **Request Body (`application/json`):**
    ```json
    {
      "topic": "auradeck/spotify",
      "payload": {
        "is_playing": true,
        "title": "เพลงรักในสายลม",
        "artist": "วงดนตรีสากล",
        "progress_ms": 120000,
        "duration_ms": 240000
      }
    }
    ```
*   **Response (`200 OK`):**
    ```json
    {
      "status": "success",
      "message": "Successfully published to auradeck/spotify"
    }
    ```

#### `POST /api/sync/{service}`
Instructs the server to manually poll a specific API, immediately publish the compiled state to Mosquitto, and return the payload in the response body.
*   **Path Parameter:** `service` (Supported options: `spotify`, `calendar`, `todos`, `stocks`, `antigravity`, `analytics`)
*   **Response (`200 OK`):**
    ```json
    {
      "is_playing": true,
      "title": "เพลงรักในสายลม",
      "artist": "วงดนตรีสากล",
      "progress_ms": 134000,
      "duration_ms": 240000
    }
    ```

---

## 📡 MQTT Topic Payload Schemas (1-bit Screen Friendly)

All payloads published to local topics are formatted as high-density, flat JSON objects, optimized for minimal parsing overhead on the ESP32-S3 microcontroller.

### 1. Spotify Now Playing (`auradeck/spotify`)
Contains playback tracking metrics.
```json
{
  "is_playing": true,
  "title": "เพลงรักในสายลม",
  "artist": "วงดนตรีสากล",
  "progress_ms": 128000,
  "duration_ms": 240000
}
```

### 2. Two-Part Calendar Agenda (`auradeck/calendar`)
Combines a monthly grid marker list with specific details for Today and Tomorrow.
```json
{
  "month_days_with_events": [1, 5, 12, 15, 20, 21, 22, 28],
  "events": [
    { "time": "14:00", "title": "ประชุมทีมสถาปัตยกรรม", "is_today": true },
    { "time": "Tomorrow 10:00", "title": "สแตนด์อัปรายวัน", "is_today": false }
  ]
}
```

### 3. Google Tasks Checklist (`auradeck/todos`)
A flat array of top task items.
```json
[
  { "id": "1", "title": "ตรวจทาน Pull Request #42", "completed": false },
  { "id": "2", "title": "ติดตั้งโปรแกรมปรับปรุงระบบฐานข้อมูล", "completed": false }
]
```

### 4. Multi-Asset Stocks & Commodities Watchlist (`auradeck/stocks`)
Holds current prices, percentage change direction, and assets categories (Thai Equities, Gold bars, Cryptocurrencies).
```json
[
  { "symbol": "CPALL", "price": 57.25, "change_pct": 1.33, "type": "TH_STOCK" },
  { "symbol": "BTC/THB", "price": 2350000.00, "change_pct": 2.15, "type": "CRYPTO" },
  { "symbol": "GOLD_TH", "price": 41200.00, "change_pct": -0.24, "type": "GOLD" }
]
```

### 5. Antigravity Quota Usage (`auradeck/antigravity`)
Monitors hourly and weekly AI credits.
```json
{
  "limit_5h": { "used": 12.5, "total": 50.0, "percentage": 25.0 },
  "limit_weekly": { "used": 140.0, "total": 500.0, "percentage": 28.0 },
  "next_reset": "02h 15m"
}
```

### 6. GCP Billing & Web Analytics (`auradeck/analytics`)
Monitors overall multi-project MTD spending (from different billing accounts) alongside standard web traffic indicators.
```json
{
  "gcp_status": "OK",
  "ga4_active_users": 34,
  "gsc_clicks": 1420,
  "gsc_impressions": 28500,
  "gcp_billing": [
    { "project_name": "AuraDeck Dev", "cost_mtd": 12.50, "currency": "USD" },
    { "project_name": "Client Prod", "cost_mtd": 148.20, "currency": "USD" }
  ]
}
```
