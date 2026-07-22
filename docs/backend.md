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
    Access your dashboard by going to `https://xxxx-xxxx.ngrok-free.app` from your phone or laptop on the same network, and click the Login buttons to safely obtain and write the persistent credential tokens directly onto the Pi!

### 6. Alternative: Local Token Mirroring (Offline / No-ngrok Shortcut)
If you already completed the OAuth logins on your local computer and do not want to set up public tunnels, you can use the offline mirror shortcut:
1.  Complete the login flow on your local computer (`http://127.0.0.1:8000`).
2.  Navigate to your local `backend/tokens/` directory. You will see cached JSON files (e.g., `google_token.json`, `spotify_token.json`).
3.  Copy/FTP the entire `tokens` folder directly to the `/backend/tokens/` path on your Raspberry Pi.
4.  The backend service on the Pi will immediately detect these files and automatically manage refreshing access tokens silently without requiring any browser interactions!

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
