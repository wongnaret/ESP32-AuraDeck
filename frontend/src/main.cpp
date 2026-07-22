/**
 * @file main.cpp
 * @brief Application Entry Point & Core Loop for ESP32 AuraDeck.
 */

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "drivers/display_st7305.h"
#include "drivers/shtc3_sensor.h"
#include "drivers/rtc_pcf85063.h"
#include "drivers/key_button.h"
#include "network_manager.h"
#include "ui/ui_manager.h"

// Instantiate Global Hardware Modules
PCF85063RTC     g_rtc;
ST7305Display   g_display;
SHTCSensor      g_sensor;
KeyButton       g_button;
AuraNetworkManager g_network;
UIManager       g_ui; // Core graphics library coordinator

// In-Memory UI State Variables (Phase 4 tracking)
int g_currentPageIndex = 0;
const int TOTAL_PAGES = 7; // Home, Antigravity, Stocks, Todos, Calendar, Spotify, Analytics
volatile bool g_pageChanged = false; // Set in ISR, handled safely in main loop

uint32_t g_lastHeaderUpdateTime = 0;
uint32_t g_lastTimePrintTime = 0;

// Button ISR callback (Rotates active screen pages)
void onPageCycleButtonPress() {
    g_currentPageIndex = (g_currentPageIndex + 1) % TOTAL_PAGES;
    g_pageChanged = true;
}

// C-linkage tick bridge for LVGL (resolves template linkage compiler error)
extern "C" uint32_t custom_tick_get(void) {
    return millis();
}

void setup() {
    // 1. Initialize Serial Console for developer diagnostics
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==========================================================");
    Serial.println("🌀 Welcome to ESP32-S3 AuraDeck Smart Terminal Core");
    Serial.println("==========================================================");

    // 2. Initialize Shared I2C Bus (Fast 400kHz clock speed)
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000);
    Serial.println("🔌 Shared I2C bus initialized at 400kHz.");

    // 3. Initialize Physical Hardware RTC Clock
    g_rtc.begin(Wire);

    // 4. Initialize SHTC3 Environmental Sensor
    g_sensor.begin(Wire);

    // 5. Initialize Side Customizable Button & Attach Hardware Interrupt
    g_button.begin(onPageCycleButtonPress);

    // 6. Initialize Custom High-Performance ST7305 Display & LVGL UI Manager
    if (g_display.begin()) {
        if (!g_ui.begin(&g_display)) {
            Serial.println("❌ Critical Error: Failed to start UI Manager!");
        }
    } else {
        Serial.println("❌ Critical Error: Failed to start ST7305 hardware display!");
    }

    // 7. Start Network Services (Wi-Fi, NTP auto-sync, MQTT subscribers)
    g_network.begin(&g_rtc);
}

void loop() {
    uint32_t now = millis();

    // 1. Process network connections, NTP syncs, and MQTT subscriptions
    g_network.tick();

    // 2. Process physical key debounce polls
    g_button.tick();

    // 3. Process safe thread/ISR boundary UI page rotations
    if (g_pageChanged) {
        g_pageChanged = false;
        Serial.printf("🔄 Screen Event: Active UI page rotated to Page [%d/%d].\n", 
                      g_currentPageIndex + 1, TOTAL_PAGES);
        g_ui.showPage(g_currentPageIndex);
    }

    // 4. Process non-blocking LVGL animation & timer cycles
    g_ui.tick();

    // 5. Poll local sensor data and update Persistent Header & Active Clock every 1 second
    if (now - g_lastHeaderUpdateTime >= 1000) {
        g_lastHeaderUpdateTime = now;

        float temperature = NAN;
        float humidity = NAN;
        g_sensor.read(temperature, humidity);

        String currentTime = g_rtc.getFormattedTime();
        String currentDate = g_rtc.getFormattedDate();
        bool isWifi = g_network.isConnected();

        // Safe Fallback: Check for sensor connection failure/disconnections (Rule 3: Graceful Degradation)
        if (isnan(temperature) || isnan(humidity)) {
            temperature = 0.0;
            humidity = 0.0;
        }

        // Update top status bar with latest telemetry
        g_ui.updateHeader(currentTime.c_str(), temperature, humidity, isWifi);

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
