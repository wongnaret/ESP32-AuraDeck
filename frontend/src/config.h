/**
 * @file config.h
 * @brief Global hardware pin mapping and server configurations for ESP32 AuraDeck.
 */

#pragma once

// =============================================================================
// Wireless & Server Configuration (Wi-Fi & MQTT)
// =============================================================================
#define WIFI_SSID "AuraDeck_AP"
#define WIFI_PASS "AuraDeck1234"

// Raspberry Pi gateway IP address when running in Local Access Point mode
#define MQTT_SERVER "10.42.0.1"
#define MQTT_PORT 1883

// NTP Servers for system time synchronization
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.nist.gov"
#define UTC_OFFSET_SECS (7 * 3600)  // GMT+7 (Thailand Standard Time)

// Polling and updates
#define SENSOR_POLL_INTERVAL_MS 15000  // SHTC3 polling interval (15 seconds)

// =============================================================================
// Hardware GPIO Pin Configurations (Waveshare ESP32-S3-RLCD-4.2)
// =============================================================================

// ST7305 reflective display (SPI)
#define PIN_LCD_CS     40
#define PIN_LCD_RS     5   // Data/Command select
#define PIN_LCD_RESET  41
#define PIN_LCD_SCL    11  // SPI SCK
#define PIN_LCD_SDA    12  // SPI MOSI

// Shared I2C Bus (SHTC3 Sensor & PCF85063 RTC)
#define PIN_I2C_SDA    13
#define PIN_I2C_SCL    14

// Touch Panel Control Pins (Shared on I2C, must be stabilized)
#define PIN_TP_INT     7
#define PIN_TP_RESET   42

// Key Customizable Side Button
#define PIN_KEY_BUTTON 18

// Battery ADC Power Sense
#define PIN_BAT_ADC    4
