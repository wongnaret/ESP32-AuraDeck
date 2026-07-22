/**
 * @file key_button.cpp
 * @brief Interrupt-driven debounced side customizable button driver (GPIO18).
 */

#include "key_button.h"
#include "config.h"

// Define static class members
volatile bool KeyButton::s_wasPressed = false;
volatile uint32_t KeyButton::s_lastInterruptTime = 0;

KeyButton::KeyButton() {}

KeyButton::~KeyButton() {
    detachInterrupt(digitalPinToInterrupt(PIN_KEY_BUTTON));
}

void KeyButton::begin(void (*clickCallback)()) {
    m_callback = clickCallback;

    // Configure pin with input pullup as active low button
    pinMode(PIN_KEY_BUTTON, INPUT_PULLUP);

    // Attach interrupt to trigger on FALLING edge (when button is pressed to GND)
    attachInterrupt(digitalPinToInterrupt(PIN_KEY_BUTTON), handleInterrupt, FALLING);

    Serial.println("✅ Custom side button (GPIO18) configured with hardware interrupt.");
}

void IRAM_ATTR KeyButton::handleInterrupt() {
    s_wasPressed = true;
    s_lastInterruptTime = millis();
}

void KeyButton::tick() {
    if (s_wasPressed) {
        // Atomic reset
        s_wasPressed = false;

        uint32_t now = millis();
        static uint32_t lastFiredTime = 0;

        // Software debounce: ignore triggers within 250ms of each other
        if (now - lastFiredTime > 250) {
            lastFiredTime = now;
            Serial.printf("🔘 Button click detected (Debounced). Triggering page rotation callback.\n");
            if (m_callback) {
                m_callback();
            }
        }
    }
}
