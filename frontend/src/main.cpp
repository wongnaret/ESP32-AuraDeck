/**
 * @file main.cpp
 * @brief Application Entry Point & Core Loop for ESP32 AuraDeck.
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "config.h"
#include "drivers/display_st7305.h"
#include "drivers/shtc3_sensor.h"
#include "drivers/rtc_pcf85063.h"
#include "drivers/key_button.h"
#include "network_manager.h"
#include "pairing_manager.h"
#include "ui/ui_manager.h"
#include "ui/pages/page_pairing.h"

// Instantiate Global Hardware Modules
PCF85063RTC        g_rtc;
ST7305Display      g_display;
SHTCSensor         g_sensor;
KeyButton          g_button;
AuraNetworkManager g_network;
UIManager          g_ui;
PairingManager     g_pairing; ///< TV-style pairing manager

// In-Memory UI State Variables
int g_currentPageIndex = 0;
const int TOTAL_PAGES = 7; // Home, Antigravity, Stocks, Todos, Calendar, Spotify, Analytics
volatile bool g_pageChanged    = false; // Set in ISR, handled safely in main loop
bool g_isPairingMode           = false; ///< true while showing pairing PIN screen
bool g_pairingCheckDone        = false; ///< pairing check runs once after first WiFi connect

uint32_t g_lastHeaderUpdateTime = 0;
uint32_t g_lastTimePrintTime    = 0;

// Button ISR callback (Rotates active screen pages)
void onPageCycleButtonPress() {
    g_currentPageIndex = (g_currentPageIndex + 1) % TOTAL_PAGES;
    g_pageChanged = true;
}

// C-linkage tick bridge for LVGL (resolves template linkage compiler error)
extern "C" uint32_t custom_tick_get(void) {
    return millis();
}

// Professional-grade I2C Bus Recovery Routine (restores bus if a slave locks SCL or SDA)
void recoverI2CBus() {
    Serial.println("🔄 Initiating active hardware I2C bus recovery sequence...");
    
    // 1. Release the bus lines by setting SCL as output and SDA as input with pullup
    pinMode(PIN_I2C_SCL, OUTPUT);
    pinMode(PIN_I2C_SDA, INPUT_PULLUP);
    delayMicroseconds(10);
    
    // 2. Clock out up to 16 cycles to force any stuck slave to release SDA
    bool released = false;
    for (int i = 0; i < 16; i++) {
        if (digitalRead(PIN_I2C_SDA) == HIGH) {
            Serial.printf("  I2C Line freed by slave after %d clock toggles.\n", i);
            released = true;
            break;
        }
        digitalWrite(PIN_I2C_SCL, LOW);
        delayMicroseconds(5);
        digitalWrite(PIN_I2C_SCL, HIGH);
        delayMicroseconds(5);
    }
    
    if (!released) {
        Serial.println("  ⚠️ Warning: I2C line did not release. Forcing STOP condition.");
    }
    
    // 3. Force a standard I2C STOP condition (SDA transitioning LOW->HIGH while SCL is HIGH)
    pinMode(PIN_I2C_SDA, OUTPUT);
    digitalWrite(PIN_I2C_SDA, LOW);
    delayMicroseconds(5);
    digitalWrite(PIN_I2C_SCL, HIGH);
    delayMicroseconds(5);
    digitalWrite(PIN_I2C_SDA, HIGH);
    delayMicroseconds(5);
    
    // 4. Return pins to standard digital inputs before letting TwoWire take over
    pinMode(PIN_I2C_SDA, INPUT_PULLUP);
    pinMode(PIN_I2C_SCL, INPUT_PULLUP);
    
    // 5. Reinitialize standard Wire interface
    Wire.end();
    delay(10);
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
    Serial.println("🔄 I2C Bus successfully re-initialized at 100kHz.");
}

void setup() {
    // 1. Initialize Serial Console for developer diagnostics
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==========================================================");
    Serial.println("🌀 Welcome to ESP32-S3 AuraDeck Smart Terminal Core");
    Serial.println("==========================================================");

    // 2. Initialize Custom High-Performance ST7305 Display & LVGL UI Manager FIRST
    // This allows us to draw live progress and diagnostic reports directly on-screen
    if (g_display.begin()) {
        if (!g_ui.begin(&g_display)) {
            Serial.println("❌ Critical Error: Failed to start UI Manager!");
        }
    } else {
        Serial.println("❌ Critical Error: Failed to start ST7305 hardware display!");
    }

    g_ui.drawBootStatus("Initializing System Core...", 10);

    // 3. Stabilize Touch Panel reset to prevent shared I2C bus contention
    g_ui.drawBootStatus("Stabilizing Touch Panel...", 20);
    pinMode(PIN_TP_RESET, OUTPUT);
    digitalWrite(PIN_TP_RESET, HIGH);
    delay(50); // Small delay to let TP bootloader settle

    // 4. Initialize Shared I2C Bus (Standard 100kHz clock speed for maximum stability)
    g_ui.drawBootStatus("Configuring I2C Bus at 100kHz...", 35);
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
    Serial.println("🔌 Shared I2C bus initialized at 100kHz.");

    // 4.1 Run I2C Scanner to verify bus integrity and list all hardware addresses
    g_ui.drawBootStatus("Scanning I2C Hardware Bus...", 45);
    Serial.println("🔍 Debug: Scanning I2C bus for physical devices...");
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("  Found I2C device at address: 0x%02X\n", address);
        } else if (error == 4) {
            Serial.printf("  Error (unknown) at address: 0x%02X\n", address);
        }
    }
    Serial.println("🔍 Debug: I2C scan complete.");

    // 5. Initialize Physical Hardware RTC Clock
    g_ui.drawBootStatus("Initializing PCF85063 RTC...", 60);
    g_rtc.begin(Wire);

    // 6. Initialize SHTC3 Environmental Sensor
    g_ui.drawBootStatus("Initializing SHTC3 Sensor...", 75);
    g_sensor.begin(Wire);

    // 7. Initialize Side Customizable Button & Attach Hardware Interrupt
    g_ui.drawBootStatus("Configuring Interrupt Keys...", 85);
    g_button.begin(onPageCycleButtonPress);

    // 8. Start Network Services (Wi-Fi, NTP auto-sync, MQTT subscribers)
    g_ui.drawBootStatus("Connecting to Local Hotspot...", 95);
    g_network.begin(&g_rtc);

    // 9. Transition cleanly to Interactive Dashboard
    g_ui.drawBootStatus("Booting Dashboard...", 100);
    delay(500);
    g_ui.completeBoot();
    // Note: Pairing check is deferred to loop() after WiFi connects (timing-safe)
}

void loop() {
    uint32_t now = millis();

    // 1. Process network connections, NTP syncs, and MQTT subscriptions
    g_network.tick();

    // 2. Process physical key debounce polls
    g_button.tick();

    // 3. Process safe thread/ISR boundary UI page rotations (only when NOT in pairing mode)
    if (g_pageChanged && !g_isPairingMode) {
        g_pageChanged = false;
        Serial.printf("🔄 Screen Event: Active UI page rotated to Page [%d/%d].\n",
                      g_currentPageIndex + 1, TOTAL_PAGES);
        g_ui.showPage(g_currentPageIndex);
    } else if (g_pageChanged && g_isPairingMode) {
        g_pageChanged = false; // Swallow button press during pairing
        Serial.println("⚠️ Button press ignored — device is in pairing mode.");
    }

    // 3a. DEFERRED PAIRING CHECK — runs exactly once after WiFi first connects.
    //     Cannot run in setup() because WiFi.begin() is async and not yet connected.
    //     Per Rule 3 (Graceful Degradation): backend unreachable → skip to dashboard.
    if (!g_pairingCheckDone && g_network.isConnected()) {
        g_pairingCheckDone = true;

        String mac     = WiFi.macAddress();
        String gateway = WiFi.gatewayIP().toString();

        Serial.printf("📺 [Pairing] First WiFi connect. MAC: %s | Gateway: %s\n",
                      mac.c_str(), gateway.c_str());
        g_pairing.init(gateway.c_str(), mac.c_str());

        bool paired = g_pairing.checkPaired();

        if (!paired) {
            char pin[8] = {0};
            bool pinOk = g_pairing.requestPin(pin, sizeof(pin));

            if (pinOk && pin[0] != '\0') {
                Serial.printf("📺 [Pairing] Device unpaired. Showing PIN screen: %s\n", pin);
                g_isPairingMode = true;
                g_ui.showPairingPage(pin, gateway.c_str());
            } else {
                Serial.println("⚠️ [Pairing] Backend unreachable for PIN. Staying on dashboard (offline mode).");
                g_isPairingMode = false;
            }
        } else {
            Serial.printf("✅ [Pairing] Already paired. Subscribing to device MQTT topics: %s\n", mac.c_str());
            g_network.subscribeDeviceTopics(mac.c_str());
            g_isPairingMode = false;
        }
    }

    // 3b. If in pairing mode: animate waiting dots + poll pairing status every 10s
    if (g_isPairingMode) {
        update_page_pairing_tick(); // Animate waiting... dots

        if (g_pairing.tick()) {
            // Newly paired! Subscribe to device-specific MAC topics and go to dashboard
            String mac = WiFi.macAddress();
            Serial.printf("🎉 Pairing successful! Switching to dashboard. MAC: %s\n", mac.c_str());
            g_network.subscribeDeviceTopics(mac.c_str());
            g_isPairingMode = false;
            g_ui.showDashboard();
        }
    }

    // 4. Process non-blocking LVGL animation & timer cycles
    g_ui.tick();

    // 5. Poll local sensor data and update Persistent Header & Active Clock every 1 second
    if (now - g_lastHeaderUpdateTime >= 1000) {
        g_lastHeaderUpdateTime = now;

        Serial.println("🔍 Debug: Polling SHTC3 environmental sensor...");
        float temperature = NAN;
        float humidity = NAN;
        bool shtc_success = g_sensor.read(temperature, humidity);
        Serial.printf("🔍 Debug: SHTC3 temperature=%.2f, humidity=%.2f (Success: %s)\n", 
                      temperature, humidity, shtc_success ? "Yes" : "No");

        if (!shtc_success) {
            Serial.println("❌ SHTC3 Poll Failed! Bus may be locked up. Attempting recovery...");
            recoverI2CBus();
            // Retry the read after bus recovery
            g_sensor.read(temperature, humidity);
        }

        Serial.println("🔍 Debug: Polling PCF85063 hardware RTC clock...");
        String currentTime = g_rtc.getFormattedTime();
        String currentDate = g_rtc.getFormattedDate();
        Serial.printf("🔍 Debug: RTC time=%s, date=%s\n", currentTime.c_str(), currentDate.c_str());
        
        bool isWifi = g_network.isConnected();
        bool isMqtt = g_network.isMqttConnected();

        // Safe Fallback: Check for sensor connection failure/disconnections (Rule 3: Graceful Degradation)
        if (isnan(temperature) || isnan(humidity)) {
            temperature = 0.0;
            humidity = 0.0;
        }

        // Update top status bar with latest telemetry (WiFi + MQTT combined status)
        g_ui.updateHeader(currentTime.c_str(), temperature, humidity, isWifi, isMqtt);

        // Forward local telemetry to active views (e.g. updating bold digital clock on Page 0 Home)
        DynamicJsonDocument telemetryDoc(256);
        telemetryDoc["time"] = currentTime.c_str();
        telemetryDoc["date"] = currentDate.c_str();
        telemetryDoc["temp"] = temperature;
        telemetryDoc["humidity"] = humidity;

        g_ui.dispatchData("auradeck/home_telemetry", telemetryDoc);
    }

    // 6. Print physical clock telemetry to Serial Console every 10 seconds
    if (now - g_lastTimePrintTime >= 10000) {
        g_lastTimePrintTime = now;

        String currentTime = g_rtc.getFormattedTime();
        String currentDate = g_rtc.getFormattedDate();
        
        Serial.printf("⏰ Hardware RTC Status: %s %s | WiFi Active: %s | Page: %d\n", 
                      currentDate.c_str(), currentTime.c_str(), 
                      g_network.isConnected() ? "YES" : "NO", g_currentPageIndex);
    }

    // Yield control to background ESP32 Core tasks (prevents WDT resets)
    delay(1);
}
