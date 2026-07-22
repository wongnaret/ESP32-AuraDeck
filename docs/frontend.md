# AuraDeck ESP32-S3 Client & Graphics Guide

This guide details the hardware integration, pinout configurations, PlatformIO flashing procedures, interactive navigation pages, and LVGL v8 graphics engine architecture for the Waveshare ESP32-S3 reflective LCD client.

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
    *   `PIN_LCD_SCL`   = GPIO11 (SPI Clock)
    *   `PIN_LCD_SDA`   = GPIO12 (SPI MOSI)
    *   `PIN_LCD_CS`    = GPIO40 (Chip Select)
    *   `PIN_LCD_RS`    = GPIO5  (Register/Data Select)
    *   `PIN_LCD_RESET` = GPIO41 (Display Hardware Reset)
*   **Environmental Sensor & PCF85063 RTC (Shared I2C Bus):**
    *   `PIN_I2C_SDA`   = GPIO13
    *   `PIN_I2C_SCL`   = GPIO14
*   **Touch Panel Bus Controller:**
    *   `PIN_TP_INT`    = GPIO7
    *   `PIN_TP_RESET`  = GPIO42 (Pulled HIGH at startup to prevent holding the shared I2C bus low!)
*   **Side Interaction Button:**
    *   `PIN_BUTTON`    = GPIO18 (Hardware interrupt with clean 150ms bounce suppression)

### 3. Flashing and Building Client Firmware
1.  Open the `frontend` folder inside VS Code.
2.  PlatformIO automatically downloads all library dependencies (LVGL v8.3.11, PubSubClient, ArduinoJson).
3.  Connect your ESP32-S3 board to your developer laptop using the **Type-C USB PORT** (ensure it's wired into the CDC UART port).
4.  In the PlatformIO toolbar:
    *   Click the **Build** checkmark icon to compile the binary.
    *   Click the **Upload** right-arrow icon to flash the firmware onto the board.
    *   Click the **Serial Monitor** plug icon to watch real-time diagnostics at `115200` baud.

### 4. Interactive Page Navigation
Press the side **KEY Button (GPIO18)** on the Waveshare terminal shell to cycle through the 7 separate screen panels:
*   **Page 0**: Home Screen (Digital Clock, Date, Live temperature/humidity)
*   **Page 1**: Antigravity Credits (AI credits remaining tracker and weekly used bar)
*   **Page 2**: Market Watchlist (SET50 index, CPALL.BK, Gold, and BTC price trend indicators)
*   **Page 3**: Google Task Checklist (Chronological Google todo items with Thai reshapement)
*   **Page 4**: Google Calendar Agendas (Upcoming daily events list)
*   **Page 5**: Spotify Now Playing (Streaming song tracking with dynamic progress updates)
*   **Page 6**: Cloud Analytics (Real-time active visitors count & Month-to-Date GCP billing spending)

---

## 🎨 UI Architecture & Graphics Engine (LVGL v8 Monochrome)

The user interface is meticulously engineered around a high-contrast **1-bit monochrome theme** tailored for the ST7305 reflective display:

*   **Interactive On-Screen Boot Progress Splash Screen:** When the terminal starts up, the ST7305 display initializes first. An elegant boot status dashboard is rendered with an active progress bar and diagnostic milestone summaries (e.g., "Initializing PCF85063 RTC...", "Connecting to Wi-Fi...") so the user has immediate visual feedback of the device startup status.
*   **Self-Healing I2C Bus Recovery Routine:** Transient voltage drops from the power-hungry Wi-Fi radio can cause I2C sensors to lock up and hold SDA/SCL lines low. The client implements a hardware-level recovery routine that programmatically toggles SCL 16 times as a GPIO pin to clock out stuck slaves, issues an I2C STOP condition, restarts the `Wire` interface at 100kHz, and cleanly retries the sensor read. This makes the environmental and time polling 100% stable and self-healing.
*   **PSRAM Custom Allocation:** To protect internal SRAM from fragmentation crashes, the LVGL draw buffer is instantiated in the 8MB Octal PSRAM (`ps_malloc`), giving the screen a massive rendering canvas without consuming precious internal heap.
*   **Safe Thread/ISR Page Rotation:** Pressing the side KEY button triggers a hardware interrupt. Rather than performing hazardous drawing calls inside the ISR, it flags a thread-safe `g_pageChanged` state variable, allowing the main loop to safely perform clean up operations (`lv_obj_clean`) on the viewport container and cleanly construct the new active page view.
*   **Thai Character Reshaper (แก้ปัญหาสระลอย):** To solve vertical vowel and tone mark overlapping without a heavy layout engine, we intercept raw UTF-8 payloads, parse characters into a clean Unicode cluster, and stack them correctly using standard Thai normalization.
*   **Graceful Degradation & Network Resilience:** SHTC3 temperature probes return `NAN` indicators if unplugged instead of freezing the loop, and all APIs fallback gracefully to clean "Idle/Offline" states without stopping active screen draws.
