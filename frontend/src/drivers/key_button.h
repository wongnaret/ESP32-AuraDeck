/**
 * @file key_button.h
 * @brief Interrupt-driven debounced side customizable button driver (GPIO18).
 */

#pragma once

#include <Arduino.h>

class KeyButton {
public:
    KeyButton();
    ~KeyButton();

    /**
     * @brief Configures GPIO18 key, registers interrupt, and binds callback.
     * @param clickCallback Function pointer to invoke on valid debounced click.
     */
    void begin(void (*clickCallback)());

    /**
     * @brief Non-blocking tick. Must be polled inside main loop().
     * Checks if an interrupt was fired and filters bounces.
     */
    void tick();

    /**
     * @brief Static interrupt service routine.
     */
    static void IRAM_ATTR handleInterrupt();

private:
    static volatile bool s_wasPressed;
    static volatile uint32_t s_lastInterruptTime;
    void (*m_callback)() = nullptr;
};
