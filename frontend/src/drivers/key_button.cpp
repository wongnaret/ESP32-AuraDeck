/**
 * @file key_button.cpp
 * @brief Interrupt-driven debounced button driver for KEY (GPIO18) and BOOT (GPIO0).
 */

#include "key_button.h"
#include "config.h"

// Define static class members
volatile bool KeyButton::s_keyPressed = false;
volatile bool KeyButton::s_bootPressed = false;

KeyButton::KeyButton(uint8_t pin) : m_pin(pin) {}

KeyButton::~KeyButton() {
    detachInterrupt(digitalPinToInterrupt(m_pin));
}

void IRAM_ATTR KeyButton::isrKeyPin() {
    s_keyPressed = true;
}

void IRAM_ATTR KeyButton::isrBootPin() {
    s_bootPressed = true;
}

void KeyButton::begin(void (*clickCallback)()) {
    m_callback = clickCallback;

    // Configure pin with input pullup as active low button
    pinMode(m_pin, INPUT_PULLUP);

    // Attach interrupt to trigger on FALLING edge (when button is pressed to GND)
    if (m_pin == PIN_BOOT_BUTTON) {
        attachInterrupt(digitalPinToInterrupt(m_pin), isrBootPin, FALLING);
        Serial.printf("✅ Side Action Button (GPIO%d) configured with hardware interrupt.\n", m_pin);
    } else {
        attachInterrupt(digitalPinToInterrupt(m_pin), isrKeyPin, FALLING);
        Serial.printf("✅ Side Navigation Button (GPIO%d) configured with hardware interrupt.\n", m_pin);
    }
}

void KeyButton::tick() {
    bool pressed = false;

    if (m_pin == PIN_BOOT_BUTTON) {
        if (s_bootPressed) {
            s_bootPressed = false;
            pressed = true;
        }
    } else {
        if (s_keyPressed) {
            s_keyPressed = false;
            pressed = true;
        }
    }

    if (pressed) {
        uint32_t now = millis();

        // Software debounce: ignore triggers within 250ms of each other
        if (now - m_lastFiredTime > 250) {
            m_lastFiredTime = now;
            Serial.printf("🔘 Button click detected on GPIO%d (Debounced).\n", m_pin);
            if (m_callback) {
                m_callback();
            }
        }
    }
}
