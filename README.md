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
*   **Target Board:** Waveshare ESP32-S3-RLCD-4.2 (ESP32-S3-WROOM-1-N16R8)
*   **Display:** 4.2" Reflective LCD (ST7305 Driver, SPI 4-wire, 300x400 Resolution, Portrait orientation)
*   **Sensors:** SHTC3 Temperature & Humidity sensor (I2C)
*   **RTC:** PCF85063 Real-Time Clock with battery backup (I2C)
*   **Button:** Onboard USER key for hardware-debounced page cycling (GPIO)

---

## ⚡ Backend Getting Started (Raspberry Pi / Local Server)

The backend is fully containerized with **Docker** and **Docker Compose** for instant, environment-isolated deployments.

### 1. Prerequisites
Ensure you have Docker and Docker Compose installed:
*   [Install Docker Engine / Desktop](https://docs.docker.com/get-docker/)

### 2. Configure Environment Variables
Copy the template configuration file and open it to input your credentials:
```bash
cp backend/.env.example backend/.env
```
Update `.env` with your API credentials:
*   **Google OAuth:** Create a project in Google Cloud Console, enable Calendar and Tasks APIs, and configure credentials.
*   **Spotify OAuth:** Register an app in Spotify Developer Dashboard and fetch Client ID/Secret.
*   **Stocks API:** Enter your AlphaVantage key or customize the watchlist symbols.

### 3. Spin Up Container Cluster
Run the compose file to build and run FastAPI and Mosquitto Broker in the background:
```bash
docker compose up -d --build
```

### 4. Verify Services
*   **AuraDeck Control Center & Developer Sandbox:** Go to [http://localhost:8000](http://localhost:8000)
*   **Swagger Interactive REST Documentation:** Go to [http://localhost:8000/docs](http://localhost:8000/docs)
*   **MQTT Broker Endpoint:** Port `1883` on your host machine.

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
*(Detailed instructions to be added in Phase 3)*
*   Development environment: **PlatformIO IDE** (VS Code extension)
*   Core Framework: ESP-IDF or Arduino-ESP32
*   Graphics library: **LVGL v8.x** with custom High-Contrast Monochrome light theme assets.
