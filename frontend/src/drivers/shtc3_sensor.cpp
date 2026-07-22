/**
 * @file shtc3_sensor.cpp
 * @brief Non-blocking I2C driver for SHTC3 temperature & humidity sensor.
 */

#include "shtc3_sensor.h"

SHTCSensor::SHTCSensor() {}

SHTCSensor::~SHTCSensor() {}

bool SHTCSensor::begin(TwoWire& wire) {
    if (!m_shtc3.begin(&wire)) {
        Serial.println("❌ Error: Failed to find SHTC3 sensor on I2C bus!");
        m_initialized = false;
        return false;
    }
    
    m_initialized = true;
    Serial.println("✅ SHTC3 Temperature & Humidity sensor initialized.");
    return true;
}

bool SHTCSensor::read(float& temp, float& humidity) {
    if (!m_initialized) {
        temp = NAN;
        humidity = NAN;
        return false;
    }

    sensors_event_t h_event, t_event;
    if (!m_shtc3.getEvent(&h_event, &t_event)) {
        Serial.println("⚠️ Warning: Failed to read from SHTC3 sensor!");
        temp = NAN;
        humidity = NAN;
        return false;
    }

    temp = t_event.temperature;
    humidity = h_event.relative_humidity;
    return true;
}
