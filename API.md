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
      "gcp_project_id": "auradeck-dashboard",
      "google_client_id": "your-client-id.apps.googleusercontent.com",
      "google_client_secret_configured": true,
      "google_redirect_uri": "http://localhost:8000/google/callback",
      "spotify_client_id": "your-spotify-id",
      "spotify_client_secret_configured": true,
      "spotify_redirect_uri": "http://localhost:8000/spotify/callback",
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
      "gcp_project_id": "auradeck-dashboard",
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
Accepts a JSON upload of OAuth client secrets or Service Account credentials.
*   **Query Parameters:** `type` (Supported options: `oauth`, `service_account`)
*   **Request Form File:** `file` (the JSON file)
*   **Response (`200 OK`):**
    ```json
    {
      "status": "success",
      "message": "Service Account key file uploaded and installed successfully."
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

## 📡 MQTT Topic Payload Schemas

To support multi-screen configurations, the backend publishes telemetry payloads targeting specific MAC addresses:

```
auradeck/device/{mac}/spotify
auradeck/device/{mac}/calendar
auradeck/device/{mac}/todos
auradeck/device/{mac}/stocks
auradeck/device/{mac}/antigravity
auradeck/device/{mac}/analytics
```

All payloads published to local topics are formatted as high-density, flat JSON objects, optimized for minimal parsing overhead on the ESP32-S3 microcontroller.

### 1. Spotify Now Playing (`auradeck/device/{mac}/spotify`)
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
A flat array of aggregated task items, with the list name appended inside brackets:
```json
[
  { "id": "1", "title": "[Work] ตรวจทาน Pull Request #42", "completed": false },
  { "id": "2", "title": "[Home] ซื้อของเข้าบ้าน", "completed": false }
]
```

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
  "credit_hours_remaining": 4.5,
  "percent_quota_used": 72.0
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
