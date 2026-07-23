# AuraDeck Backend Server Guide & Setup

This guide details the system architecture, step-by-step installation instructions, developer sandbox capabilities, and troubleshooting procedures for the AuraDeck Python/FastAPI backend on your Raspberry Pi or local server.

---

## 🏗️ System Architecture

```
                 [ Third-Party Cloud APIs ]
 (Google Calendar / Tasks, Spotify, Yahoo Finance, GCP Billing)
                            │
                            ▼
          ┌───────────────────────────────────┐
          │      Raspberry Pi (Backend)       │
          │  - Python FastAPI Service         │
          │  - Eclipse Mosquitto Broker       │
          │  - Periodic Cron Background Poll  │
          │  - OAuth2 Silent Refresh Manager  │
          └─────────────────┬─────────────────┘
                            │ Wi-Fi (JSON Payload)
                            ▼
          ┌───────────────────────────────────┐
          │  ESP32-S3 RLCD Smart Terminal     │
          │  - 4.2" 1-bit reflective LCD      │
          │  - LVGL v8 Graphics Engine        │
          │  - Real-time Temp/Hum (SHTC3)     │
          │  - Hardware RTC (PCF85063)        │
          └───────────────────────────────────┘
```

---

## ⚡ Backend Getting Started (Raspberry Pi / Local Server)

The backend is fully containerized with **Docker** and **Docker Compose** for instant, environment-isolated deployments. Additionally, we support configuring the Raspberry Pi as a **Local Wi-Fi Access Point (AP)** to bypass corporate security policies and client isolation.

### 1. Prerequisites
Ensure you have Docker and Docker Compose installed:
*   [Install Docker Engine / Desktop](https://docs.docker.com/get-docker/)

### 2. Configure Local Wi-Fi Access Point (Recommended for Office/Enterprise)
If deploying in an office or university with WPA2-Enterprise or client isolation, use the automated setup script to turn your Raspberry Pi into a private Wi-Fi hotspot:
```bash
sudo chmod +x backend/scripts/setup_ap.sh
sudo ./backend/scripts/setup_ap.sh
```
This script configures a persistent hotspot on the Pi with:
*   **SSID:** `AuraDeck_Hotspot`
*   **Password:** `AuraDeck1234`
*   **Pi Server IP:** `10.42.0.1` (which acts as the static gateway, API endpoint, and MQTT broker for the ESP32 screen!)

> [!TIP]
> **Troubleshooting Supplicant/Authentication Timeouts:**
> If you encounter `Error: Connection activation failed: 802.1X supplicant took too long to authenticate` on your Raspberry Pi, the setup script automatically mitigates this by disabling **PMF (Protected Management Frames)** and **Wi-Fi Power Save mode**, which commonly cause Broadcom driver-level handshaking timeouts. If the issue persists, run `sudo systemctl restart NetworkManager` and try executing the script again.

### 3. Spin Up Container Cluster
To check environment variables and launch the API server + MQTT broker cluster in the background, run the control script:
```bash
chmod +x backend/scripts/start_backend.sh
./backend/scripts/start_backend.sh
```
This automatically ensures `.env` is initialized, verifies your docker daemon, and executes:
```bash
docker compose up -d --build
```

### 4. Verify Services
*   **AuraDeck Control Center & Developer Sandbox:** Go to [http://localhost:8000](http://localhost:8000) (or [http://10.42.0.1:8000](http://10.42.0.1:8000) when connected to the AP)
*   **Swagger Interactive REST Documentation:** Go to [http://localhost:8000/docs](http://localhost:8000/docs)
*   **MQTT Broker Endpoint:** Port `1883` on your host machine.

### 5. Secure OAuth Setup via ngrok (Recommended for Pi Deployment)
Since both Google and Spotify enforce strict security policies, they forbid redirecting back to raw local LAN IPs (`http://192.168.1.x`) over unsecure HTTP. To complete the OAuth flow directly on your physical Raspberry Pi over Wi-Fi, you can set up an HTTPS tunnel using **ngrok**.

We provide an automated setup script that installs ngrok (if missing) and registers your credentials in one step on your Raspberry Pi:

1.  **Run the Automated Setup Script:**
    ```bash
    chmod +x backend/scripts/setup_ngrok.sh
    ./backend/scripts/setup_ngrok.sh <your_ngrok_authtoken>
    ```
    *(Note: If you omit the authtoken argument, the script will interactively prompt you to paste it.)*

2.  **Launch the HTTPS Tunnel:**
    Start the secure tunnel on port 8000:
    ```bash
    ngrok http 8000
    ```
3.  **Update Environment Configuration:**
    Copy the generated secure `https://xxxx-xxxx.ngrok-free.app` URL from your ngrok terminal, open your `.env` file on the Pi, and update the redirect URIs:
    ```env
    GOOGLE_REDIRECT_URI=https://xxxx-xxxx.ngrok-free.app/google/callback
    SPOTIFY_REDIRECT_URI=https://xxxx-xxxx.ngrok-free.app/spotify/callback
    ```
4.  **Add Authorized Redirect URIs in Developer Portals:**
    *   **Google Cloud Console (Audience/Audience details):** Add `https://xxxx-xxxx.ngrok-free.app/google/callback` to your **Authorized redirect URIs**.
    *   **Spotify Developer Dashboard:** Add `https://xxxx-xxxx.ngrok-free.app/spotify/callback` to your App's **Redirect URIs** (remember to click **Add** and **Save**).
5.  **Perform Login:**
    Access your dashboard by going to `https://xxxx-xxxx.ngrok-free.app` from your phone or laptop on the same network, click **Sign In with Google**, and the server will automatically authenticate you, set up your secure cookie session, and auto-provision your private dashboard!

### 6. Alternative: Local Token Mirroring (Offline / No-ngrok Shortcut)
If you already completed the OAuth logins on your local computer and do not want to set up public tunnels, you can use the offline mirror shortcut:
1.  Complete the login flow on your local computer (`http://127.0.0.1:8000`).
2.  Navigate to your local `backend/tokens/profiles/` directory. You will see cached user workspace folders (e.g., `profiles/john_gmail_com/` containing `google_tokens.json`, `settings.json`, and `spotify_tokens.json`).
3.  Copy/FTP the entire `profiles` folder directly to the `/backend/tokens/profiles/` path on your Raspberry Pi.
4.  The backend service on the Pi will immediately detect these directories, and you can log in directly or allow paired screens to fetch synchronized widgets without further setup!

---

## 🧪 Developer Sandbox & Mock API Bench

The control panel features an interactive **Developer Sandbox** enabling you to:
1.  **Test Live Integrations:** Manually trigger API synchronization routines and display raw response bodies on a scrolling live console.
2.  **Mock UI Layouts:** Choose your target MQTT topic (`auradeck/spotify`, `auradeck/calendar`, etc.), customize the payload, and click **Publish to Mosquitto** to push the JSON state instantly. This permits full frontend UI testing without needing active API logins.

---

## 🔧 Troubleshooting & Common Issues (การแก้ปัญหาระหว่างติดตั้ง)

Here are standard solutions to common deployment quirks encountered during local setup on the Raspberry Pi:

### 1. `docker-compose: command not found`
*   **Cause:** Newer Docker installations package Docker Compose as a built-in CLI plugin (`docker compose`) instead of the legacy standalone script (`docker-compose`).
*   **Solution:** Use the modern space-separated command:
    ```bash
    docker compose up -d --build
    ```
*   **Alternative:** Install the official Compose plugin if missing:
    ```bash
    sudo apt-get update && sudo apt-get install -y docker-compose-plugin
    ```

### 2. `env file .../backend/.env not found: no such file or directory`
*   **Cause:** The `.env` configuration is omitted from Git (via `.gitignore`) to safeguard private keys and local config overrides.
*   **Solution:** Copy the default template and populate it:
    ```bash
    cp backend/.env.example backend/.env
    ```

### 3. `permission denied while trying to connect to the docker API`
*   **Cause:** Your current Linux session user lacks socket privileges to access `/var/run/docker.sock`.
*   **Quick Fix:** Prefix the command with `sudo`:
    ```bash
    sudo docker compose up -d --build
    ```
*   **Permanent Fix (Recommended):** Add your user account to the local `docker` security group so you don't need `sudo` for future runs:
    ```bash
    sudo usermod -aG docker $USER
    newgrp docker
    ```

---

## 🤖 Technical Note: Antigravity & AI Resource Monitoring Protocol

Since Google Antigravity does not offer an official public REST API, AuraDeck supports monitoring AI resource credits, rate limits, and quota statuses via local CLI and local configuration inspection patterns.

### Agent Role & Task Description
The **System & AI Resource Monitor Agent** inspects local CLI tools, environment configurations, and session stores to extract active AI quota, rate limit, and credit balances safely.

### Standard Operating Instructions & Workflow

1. **Check Authentication Status:**
   - Verify CLI login status and account context.
   - If unauthenticated, guide the user through secure credential/token sync without requesting raw passwords or secret keys directly in chat.

2. **Inspect Environment & Configuration:**
   - Inspect local configuration stores (e.g. `settings.json`, `oauth_creds.json`, or environment variables).
   - Check key flags such as `"useG1Credits": true` or tier-specific credit toggles.

3. **Execute Quota Check Commands:**
   - Execute CLI status/query commands to retrieve active model quotas, rate limits, and credit consumption.
   - Gracefully handle `Permission Denied` or `Rate Limit Exceeded` exceptions with actionable diagnostics.

4. **Format & Display Output:**

   ### 📊 Summary Report
   | Item | Details / Current Status |
   | :--- | :--- |
   | **Account / Context** | [Account Email or Project ID] |
   | **Active Model / Service** | [e.g. Gemini / Claude / GPT] |
   | **Quota / Rate Limit** | [Requests/Tokens Used vs Maximum Quota] |
   | **Credit Status** | [Available Credit Balance / Usage Tier] |
   | **Reset Time** | [Quota Reset Countdown] |

### Security & Governance Rules
- 🛑 **Strict Confidentiality:** Never expose raw API Keys, Access Tokens, Refresh Tokens, or Passwords in output logs or telemetry.
- 🔒 **Read-Only Operation:** Perform read/query/inspect operations only. Modifications to configuration or credential files are strictly prohibited without explicit user consent.

---

### 🛠️ Antigravity CLI Setup & Host Cron Runner

To enable automatic AI credit & quota tracking when running AuraDeck inside Docker:

1. **Install and Authenticate `agy` CLI on the Host Machine / Raspberry Pi:**
   ```bash
   agy auth login
   ```
2. **Verify CLI JSON Output on Host:**
   ```bash
   agy status --json
   ```
3. **Set Up Host Cron Job for Automatic Synchronization:**
   Add a 5-minute cron schedule on the Host Machine to execute the provided helper script:
   ```bash
   chmod +x backend/scripts/update_antigravity_cli.sh
   crontab -e
   ```
   Add the following line (replace `/path/to/ESP32-AuraDeck` with your actual repository path):
   ```cron
   */5 * * * * /bin/bash /path/to/ESP32-AuraDeck/backend/scripts/update_antigravity_cli.sh >> /tmp/auradeck_cli.log 2>&1
   ```
4. **Container Shared Volume:**
   The backend container uses `./backend/tokens:/app/tokens` volume mapping in `docker-compose.yml`. Whenever the host cron job updates `tokens/antigravity_data.json`, the FastAPI container automatically broadcasts the fresh credit metrics via MQTT to all paired ESP32-S3 screens!


