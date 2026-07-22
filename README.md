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

## 📖 Comprehensive Guides & Setup Manuals

To make the codebase highly modular and maintainable, setup details, firmware flashing guides, and hardware-specific graphics explanations have been organized into the following manuals within the `docs/` directory:

### ⚡ [Backend Server Setup & Developer Guide](./docs/backend.md)
*   **Docker Ingestion & Deployment**: Step-by-step procedures to build and start the cluster (`FastAPI` & `Mosquitto Broker`) in the background.
*   **Raspberry Pi Local Access Point**: Restructuring the Pi into a standalone Wi-Fi hotspot, and solving supplicant or Broadcom driver handshake timeouts.
*   **Secure OAuth via ngrok & Token Mirroring**: Tunneling redirects safely and managing fallback tokens locally.
*   **Developer Sandbox**: Publishing mock JSON payloads to test screen layouts instantaneously without live cloud logins.
*   **System Diagnostics**: Log checking and common Linux/Docker privilege troubleshooting.

### 🔌 [Frontend Firmware & Graphics Guide](./docs/frontend.md)
*   **PlatformIO IDE Compiler Environment**: Setup instructions for ESP32-S3 firmware compiling and automatic dependency installations.
*   **Physical Pinout Configurations**: Wiring schematics for ST7305 RLCD screen SPI connections, I2C ambient sensors, and user interrupt buttons.
*   **Interactive Navigation Panels**: Breakdown of screen displays (Pages 0–6) cycling with a physical debounced button.
*   **Graphics Engine (LVGL v8 Monochrome)**: Meticulous rendering optimizations for reflective displays, custom PSRAM allocations, thread-safe page switching, and Thai vowel stacking (C++ ThaiReshaper).
*   **Network Resilience**: Bulletproof I2C bus self-healing, automatic clock synchronization, and graceful error degradations.

---

## 🌀 Architectural Features in AuraDeck 2.0

*   **Multi-Profile Session Sandboxing**: Users can register independent environments, Service Account keys, and custom Google/Spotify credentials.
*   **Multi-List Google Tasks Aggregation**: Automatically merges and prefixes tasks from multiple user-checked lists in parallel.
*   **TV-Style PIN Screen Pairing**: Registers physical screen MAC addresses wirelessly with standard 6-digit verification codes.
*   **D-Bus Container Access Point Toggles**: FastAPI containers interact with host NetworkManager services securely without root privilege elevations.
*   **High-End Dark UI**: Modern glassmorphic web dashboard providing live AP statuses, device pairings, and credentials configuration.
