/**
 * @file pairing_manager.cpp
 * @brief TV-style device pairing flow implementation using Arduino HTTPClient.
 */

#include "pairing_manager.h"
#include <Arduino.h>

void PairingManager::init(const char* gatewayIp, const char* mac) {
    strncpy(m_gatewayIp, gatewayIp, sizeof(m_gatewayIp) - 1);
    strncpy(m_mac,       mac,       sizeof(m_mac) - 1);
    m_paired    = false;
    m_lastPollMs = 0;
    Serial.printf("[Pairing] Initialized. Gateway=%s  MAC=%s\n", m_gatewayIp, m_mac);
}

String PairingManager::buildUrl(const char* path, const char* queryParam) const {
    String url = "http://";
    url += m_gatewayIp;
    url += ":8000";
    url += path;
    if (queryParam) {
        url += "?mac=";
        url += m_mac;
        if (strlen(queryParam) > 0) {
            url += "&";
            url += queryParam;
        }
    }
    return url;
}

bool PairingManager::checkPaired() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Pairing] checkPaired: WiFi not connected, skipping.");
        return false;
    }

    HTTPClient http;
    String url = buildUrl("/api/pairing/status", "");
    http.begin(url);
    http.setTimeout(5000);
    int code = http.GET();

    if (code != 200) {
        Serial.printf("[Pairing] checkPaired HTTP %d — assuming unpaired.\n", code);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    DynamicJsonDocument doc(256);
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        Serial.println("[Pairing] checkPaired: JSON parse error.");
        return false;
    }

    m_paired = doc["paired"] | false;
    Serial.printf("[Pairing] checkPaired: paired=%s\n", m_paired ? "true" : "false");
    return m_paired;
}

bool PairingManager::requestPin(char* pinOut, int bufSize) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Pairing] requestPin: WiFi not connected.");
        return false;
    }

    HTTPClient http;
    String url = buildUrl("/api/pairing/request", "");
    http.begin(url);
    http.setTimeout(5000);
    int code = http.GET();

    if (code != 200) {
        Serial.printf("[Pairing] requestPin HTTP %d — cannot obtain PIN.\n", code);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    DynamicJsonDocument doc(256);
    DeserializationError err = deserializeJson(doc, payload);
    if (err || !doc.containsKey("pin")) {
        Serial.println("[Pairing] requestPin: JSON parse error or missing 'pin' key.");
        return false;
    }

    const char* pin = doc["pin"];
    strncpy(pinOut, pin, bufSize - 1);
    pinOut[bufSize - 1] = '\0';
    Serial.printf("[Pairing] PIN obtained: %s\n", pinOut);
    return true;
}

bool PairingManager::tick() {
    if (m_paired) return false; // already paired, nothing to do

    uint32_t now = millis();
    if (now - m_lastPollMs < POLL_INTERVAL_MS) return false;
    m_lastPollMs = now;

    bool wasPaired = m_paired;
    bool nowPaired = checkPaired();

    // Return true only on the transition from unpaired -> paired
    return (!wasPaired && nowPaired);
}
