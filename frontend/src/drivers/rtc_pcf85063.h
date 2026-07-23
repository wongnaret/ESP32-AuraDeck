/**
 * @file rtc_pcf85063.h
 * @brief I2C driver for PCF85063 Real-Time Clock.
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

class PCF85063RTC {
public:
    PCF85063RTC();
    ~PCF85063RTC();

    /**
     * @brief Initializes PCF85063 RTC on standard I2C.
     * @param wire Reference to standard Wire instance.
     * @return true if successful, false otherwise.
     */
    bool begin(TwoWire& wire);

    /**
     * @brief Fetches current date and time from physical RTC registers.
     * @return DateTime object representing current time.
     */
    DateTime now();

    /**
     * @brief Updates the physical RTC clock with new parameters.
     * @param dt Reference to DateTime object to save.
     */
    void adjust(const DateTime& dt);

    /**
     * @brief Formats the current time as "HH:MM:SS" (e.g. 14:30:15).
     */
    String getFormattedTime();

    /**
     * @brief Formats the current date as "YYYY-MM-DD" (e.g. 2026-07-22).
     */
    String getFormattedDate();

    /**
     * @brief Checks if the RTC has lost power or lost time configuration.
     * @return true if lost time, false otherwise.
     */
    bool isLostPower();

private:
    TwoWire* m_wire = nullptr;
    bool m_initialized = false;
    static const uint8_t PCF85063_I2C_ADDR = 0x51;
};

