# ESP32-S3 AuraDeck: Smart Ambient RLCD Dashboard

An ambient, ultra-high-contrast, low-power desktop information display utilizing a **Waveshare ESP32-S3-RLCD-4.2** reflective monochrome screen. 

The system leverages a **Raspberry Pi** (or any local home server running Docker) to manage heavy integration logic, OAuth2 credential lifecycles, and backend pollers. It passes simplified, lightweight JSON payloads to the ESP32 via REST and MQTT.

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

## 🛠️ Hardware Requirements & Specifications
*   **Target Board:** Waveshare ESP32-S3-RLCD-4.2 (ESP32-S3-WROOM-1-N16R8 with 16MB Flash and 8MB Octal PSRAM)
*   **Display:** 4.2" Reflective LCD (ST7305 Driver, SPI 4-wire, 400x300 Resolution, Landscape orientation)
*   **Sensors:** SHTC3 Temperature & Humidity sensor (I2C)
*   **RTC:** PCF85063 Real-Time Clock with battery backup (I2C)
*   **Button:** Onboard USER key for hardware-debounced page cycling (GPIO18)

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
*   **SSID:** `AuraDeck_AP`
*   **Password:** `AuraDeck1234`
*   **Pi Server IP:** `10.42.0.1` (which acts as the static gateway, API endpoint, and MQTT broker for the ESP32 screen!)

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
4.  **Update Environment Configuration:**
    Copy the generated secure `https://xxxx-xxxx.ngrok-free.app` URL from your ngrok terminal, open your `.env` file on the Pi, and update the redirect URIs:
    ```env
    GOOGLE_REDIRECT_URI=https://xxxx-xxxx.ngrok-free.app/google/callback
    SPOTIFY_REDIRECT_URI=https://xxxx-xxxx.ngrok-free.app/spotify/callback
    ```
5.  **Add Authorized Redirect URIs in Developer Portals:**
    *   **Google Cloud Console (Audience/Audience details):** Add `https://xxxx-xxxx.ngrok-free.app/google/callback` to your **Authorized redirect URIs**.
    *   **Spotify Developer Dashboard:** Add `https://xxxx-xxxx.ngrok-free.app/spotify/callback` to your App's **Redirect URIs** (remember to click **Add** and **Save**).
6.  **Perform Login:**
    Access your dashboard by going to `https://xxxx-xxxx.ngrok-free.app` from your phone or laptop on the same network, and click the Login buttons to safely obtain and write the persistent credential tokens directly onto the Pi!

### 6. Alternative: Local Token Mirroring (Offline / No-ngrok Shortcut)
If you already completed the OAuth logins on your local Windows PC and do not want to set up public tunnels, you can use the offline mirror shortcut:
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

---

## 🚀 Frontend Getting Started (ESP32-S3 PlatformIO)

The AuraDeck client runs on the ESP32-S3 with a beautiful, responsive 1-bit monochrome user interface powered by **LVGL v8**.

### 1. Development Environment
*   **IDE:** VS Code with the **PlatformIO IDE** extension installed.
*   **Framework:** Arduino-ESP32
*   **Compiler Toolchain:** Xtensa-ESP32S3-ELF (compiled with C++17 support)

### 2. Physical Pin Wiring Configurations
The default pinouts for the Waveshare board are managed inside [config.h](frontend/src/config.h):
*   **ST7305 RLCD Screen (SPI Bus):**
    *   `PIN_SPI_SCLK` = GPIO12
    *   `PIN_SPI_MOSI` = GPIO11
    *   `PIN_SPI_CS`   = GPIO10
    *   `PIN_SPI_DC`   = GPIO14
    *   `PIN_SPI_RST`  = GPIO9
    *   `PIN_DISP_EN`  = GPIO13 (Backlight/Power Gate Enable)
*   **Environmental Sensor & PCF85063 RTC (I2C Bus):**
    *   `PIN_I2C_SDA`  = GPIO4
    *   `PIN_I2C_SCL`  = GPIO5
*   **Side Interaction Button:**
    *   `GPIO18` (Hardware interrupt with clean 150ms bounce suppression)

### 3. Flashing and Building Client Firmware
1.  Open the `frontend` folder inside VS Code.
2.  PlatformIO automatically downloads all library dependencies (LVGL v8.3.11, PubSubClient, ArduinoJson).
3.  Connect your ESP32-S3 board to your developer laptop using the **Type-C USB PORT** (ensure it's wired into the CDC UART port).
4.  In the PlatformIO toolbar:
    *   Click the **Build** checkmark icon to compile the binary.
    *   Click the **Upload** right-arrow icon to flash the firmware onto the board.
    *   Click the **Serial Monitor** plug icon to watch real-time diagnostics at `115200` baud.

### 4. Interactive Page Navigation
*   Press the side **KEY Button (GPIO18)** on the Waveshare terminal shell to cycle through the 7 separate screen panels:
    *   `Page 0`: Home Screen (Digital Clock, Date, Live temperature/humidity)
    *   `Page 1`: Antigravity Credits (AI credits remaining tracker and weekly used bar)
    *   `Page 2`: Market Watchlist (SET50 index, CPALL.BK, Gold, and BTC price trend indicators)
    *   `Page 3`: Google Task Checklist (Chronological Google todo items with Thai reshapement)
    *   `Page 4`: Google Calendar Agendas (Upcoming daily events list)
    *   `Page 5`: Spotify Now Playing (Streaming song tracking with dynamic progress updates)
    *   `Page 6`: Cloud Analytics (Real-time active visitors count & Month-to-Date GCP billing spending)

---

## 🎨 UI Architecture & Graphics Engine (LVGL v8 Monochrome)
The user interface is meticulously engineered around a high-contrast **1-bit monochrome theme** tailored for the ST7305 reflective display:

*   **PSRAM Custom Allocation:** To protect internal SRAM from fragmentation crashes, the LVGL draw buffer is instantiated in the 8MB Octal PSRAM (`ps_malloc`), giving the screen a massive rendering canvas without consuming precious internal heap.
*   **Safe Thread/ISR Page Rotation:** Pressing the side KEY button triggers a hardware interrupt. Rather than performing hazardous drawing calls inside the ISR, it flags a thread-safe `g_pageChanged` state variable, allowing the main loop to safely perform clean up operations (`lv_obj_clean`) on the viewport container and cleanly construct the new active page view.
*   **Thai Character Reshaper (แก้ปัญหาสระลอย):** To solve vertical vowel and tone mark overlapping without a heavy layout engine, we intercept raw UTF-8 payloads, parse characters into a clean Unicode cluster, and stack them correctly using standard Thai normalization.
*   **Graceful Degradation & Network Resilience:** SHTC3 temperature probes return `NAN` indicators if unplugged instead of freezing the loop, and all APIs fallback gracefully to clean "Idle/Offline" states without stopping active screen draws.
