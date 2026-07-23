/**
 * @file rtc_pcf85063.cpp
 * @brief Direct I2C driver for PCF85063 Real-Time Clock.
 */

#include "rtc_pcf85063.h"

static uint8_t bcd2bin(uint8_t val) {
    return val - 6 * (val >> 4);
}

static uint8_t bin2bcd(uint8_t val) {
    return val + 6 * (val / 10);
}

PCF85063RTC::PCF85063RTC() {}

PCF85063RTC::~PCF85063RTC() {}

bool PCF85063RTC::begin(TwoWire& wire) {
    m_wire = &wire;

    m_wire->beginTransmission(PCF85063_I2C_ADDR);
    if (m_wire->endTransmission() != 0) {
        Serial.println("❌ Error: Failed to find PCF85063 RTC at address 0x51!");
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
    if (!m_initialized || m_wire == nullptr) {
        return DateTime(2026, 1, 1, 0, 0, 0); // Safe fallback
    }

    m_wire->beginTransmission(PCF85063_I2C_ADDR);
    m_wire->write(0x04); // Start at register 0x04 (Seconds)
    if (m_wire->endTransmission() != 0) {
        return DateTime(2026, 1, 1, 0, 0, 0);
    }

    if (m_wire->requestFrom(PCF85063_I2C_ADDR, (uint8_t)7) != 7) {
        return DateTime(2026, 1, 1, 0, 0, 0);
    }

    uint8_t rawSec = m_wire->read();    // Reg 0x04: [OS | 10sec | sec]
    uint8_t rawMin = m_wire->read();    // Reg 0x05: [10min | min]
    uint8_t rawHour = m_wire->read();   // Reg 0x06: [10hr | hr]
    uint8_t rawDay = m_wire->read();    // Reg 0x07: [10day | day]
    m_wire->read();                     // Reg 0x08: Weekday (ignored for DateTime constructor)
    uint8_t rawMonth = m_wire->read();  // Reg 0x09: [10mon | mon]
    uint8_t rawYear = m_wire->read();   // Reg 0x0A: [10yr | yr]

    uint8_t sec = bcd2bin(rawSec & 0x7F);
    uint8_t min = bcd2bin(rawMin & 0x7F);
    uint8_t hour = bcd2bin(rawHour & 0x3F);
    uint8_t day = bcd2bin(rawDay & 0x3F);
    uint8_t month = bcd2bin(rawMonth & 0x1F);
    uint16_t year = bcd2bin(rawYear) + 2000;

    // Sanity check valid date ranges
    if (month < 1 || month > 12) month = 1;
    if (day < 1 || day > 31) day = 1;
    if (hour > 23) hour = 0;
    if (min > 59) min = 0;
    if (sec > 59) sec = 0;

    return DateTime(year, month, day, hour, min, sec);
}

void PCF85063RTC::adjust(const DateTime& dt) {
    if (!m_initialized || m_wire == nullptr) return;

    m_wire->beginTransmission(PCF85063_I2C_ADDR);
    m_wire->write(0x04); // Start register 0x04
    m_wire->write(bin2bcd(dt.second()) & 0x7F);              // 0x04: Seconds (OS bit cleared)
    m_wire->write(bin2bcd(dt.minute()) & 0x7F);              // 0x05: Minutes
    m_wire->write(bin2bcd(dt.hour()) & 0x3F);                // 0x06: Hours (24h format)
    m_wire->write(bin2bcd(dt.day()) & 0x3F);                 // 0x07: Day
    m_wire->write(bin2bcd(dt.dayOfTheWeek()) & 0x07);        // 0x08: Weekday
    m_wire->write(bin2bcd(dt.month()) & 0x1F);               // 0x09: Month
    m_wire->write(bin2bcd(dt.year() % 100));                 // 0x0A: Year
    m_wire->endTransmission();

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
    if (!m_initialized || m_wire == nullptr) return true;

    m_wire->beginTransmission(PCF85063_I2C_ADDR);
    m_wire->write(0x04); // Register 0x04 contains OS (Oscillator Stop) flag at Bit 7
    if (m_wire->endTransmission() != 0) return true;

    if (m_wire->requestFrom(PCF85063_I2C_ADDR, (uint8_t)1) != 1) return true;

    uint8_t rawSec = m_wire->read();
    return (rawSec & 0x80) != 0; // Bit 7 = 1 means oscillator was stopped / power lost
}

