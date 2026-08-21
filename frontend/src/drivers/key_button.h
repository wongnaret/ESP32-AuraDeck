/**
 * @file key_button.h
 * @brief Interrupt-driven debounced button driver for KEY (GPIO18) and BOOT (GPIO0).
 */

#pragma once

#include <Arduino.h>

class KeyButton {
public:
    KeyButton(uint8_t pin = 18);
    ~KeyButton();

    /**
     * @brief Configures GPIO pin, registers interrupt, and binds click callback.
     * @param clickCallback Function pointer to invoke on valid debounced click.
     */
    void begin(void (*clickCallback)());

    /**
     * @brief Non-blocking tick. Must be polled inside main loop().
     * Checks if an interrupt was fired and filters bounces.
     */
    void tick();

    /**
     * @brief Direct ISR triggers for static pin instances.
     */
    static void IRAM_ATTR isrKeyPin();
    static void IRAM_ATTR isrBootPin();

private:
    uint8_t m_pin;
    void (*m_callback)() = nullptr;
    uint32_t m_lastFiredTime = 0;

    static volatile bool s_keyPressed;
    static volatile bool s_bootPressed;
};
