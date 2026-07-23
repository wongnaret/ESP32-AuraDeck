/**
 * @file pairing_manager.h
 * @brief Manages the TV-style device pairing flow via REST API calls to the backend.
 *
 * Boot flow:
 *   1. checkPaired()    -> GET /api/pairing/status?mac=...
 *   2. requestPin()     -> GET /api/pairing/request?mac=...
 *   3. tick() every 10s -> re-poll /api/pairing/status until paired==true
 */

#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

class PairingManager {
public:
    PairingManager() = default;

    void init(const char* gatewayIp, const char* mac);
    bool checkPaired();
    bool requestPin(char* pinOut, int bufSize);
    bool tick();
    bool isPaired() const { return m_paired; }
    const char* getGatewayIp() const { return m_gatewayIp; }
    const char* getMac() const { return m_mac; }

private:
    bool    m_paired        = false;
    uint32_t m_lastPollMs  = 0;
    static const uint32_t POLL_INTERVAL_MS = 10000;

    char m_gatewayIp[20] = {0};
    char m_mac[18]       = {0};

    String buildUrl(const char* path, const char* queryParam = nullptr) const;
};
