/**
 * @file shtc3_sensor.cpp
 * @brief Non-blocking I2C driver for SHTC3 temperature & humidity sensor using raw Wire commands.
 */

#include "shtc3_sensor.h"

SHTCSensor::SHTCSensor() {}

SHTCSensor::~SHTCSensor() {}

bool SHTCSensor::begin(TwoWire& wire) {
    m_wire = &wire;
    
    // Test if the sensor is responsive by sending a wakeup and reading its ID
    m_wire->beginTransmission(0x70);
    m_wire->write(0x35);
    m_wire->write(0x17);
    if (m_wire->endTransmission() != 0) {
        Serial.println("❌ Error: SHTC3 wakeup command failed!");
        m_initialized = false;
        return false;
    }
    delay(1);

    // Read ID command: 0xEFC8
    m_wire->beginTransmission(0x70);
    m_wire->write(0xEF);
    m_wire->write(0xC8);
    if (m_wire->endTransmission() != 0) {
        Serial.println("❌ Error: SHTC3 ID read command failed!");
        m_initialized = false;
        return false;
    }

    if (m_wire->requestFrom(0x70, 3) != 3) {
        Serial.println("❌ Error: SHTC3 ID read response failed!");
        m_initialized = false;
        return false;
    }

    uint16_t id = (m_wire->read() << 8) | m_wire->read();
    uint8_t crc = m_wire->read();
    Serial.printf("✅ SHTC3 sensor detected! ID: 0x%04X\n", id);

    // Put sensor to sleep
    m_wire->beginTransmission(0x70);
    m_wire->write(0xB0);
    m_wire->write(0x98);
    m_wire->endTransmission();

    m_initialized = true;
    return true;
}

bool SHTCSensor::read(float& temp, float& humidity) {
    if (!m_initialized || m_wire == nullptr) {
        temp = NAN;
        humidity = NAN;
        return false;
    }

    // 1. Wakeup SHTC3
    m_wire->beginTransmission(0x70);
    m_wire->write(0x35);
    m_wire->write(0x17);
    if (m_wire->endTransmission() != 0) {
        Serial.println("⚠️ Warning: SHTC3 wakeup failed!");
        temp = NAN;
        humidity = NAN;
        return false;
    }
    delay(1); // SHTC3 wake up time is max 240us, 1ms is extremely safe

    // 2. Send Measurement Command (Normal power, clock stretching disabled, Temperature first)
    m_wire->beginTransmission(0x70);
    m_wire->write(0x78);
    m_wire->write(0x66);
    if (m_wire->endTransmission() != 0) {
        Serial.println("⚠️ Warning: SHTC3 measure command failed!");
        temp = NAN;
        humidity = NAN;
        return false;
    }
    delay(15); // Wait 15ms for measurement (typical is 12ms)

    // 3. Read 6 bytes of data (Temperature Raw, Temperature CRC, Humidity Raw, Humidity CRC)
    if (m_wire->requestFrom(0x70, 6) != 6) {
        Serial.println("⚠️ Warning: SHTC3 data read timed out or failed!");
        temp = NAN;
        humidity = NAN;
        return false;
    }

    uint16_t t_raw = (m_wire->read() << 8) | m_wire->read();
    uint8_t t_crc = m_wire->read();
    uint16_t h_raw = (m_wire->read() << 8) | m_wire->read();
    uint8_t h_crc = m_wire->read();

    // 4. Send Sleep Command to save power
    m_wire->beginTransmission(0x70);
    m_wire->write(0xB0);
    m_wire->write(0x98);
    m_wire->endTransmission();

    // 5. Convert raw data to Celsius and RH%
    temp = -45.0f + 175.0f * ((float)t_raw / 65536.0f);
    humidity = 100.0f * ((float)h_raw / 65536.0f);

    return true;
}
