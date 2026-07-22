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
        "track": "เพลงรักในสายลม",
        "artist": "วงดนตรีสากล",
        "progress": 120,
        "duration": 240
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
      "track": "เพลงรักในสายลม",
      "artist": "วงดนตรีสากล",
      "progress": 134,
      "duration": 240
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
  "track": "เพลงรักในสายลม",
  "artist": "วงดนตรีสากล",
  "progress": 128,
  "duration": 240
}
```

### 2. Calendar Agenda (`auradeck/calendar`)
Combines a monthly grid marker list with specific details for Today and Tomorrow.
```json
{
  "month_days_with_events": [1, 5, 12, 15, 20, 21, 22, 28],
  "events": [
    { "time": "14:00", "title": "ประชุมทีมสถาปัตยกรรม", "is_today": true },
    { "time": "10:00", "title": "สแตนด์อัปรายวัน", "is_today": false }
  ]
}
```

### 3. Google Tasks Checklist (`auradeck/todos`)
A flat array of top task items.
```json
{
  "todos": [
    "ตรวจทาน Pull Request #42",
    "ติดตั้งโปรแกรมปรับปรุงระบบฐานข้อมูล"
  ]
}
```

### 4. Multi-Asset Stocks & Commodities Watchlist (`auradeck/stocks`)
Holds current prices, percentage change direction, and asset categories (Thai Equities, Gold bars, Cryptocurrencies).
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

### 5. Antigravity Quota Usage (`auradeck/antigravity`)
Monitors hourly and weekly AI credits.
```json
{
  "credit_hours_remaining": 4.5,
  "percent_quota_used": 72.0
}
```

### 6. GCP Billing & Web Analytics (`auradeck/analytics`)
Monitors overall multi-project MTD spending alongside standard web traffic indicators.
```json
{
  "active_users": 15,
  "mtd_billing": 160.70
}
```
