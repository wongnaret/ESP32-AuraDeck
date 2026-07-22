/**
 * @file network_manager.h
 * @brief Manages asynchronous Wi-Fi, NTP sync with RTC, and MQTT subscriptions.
 */

#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "drivers/rtc_pcf85063.h"

class AuraNetworkManager {
public:
    AuraNetworkManager();
    ~AuraNetworkManager();

    /**
     * @brief Starts Wi-Fi connection, initializes NTP time sync, and setups MQTT.
     * @param rtc Pointer to physical hardware PCF85063 RTC object.
     * @return true if initialization completes, false otherwise.
     */
    bool begin(PCF85063RTC* rtc);

    /**
     * @brief Non-blocking tick polled in main loop().
     * Manages Wi-Fi reconnect state machine and processes MQTT loop.
     */
    void tick();

    /**
     * @brief Checks if Wi-Fi is actively connected.
     */
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }

    /**
     * @brief Checks if MQTT Broker connection is active.
     */
    bool isMqttConnected() { return m_mqttClient.connected(); }

private:
    WiFiClient m_espClient;
    PubSubClient m_mqttClient;
    PCF85063RTC* m_rtc = nullptr;

    uint32_t m_lastWifiCheckTime = 0;
    uint32_t m_lastMqttReconnectTime = 0;
    bool m_timeSynced = false;

    void connectWifi();
    void connectMqtt();
    bool syncNTPTime();

    // Inbound MQTT parser callback
    static void staticMqttCallback(char* topic, byte* payload, unsigned int length);
    void handleMqttMessage(const char* topic, const JsonDocument& doc);
};
