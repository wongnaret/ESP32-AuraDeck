# AuraDeck API & Pub-Sub Documentation

This document describes all local REST endpoints, OAuth2 flows, and MQTT pub-sub payload schemas used to synchronize data between the Raspberry Pi backend and the ESP32-S3 frontend screen.

---

## 🔌 Local REST API Endpoints

### 1. Multi-Profile Session Management

#### `GET /api/profiles`
Lists the active user's profile (restricted to the logged-in user).
*   **Request Cookies:** `active_profile_id`
*   **Response (`200 OK`):**
    ```json
    [
      { "id": "john_gmail_com", "name": "John Doe" }
    ]
    ```

#### `POST /api/profiles` [DEPRECATED]
Disabled. Manual profile creation is forbidden. Profiles are auto-provisioned upon Google Sign-In.
*   **Response (`403 Forbidden`):**
    ```json
    { "detail": "Manual profile creation is disabled. Use Google Login to auto-provision profiles." }
    ```

#### `DELETE /api/profiles/{profile_id}`
Deletes the profile's tokens and configurations, and unpairs any screens. Requires the `active_profile_id` cookie to match `{profile_id}`.
*   **Request Cookies:** `active_profile_id`
*   **Response (`200 OK`):**
    ```json
    {
      "status": "success",
      "message": "Successfully deleted profile john_gmail_com"
    }
    ```
*   **Error Response (`403 Forbidden`):** If the session cookie does not match `{profile_id}`.

#### `GET /api/profiles/{profile_id}/config`
Retrieves a profile's current safe configuration (secrets are omitted or represented as boolean states).
*   **Response (`200 OK`):**
    ```json
    {
      "profile_name": "Office Studio",
      "ga_property_id": "453120000",
      "google_client_id": "your-client-id.apps.googleusercontent.com",
      "google_client_secret_configured": true,
      "google_redirect_uri": "http://localhost:8000/google/callback",
      "active_task_lists": ["@default", "list_work_items"],
      "google_sa_configured": true
    }
    ```

#### `POST /api/profiles/{profile_id}/config`
Updates configuration settings for a profile.
*   **Request Body (`application/json`):**
    ```json
    {
      "ga_property_id": "453120000",
      "active_task_lists": ["@default", "list_work_items"]
    }
    ```
*   **Response (`200 OK`):**
    ```json
    {
      "status": "success",
      "message": "Profile configuration updated successfully."
    }
    ```

#### `POST /api/profiles/{profile_id}/upload-secrets`
Accepts a JSON upload of OAuth client secrets or legacy single-project Service Account credentials.
*   **Query Parameters:** `type` (Supported options: `oauth`, `service_account`)
*   **Request Form File:** `file` (the JSON file)
*   **Response (`200 OK`):**
    ```json
    {
      "status": "success",
      "message": "Service Account key file uploaded and installed successfully."
    }
    ```

#### `GET /api/profiles/{profile_id}/gcp-projects`
Lists all multi-project Service Account keys configured under the profile's dedicated storage directory.
*   **Response (`200 OK`):**
    ```json
    [
      {
        "project_id": "client-prod-billing",
        "client_email": "billing-reader@client-prod-billing.iam.gserviceaccount.com"
      },
      {
        "project_id": "internal-auradeck-dev",
        "client_email": "auradeck-sa@internal-auradeck-dev.iam.gserviceaccount.com"
      }
    ]
    ```

#### `POST /api/profiles/{profile_id}/gcp-projects/upload`
Uploads and installs a new Service Account JSON key, extracting its native `project_id` and locking it to `{project_id}.json` in the profile workspace folder.
*   **Request Form File:** `file` (the service account JSON key file)
*   **Response (`200 OK`):**
    ```json
    {
      "status": "success",
      "project_id": "internal-auradeck-dev",
      "message": "Successfully added GCP Project 'internal-auradeck-dev' with Service Account."
    }
    ```

#### `DELETE /api/profiles/{profile_id}/gcp-projects/{project_id}`
Removes a specific GCP project service account key from the profile's workspace directory.
*   **Response (`200 OK`):**
    ```json
    {
      "status": "success",
      "message": "Successfully removed GCP Project 'internal-auradeck-dev'."
    }
    ```
---

### 2. Unified Google Login & OAuth2 Redirection Endpoints

#### `GET /google/login`
Redirects the user's web browser to the Google Consent screen to login or grant additional scopes.
*   **Query Parameters (Optional):** `profile_id` (fallback is `"login_session"`)
*   **If `profile_id` is `"login_session"`:** Requests openid identity scopes (`openid`, `email`, `profile`) in addition to integration scopes (`calendar.readonly`, `tasks.readonly`).

#### `GET /google/callback?code={code}&state={state}`
Processes the authorization code returned by Google.
*   **If `state == "login_session"`:**
    *   Exchanges the authorization code.
    *   Queries Google's Userinfo API to verify their email address.
    *   Sanitizes the email to obtain the `profile_id` (e.g., `john_gmail_com`).
    *   Auto-provisions the directory `/backend/tokens/profiles/john_gmail_com/` and safe default configurations if they don't exist.
    *   Sets the `active_profile_id=john_gmail_com` cookie.
    *   Redirects the user directly to the home dashboard `/`.
*   **If `state == {profile_id}`:**
    *   Integrates calendar/tasks credentials for that specific profile directory, persisting `google_tokens.json`.

---

#### `GET /spotify/login?profile_id={profile_id}`
Redirects the user's web browser to the Spotify Account login screen to authorize player metadata access. Passes `profile_id` as the state parameter.

#### `GET /spotify/callback?code={code}&state={profile_id}`
Processes the authorization code returned by Spotify, performs the authorization code exchange, and persists the credentials in `backend/tokens/profiles/{profile_id}/spotify_tokens.json`.

---

### 3. TV-Style Device Screen Pairing Flow

#### `GET /api/pairing/request?mac={mac}`
Generates a temporary unique 6-digit PIN code for an unconfigured ESP32 screen booting up.
*   **Response (`200 OK`):**
    ```json
    {
      "pin": "645902",
      "expires_in_secs": 300
    }
    ```

#### `POST /api/pairing/verify`
Pairs a temporary PIN entered by the user in the Web Interface with their currently active profile session.
*   **Request Body (`application/json`):**
    ```json
    {
      "pin": "645902",
      "profile_id": "profile_office_studio"
    }
    ```
*   **Response (`200 OK`):**
    ```json
    {
      "status": "success",
      "message": "Device paired successfully!",
      "mac": "84:F3:EB:C9:4A:E1"
    }
    ```

#### `GET /api/pairing/status?mac={mac}`
Queries if the device is currently paired.
*   **Response (`200 OK`):**
    ```json
    {
      "paired": true,
      "profile_id": "profile_office_studio"
    }
    ```

#### `GET /api/pairing/list`
Lists all screens currently paired with the user's active profile session.
*   **Response (`200 OK`):**
    ```json
    [
      { "mac": "84:F3:EB:C9:4A:E1", "paired_at": "2026-07-22T14:35:00Z" }
    ]
    ```

---

### 4. Local Access Point Configuration (Host nmcli)

#### `GET /api/ap/status`
Queries the active state of the AuraDeck Hotspot AP, client count, and default IP.
*   **Response (`200 OK`):**
    ```json
    {
      "status": "Active",
      "ssid": "AuraDeck_Hotspot",
      "password": "AuraDeck1234",
      "gateway": "10.42.0.1",
      "clients_connected": 2,
      "is_mock": false
    }
    ```

#### `POST /api/ap/restart`
Triggers an asynchronous toggle of NetworkManager's Hotspot (down and back up) to resolve supplicant or driver timeout issues.
*   **Response (`200 OK`):**
    ```json
    {
      "status": "success",
      "message": "AuraDeck AP Hotspot restarted successfully."
    }
    ```

---

### 5. Stock Watchlist & Search Endpoints

#### `GET /api/v1/stocks/search?q={query}`
Searches Yahoo Finance tickers using ticker symbol or company name with autocomplete support.
*   **Query Parameters:** `q` (e.g., `CPALL`, `NVDA`, `BTC`)
*   **Response (`200 OK`):**
    ```json
    [
      {
        "symbol": "CPALL.BK",
        "name": "CP ALL PUBLIC COMPANY LIMITED",
        "exchange": "SET",
        "type": "TH_STOCK"
      },
      {
        "symbol": "CPALL-F.BK",
        "name": "CP ALL PUBLIC COMPANY LIMITED",
        "exchange": "SET",
        "type": "TH_STOCK"
      }
    ]
    ```

#### `GET /api/v1/stocks/watchlist?profile_id={profile_id}`
Gets live prices and metadata for all configured stocks in the specified profile's watchlist.
*   **Query Parameters:** `profile_id` (default `"default"`)
*   **Response (`200 OK`):**
    ```json
    {
      "profile_id": "default",
      "items": [
        {
          "symbol": "CPALL",
          "raw_symbol": "CPALL.BK",
          "name": "CP ALL PUBLIC COMPANY LIMITED",
          "price": 58.25,
          "change": 0.5,
          "change_pct": 0.87,
          "type": "TH_STOCK"
        },
        {
          "symbol": "BTC",
          "raw_symbol": "BTC-USD",
          "name": "Bitcoin USD",
          "price": 64230.1,
          "change": 1200.0,
          "change_pct": 1.9,
          "type": "CRYPTO"
        }
      ]
    }
    ```

#### `POST /api/v1/stocks/watchlist`
Adds a new stock symbol to the specified profile's watchlist.
*   **Request Body:**
    ```json
    {
      "symbol": "CPALL.BK",
      "name": "CP ALL PUBLIC COMPANY LIMITED",
      "profile_id": "default"
    }
    ```
*   **Response (`200 OK`):**
    ```json
    {
      "status": "success",
      "message": "Added CPALL.BK to watchlist."
    }
    ```

#### `DELETE /api/v1/stocks/watchlist?symbol={symbol}&profile_id={profile_id}`
Removes a stock from the specified profile's watchlist.
*   **Query Parameters:** `symbol`, `profile_id`
*   **Response (`200 OK`):**
    ```json
    {
      "status": "success",
      "message": "Removed CPALL.BK from watchlist."
    }
    ```

#### `GET /api/v1/stocks/debug?publish={bool}&profile_id={profile_id}`
Diagnostic endpoint — fetches fresh prices from all data sources and reports MQTT connection status. Optionally republishes to all relevant MQTT topics.
*   **Query Parameters:** `publish` (default `false`), `profile_id` (default `"default"`)
*   **Response (`200 OK`):**
    ```json
    {
      "mqtt_connected": true,
      "profile_id": "default",
      "watchlist_configured": false,
      "watchlist_items": null,
      "default_prices": [
        { "symbol": "GOLD/TH", "price": 45000.0, "change_pct": 0.0, "type": "GOLD" },
        { "symbol": "CPALL", "price": 47.0, "change_pct": 1.08, "type": "TH_STOCK" },
        { "symbol": "BTC/USD", "price": 65000.0, "change_pct": 2.5, "type": "CRYPTO" },
        { "symbol": "GC", "price": 2350.0, "change_pct": 0.3, "type": "COMMODITY" }
      ],
      "profile_prices": [ "... same as default_prices if no custom watchlist ..." ]
    }
    ```
*   **Asset type values:** `GOLD` (Thai gold bar), `TH_STOCK` (SET-listed equity), `CRYPTO` (cryptocurrency), `COMMODITY` (futures like GC=F), `GLOBAL` (other international equities)

### 6. Google Analytics 4 (GA4) & GCP Billing Endpoints

#### `GET /api/v1/ga4?profile_id={profile_id}`
Fetches real-time 30-minute visitor counts, 28-day active/new users, average engagement duration, total events, and top cities from GA4 Data API.
*   **Query Parameters:** `profile_id` (default `"default"`)
*   **Response (`200 OK`):**
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

#### `POST /api/v1/ga4/test`
Validates credentials and queries the GA4 Data API in real-time to confirm property access.
*   **Request Body (`application/json`):**
    ```json
    {
      "property_id": "453120000",
      "profile_id": "default"
    }
    ```
*   **Response (`200 OK`):**
    ```json
    {
      "success": true,
      "message": "Successfully connected to GA4 Property 453120000! Realtime Active Users: 34",
      "active_users_30m": 34
    }
    ```

#### `GET /api/v1/gcp/billing?profile_id={profile_id}`
Scans and aggregates multi-project GCP billing status, month-to-date costs, month-end forecasts, top 4 services breakdown, and 10-day daily spend arrays.
*   **Query Parameters:** `profile_id` (default `"default"`)
*   **Response (`200 OK`):**
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

---

## 📡 MQTT Topic Payload Schemas

To support multi-screen configurations, the backend publishes telemetry payloads targeting specific MAC addresses:

```
auradeck/device/{mac}/spotify
auradeck/device/{mac}/calendar
auradeck/device/{mac}/todos
auradeck/device/{mac}/stocks
auradeck/device/{mac}/ga4
auradeck/device/{mac}/gcp
auradeck/device/{mac}/antigravity
auradeck/device/{mac}/weather
```

All payloads published to local topics are formatted as high-density, flat JSON objects, optimized for minimal parsing overhead on the ESP32-S3 microcontroller.

> **📌 ESP32 Topic Normalization:** The ESP32 firmware (`network_manager.cpp`) automatically normalizes device-specific topics (`auradeck/device/{mac}/{service}`) into their generic form (`auradeck/{service}`) before dispatching to UI page modules. This means `ui_manager.dispatchData()` and all page `update_page_*()` functions remain agnostic to whether the device is paired or running in generic dev mode.

### 1. Spotify Now Playing (`auradeck/device/{mac}/spotify`)
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
| Field | Type | Description |
|-------|------|-------------|
| `is_playing` | bool | Whether Spotify is actively streaming |
| `title` | string | Track name (from Spotify API `item.name`) |
| `artist` | string | Comma-separated artist names |
| `progress_ms` | int | Playback progress in **milliseconds** |
| `duration_ms` | int | Total track duration in **milliseconds** |

> **ESP32 Handling:** `page_spotify.cpp` reads `title` (with `track` as legacy fallback) and converts `progress_ms`/`duration_ms` from milliseconds to seconds for UI display.

### 2. Calendar Agenda (`auradeck/device/{mac}/calendar`)
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

### 3. Google Tasks Checklist (`auradeck/device/{mac}/todos`)
Published as a **root JSON array** of task objects. When multiple Google Task lists are configured in the profile (`active_task_lists`), each task's `title` includes a `[List Name]` prefix for contextual visibility.
```json
[
  { "id": "task_abc123", "title": "[Work] ตรวจทาน Pull Request #42", "completed": false },
  { "id": "task_def456", "title": "[Shopping] ซื้อของเข้าบ้าน", "completed": false },
  { "id": "task_ghi789", "title": "Deploy hotfix to production", "completed": false }
]
```
| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Google Tasks internal task ID |
| `title` | string | Task text, prefixed with `[List Name]` when from a non-default list |
| `completed` | bool | Always `false` (completed tasks are filtered server-side) |

> **ESP32 Handling:** `page_todos.cpp` supports both root-array (from device topics) and wrapped `{"todos": [...]}` (from generic topics) formats for backward compatibility. Renders up to **4 items**; remaining rows are cleared.

### 4. Multi-Asset Stocks & Commodities Watchlist (`auradeck/device/{mac}/stocks`)
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

### 5. Antigravity Quota Usage (`auradeck/device/{mac}/antigravity`)
Monitors hourly and weekly AI credits.
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

### 6. GCP Billing & Web Analytics (`auradeck/device/{mac}/analytics`)
Monitors overall multi-project MTD spending alongside GA4 real-time indicators.
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
