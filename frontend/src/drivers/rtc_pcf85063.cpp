/**
 * @file rtc_pcf85063.cpp
 * @brief I2C driver for PCF85063 Real-Time Clock.
 */

#include "rtc_pcf85063.h"

PCF85063RTC::PCF85063RTC() {}

PCF85063RTC::~PCF85063RTC() {}

bool PCF85063RTC::begin(TwoWire& wire) {
    if (!m_rtc.begin(&wire)) {
        Serial.println("❌ Error: Failed to find PCF85063 RTC on I2C bus!");
        m_initialized = false;
        return false;
    }

    m_initialized = true;
    Serial.println("✅ PCF85063 Hardware RTC Clock initialized.");
    
    if (isLostPower()) {
        Serial.println("⚠️ Warning: RTC lost power! Needs time synchronization.");
    }
    
    return true;
}

DateTime PCF85063RTC::now() {
    if (!m_initialized) {
        return DateTime(2026, 1, 1, 0, 0, 0); // Safe fallback
    }
    return m_rtc.now();
}

void PCF85063RTC::adjust(const DateTime& dt) {
    if (!m_initialized) return;
    m_rtc.adjust(dt);
    
    // Clear OS (Oscillator Stop) flag if it was set
    // In RTClib's RTC_PCF85063, writing the time автоматически resets the stop flag.
    Serial.printf("⏰ Hardware RTC time updated to: %04d-%02d-%02d %02d:%02d:%02d\n", 
                  dt.year(), dt.month(), dt.day(), dt.hour(), dt.minute(), dt.second());
}

String PCF85063RTC::getFormattedTime() {
    DateTime dt = now();
    char buf[12];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
    return String(buf);
}

String PCF85063RTC::getFormattedDate() {
    DateTime dt = now();
    char buf[16];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", dt.year(), dt.month(), dt.day());
    return String(buf);
}

bool PCF85063RTC::isLostPower() {
    if (!m_initialized) return true;
    
    // Check if the time is invalid/uninitialized (default chip value is 0 or 2000)
    // RTClib's RTC_PCF85063 has lostPower() method.
    // It returns true if the clock integrity flag is set.
    return m_rtc.lostPower();
}
