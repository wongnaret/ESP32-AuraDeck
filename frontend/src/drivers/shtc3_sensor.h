/**
 * @file shtc3_sensor.h
 * @brief Non-blocking I2C driver for SHTC3 temperature & humidity sensor.
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>

class SHTCSensor {
public:
    SHTCSensor();
    ~SHTCSensor();

    /**
     * @brief Initializes SHTC3 on the I2C bus.
     * @param wire Reference to standard Wire instance.
     * @return true if successful, false otherwise.
     */
    bool begin(TwoWire& wire);

    /**
     * @brief Reads live temperature and humidity values.
     * @param temp Reference to float to store temperature in Celsius.
     * @param humidity Reference to float to store humidity percentage.
     * @return true if successful, false if sensor failed (values default to NAN).
     */
    bool read(float& temp, float& humidity);

private:
    TwoWire* m_wire = nullptr;
    bool m_initialized = false;
};
